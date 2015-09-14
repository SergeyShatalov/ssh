
; BC1/DXT1 compression (4 bits per texel)
;uint16_t    rgb[2]; // 565 colors
;uint32_t    bitmap; // 2bpp rgb bitmap

; BC2/DXT2/3 compression (8 bits per texel)
;uint32_t    bitmap[2];  // 4bpp alpha bitmap
;D3DX_BC1    bc1;        // BC1 rgb data

; BC3/DXT4/5 compression (8 bits per texel)
;uint8_t     alpha[2];   // alpha values
;uint8_t     bitmap[6];  // 3bpp alpha bitmap
;D3DX_BC1    bc1;        // BC1 rgb data


OPTION NOKEYWORD:<width type>

.const

align 16
_row		dd 0, 0, 0, 0
row0		dd 3, 3, 3, 3, 4, 5, 6, 7
row1		dd 3, 3, 3, 3, 5, 0, 1, 7
row2		dd 3, 3, 3, 3, 6, 1, 2, 7
grid		dd 31.0, 63.0, 31.0, 0.0
half		dd 0.5, 0.5, 0.5, 0.0
f_255x3_256 dd 255.0, 255.0, 255.0, 256.0
f_0_33x4	dd 0.33, 0.33, 0.33, 0.0
f_0_66x4	dd 0.66, 0.66, 0.66, 0.0
i_0x3_1		dd 0.0, 0.0, 0.0, 1.0
msk			db -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
psteps3		dq 0, 2, 1, 0
psteps4		dq 0, 2, 3, 1
flt_max		dd 3.402823466e+38F

alpha_func	dq null_dxt1, 3, null_dxt1, 3
			dq decomp_dxt3, 4, comp_dxt3, 4
			dq decomp_dxt5, 4, comp_dxt5, 4

.data?

codes		dd 16 dup(?)
closest		db 16 dup(?)
indices		dd 16 dup(?)
swapped		db 16 dup(?)
points		dd 64 dup(?)
weights		dd 16 dup(?)

.data

.code

asm_ssh_bc_x proc USES rbx r12
		lea r15, [rcx * 4]						; pitch
		shr rcx, 2
		jz fin
		shr rdx, 2
		jz fin
		vmovaps xmm10, grid
		vmovaps xmm11, half
		vmovaps xmm12, f_255x3_256
		mov r12, offset alpha_func
		shl rax, 5
		shl r11, 4
		add r12, rax
		add r12, r11							; адрес функции работы с альфой
		mov rbx, [r12 + 8]
		test r11, r11
		jz _decompress
		; компрессор
		mov r10, offset points
		mov r11, offset weights
_loop:	push r9
		push rcx
@@:		call asm_set_colors
		call qword ptr [r12]					; пакуем альфу
		call asm_compress_colors
		add r8, 8
		add r9, 16
		loop @b
		pop rcx
		pop r9
		lea r9, [r9 + r15 * 4]
		dec rdx
		jnz _loop
		ret
_decompress:
fin:	ret
asm_ssh_bc_x endp


; in - xmm2, out - rax
asm_encode565 proc USES rcx rdx
		vmulps xmm2, xmm2, xmm10		; grid
		vaddps xmm2, xmm2, xmm11		; half
		cvtps2dq xmm2, xmm2
		pextrd edx, xmm2, 0
		pextrd ecx, xmm2, 1
		pextrd eax, xmm2, 2
		shl ecx, 5
		shl edx, 11
		or eax, ecx
		or eax, edx
		ret
asm_encode565 endp

asm_set_colors proc public USES r9 r12 rsi rdi rcx rdx
		mov rdi, r10
		mov rsi, r11
		mov rcx, 4
__loop:	mov r12, r9
		mov rdx, 4
@@:		vpmovzxbd xmm0, dword ptr [r12]
		vpshufd xmm0, xmm0, 11000110b
		vpshufd xmm1, xmm0, 11111111b
		vmovss dword ptr [rsi], xmm1			; alpha
		vcvtdq2ps xmm0, xmm0
		vpshufd xmm0, xmm0, 11100100b
		vaddps xmm0, xmm0, i_0x3_1
		vdivps xmm0, xmm0, xmm12				; 1/f_255x3_256
		vmovaps [rdi], xmm0						; points
		add rsi, 4
		add r12, 4
		add rdi, 16
		dec rdx
		jnz @b
		add r9, r15
		loop __loop
		ret
asm_set_colors endp

; r10 - points, r11 - weights(alpha), rbx(3 or 4)
asm_compress_colors proc public USES r12 rdi rcx rdx rbx
		vxorps xmm0, xmm0, xmm0					; centroid = 0
		vxorps xmm1, xmm1, xmm1					; total = 0
		mov rdi, r10
		; covariance
		mov rcx, 16
@@:		vmovaps xmm2, [rdi]
		vpshufd xmm2, xmm2, 11111111b
		vaddps xmm1, xmm1, xmm2					; total += weights[i]
		vfmadd231ps xmm0, xmm2, [rdi]			; centroid += weigths[i] * points[i]
		add rdi, 16
		loop @b
		vdivps xmm0, xmm0, xmm1					; centroid /= total
		vxorps xmm14, xmm14, xmm14				; matrix
		vxorps xmm15, xmm15, xmm15				; matrix
		mov rdi, r10
		mov rcx, 16
@@:		vmovaps xmm1, [rdi]
		vpshufd xmm2, xmm1, 11111111b
		vsubps xmm1, xmm1, xmm0					; a = (points[i] - centroid)
		vmulps xmm2, xmm2, xmm1					; b = weights[i] * a - b.x|b.y|b.z|b.w
		vpshufd xmm3, xmm1,	11000000b			; a.w|a.x|a.x|a.x
		vpshufd xmm1, xmm1,	11100101b			; a.w|a.z|a.y|a.y
		vpshufd xmm4, xmm2, 11101001b			; b.w|b.z|b.z|b.y
		vfmadd231ps xmm14, xmm3, xmm2			; c.w|c.z|c.y|c.x
		vfmadd231ps xmm15, xmm1, xmm4			; d.w|d.z|d.y|d.x
		add rdi, 16
		loop @b
		; principle
		vmovaps xmm0, xmm14						; row0 - c.w|c.z|c.y|c.x
		vshufps xmm1, xmm14, xmm15, 01001101b	; row1 - d.y|d.x|c.w|c.y
		vpshufd xmm1, xmm1, 01111000b			; c.w|d.y|d.x|c.y
		vshufps xmm2, xmm14, xmm15, 10011110b	; row2 - d.z|d.y|c.w|c.z
		vpshufd xmm2, xmm2, 01111000b			; c.w|d.z|d.y|c.z
		; EstimatePrincipleComponent
		vdpps xmm3, xmm0, xmm0, 01110001b		; r0
		vdpps xmm4, xmm1, xmm1, 01110001b		; r1
		vdpps xmm5, xmm2, xmm2, 01110001b		; r2
		vucomiss xmm3, xmm4						; r0 > r1
		jbe @f
		vmovaps xmm6, xmm0
		vucomiss xmm3, xmm5						; r0 > r2
		ja _next
@@:		vmovaps xmm6, xmm1
		vucomiss xmm4, xmm5						; r1 > r2
		ja _next
		movaps xmm6, xmm2
_next:	mov rcx, 8
@@:		vpshufd xmm3, xmm6, 11000000b			; tmp = v.w|v.x|v.x|v.x
		vmulps xmm4, xmm0, xmm3					; w = row0 * tmp
		vpshufd xmm3, xmm6, 11010101b			; tmp = v.w|v.y|v.y|v.y
		vfmadd231ps xmm4, xmm1, xmm3			; w += row1 * tmp
		vpshufd xmm3, xmm6, 11101010b			; tmp = v.w|v.z|v.z|v.z
		vfmadd231ps xmm4, xmm2, xmm3			; w += row2 * tmp
		vpshufd xmm5, xmm4, 11000000b			; tmp1 = w.splatx
		vpshufd xmm7, xmm4, 11010101b			; tmp2 = w.splaty
		vpshufd xmm8, xmm4, 11101010b			; tmp3 = w.splatz
		vmaxps xmm7, xmm7, xmm8
		vmaxps xmm5, xmm5, xmm7					; a
		vrcpps xmm5, xmm5
		vmulps xmm6, xmm4, xmm5					; v = w * rcp(a)
		loop @b
		; поиск начальной и конечной точки
		vmovss xmm2, flt_max					; min
		vxorps xmm3, xmm3, xmm3					; max
		mov rdi, r10
		mov rcx, 16
_loop:	vmovaps xmm5, [rdi]
		add rdi, 16
		vdpps xmm4, xmm5, xmm6, 01110001b		; val = dot(points[i], principle)
		vucomiss xmm4, xmm3						; val > max
		jb @f
		vmovaps xmm0, xmm5
		vmovss xmm3, xmm3, xmm4
@@:		vucomiss xmm4, xmm2						; val < min
		ja @f
		vmovaps xmm1, xmm5
		vmovss xmm2, xmm2, xmm4
@@:		loop _loop
		; запаковка
		movaps xmm2, xmm0
		call asm_encode565
		mov rcx, rax					; wColorA
		movaps xmm2, xmm1
		call asm_encode565
		mov rdx, rax					; wColorB
		mov r12, offset psteps4
		cmp rbx, 4
		jz @f
		mov r12, offset psteps3
		cmp ecx, edx
		ja @f
		xchg rcx, rdx
		movaps xmm2, xmm0
		movaps xmm0, xmm1
		movaps xmm1, xmm2
@@:		mov [r8 + 0], cx
		mov [r8 + 2], dx
		vsubps xmm2, xmm1, xmm0				; dir = colorB - colorA
		dec rbx
		vcvtsi2ss xmm3, xmm3, rbx			; fStep = uSteps - 1
		vxorps xmm4, xmm4, xmm4				; fScale = 0
		cmp ecx, edx
		jz @f
		vdpps xmm5, xmm2, xmm2, 01110001b	; tmp = dot(dir, dir)
		vdivss xmm4, xmm3, xmm5				; fScale = fSteps / tmp
@@:		vpshufd xmm4, xmm4, 0
		vmulps xmm2, xmm2, xmm4				; dir *= fScale
		xor rdx, rdx						; dw = 0
		mov rdi, r10
		mov rcx, 16							; i
_loop_:	vmovaps xmm4, [rdi]					; tmp = colors[i]
		vsubps xmm4, xmm4, xmm0
		vdpps xmm4, xmm4, xmm2, 01110001b	; fDot
		mov rax, 1							; iStep = 1
		vucomiss xmm4, xmm3					; fDot >= fStep
		jae @f
		vaddss xmm4, xmm4, half				; fDot += 0.5
		vcvtss2si rax, xmm4
		mov rax, [r12 + rax * 8]			; iStep
@@:		shl rax, 30
		shr rdx, 2
		or rdx, rax
		add rdi, 16
		loop _loop_
		mov [r8 + 4], edx
		add r8, 8
		ret
asm_compress_colors endp

comp_dxt3 proc USES rcx
		movaps xmm3, xmmword ptr msk
		mov rdi, r8
		mov rsi, r11
		mov rcx, 2
@@:		vpshufd ymm1, [rsi], 00001000b
		vpshufd ymm2, [rsi], 00001101b
		vpsrld ymm1, ymm1, 4
		vpsrld ymm2, ymm2, 4
		vpslld ymm2, ymm2, 4
		vorps ymm1, ymm1, ymm2
		vpackusdw ymm1, ymm1, ymm1
		vpackuswb ymm1, ymm1, ymm1
		vextractf128 xmm2, ymm1, 1
		vmaskmovdqu xmm1, xmm3
		add rdi, 2
		maskmovdqu xmm2, xmm3
		add rdi, 2
		add rsi, 32
		loop @b
		mov r8, rdi
		ret
;		xor rax, rax
;@@:		mov ecx, [r11 + rax * 8]
;		mov edx, [r11 + rax * 8 + 4]
;		shr rcx, 4
;		shr edx, 4
;		shl dl, 4
;		or cl, dl
;		mov [r8], cl
;		inc r8
;		inc rax
;		cmp rax, 8
;		jb @b
;		ret
comp_dxt3 endp

asm_fit_codes proc USES r12 r13
		xor rax, rax							; err = 0
		xor rcx, rcx
_loop0:	push rax
		movsxd rax, dword ptr [r11 + rcx * 4]	; value
		mov rdx, 2147483647						; least = int_max
		xor rbx, rbx							; index = 0
		xor r12, r12							; j = 0
_loop1:	mov r13, rax							; dist = value
		sub r13d, [rsi + r12 * 4]				; dist -= codes[j]
		imul r13, r13
		cmp r13, rdx
		jae @f
		mov rdx, r13
		mov rbx, r12
@@:		inc r12
		cmp r12, 8
		jb _loop1
		mov al, bl
		stosb
		pop rax
		add rax, rdx							; err += least
		inc rcx
		cmp rcx, 16
		jb _loop0
		ret
asm_fit_codes endp

comp_dxt5 proc USES rsi rdi rbx r12 r13 r14 r15
		mov r12, 255		; min5
		xor r13, r13		; max5
		mov r14, 255		; min7
		xor r15, r15		; max7
		xor rcx, rcx
_loop0:	movsxd rax, dword ptr [r11 + rcx * 4]
		cmp rax, r14
		cmovb r14, rax
		cmp rax, r15
		cmova r15, rax
		test rax, rax
		jz @f
		cmp rax, r12
		cmovb r12, rax
@@:		cmp rax, 255
		jz @f
		cmp rax, r13
		cmova r13, rax
@@:		loop _loop0
		cmp r12, r13
		cmova r12, r13
		cmp r14, r15
		cmova r14, r15
		; fix_range5
		mov rax, r13
		sub rax, r12
		cmp rax, 5
		jae @f
		lea rax, [r12 + 5]
		mov r13, 255
		cmp rax, r13
		cmovb r13, rax
@@:		mov rax, r13
		sub rax, r12
		cmp rax, 5
		jae @f
		lea rax, [r13 - 5]
		xor r12, r12
		cmp rax, r12
		cmova r12, rax
@@:		; fix_range7
		mov rax, r15
		sub rax, r14
		cmp rax, 7
		jae @f
		lea rax, [r14 + 7]
		mov r15, 255
		cmp rax, r15
		cmovb r15, rax
@@:		mov rax, r15
		sub rax, r14
		cmp rax, 7
		jae @f
		lea rax, [r15 - 7]
		xor r14, r14
		cmp rax, r14
		cmova r14, rax
@@:		inc rcx
		cmp rcx, 16
		jb _loop0
		mov rsi, offset codes
		mov [rsi + 0], r12b
		mov [rsi + 1], r13b
		mov rcx, 1
@@:		mov rax, 5
		sub rax, rcx
		imul rax, r12					; tmp1 = (5 - i) * min5
		mov rdx, r13
		imul rdx, rcx					; tmp2 = i * max5
		add rax, rdx					; tmp1 += tmp2
		xor rdx, rdx			
		mov rbx, 5
		div rbx							; tmp1 /= 5
		mov [rsi + rcx + 1], al
		inc rcx
		cmp rcx, 5
		jb @b
		mov byte ptr [rsi + 6], 0
		mov byte ptr [rsi + 7], 255
		mov [rsi + 8], r14b
		mov [rsi + 9], r15b
		mov rcx, 1
@@:		mov rax, 7
		sub rax, rcx
		imul rax, r14					; tmp1 = (7 - i) * min7
		mov rdx, r15
		imul rdx, rcx					; tmp2 = i * max7
		add rax, rdx					; tmp1 += tmp2
		xor rdx, rdx			
		mov rbx, 7
		div rbx							; tmp1 /= 7
		mov [rsi + rcx + 8 + 1], al
		inc rcx
		cmp rcx, 7
		jb @b
		mov rdi, offset indices
		call asm_fit_codes				; 5
		mov rcx, rax
		mov rdi, offset indices + 16
		add rsi, 8
		call asm_fit_codes				; 7
		mov rsi, offset indices
		mov rbx, 5
		cmp rcx, rax
		jbe @f
		mov r12, r14
		mov r13, r15
		mov rbx, 7
		add rsi, 16
@@:		; WriteAlphaBlock5(7)
		mov rdi, rsi
		cmp rbx, 7
		jz _7
		cmp r12, r13
		jbe _write
		mov rdi, offset swapped
		mov rcx, 16
_loop1:	lodsb
		mov ah, 1
		test al, al
		jz @f
		xor ah, ah
		cmp al, 1
		jz @f
		mov ah, 7
		sub ah, al
		cmp al, 5
		jbe @f
		mov ah, al
@@:		mov al, ah
		stosb
		loop _loop1
		xchg r12, r13
		sub rdi, 16
		jmp _write
_7:		cmp r12, r13
		jae _write
		mov rdi, offset swapped
		mov rcx, 16
_loop2:	lodsb
		mov ah, 1
		test al, al
		jz @f
		xor ah, ah
		cmp al, 1
		jz @f
		mov ah, 9
		sub ah, al
@@:		mov al, ah
		stosb
		loop _loop2
		xchg r12, r13
		sub rdi, 16
_write:	; WriteAlphaBlock
		mov [r8 + 00], r12b
		mov [r8 + 01], r13b
		add r8, 2
		mov rcx, 2
_loop3:	push rcx
		xor rax, rax					; value = 0
		xor rbx, rbx
@@:		movzx rdx, byte ptr [rdi]
		inc rdi
		lea rcx, [rbx * 2 + rbx]
		shl rdx, cl
		or rax, rdx
		inc rbx
		cmp rbx, 8
		jb @b
		xor rbx, rbx
@@:		lea rcx, [rbx * 8]
		mov rdx, rax
		shr rdx, cl
		mov [r8], dl
		inc r8
		inc rbx
		cmp rbx, 3
		jb @b
		pop rcx
		loop _loop3
		ret
comp_dxt5 endp

end

void D3DXEncodeBC3(uint8_t *pBC, const XMVECTOR *pColor, DWORD flags)
{
    HDRColorA Color[NUM_PIXELS_PER_BLOCK];
    for(size_t i = 0; i < NUM_PIXELS_PER_BLOCK; ++i) XMStoreFloat4( reinterpret_cast<XMFLOAT4*>( &Color[i] ), pColor[i] );
    auto pBC3 = reinterpret_cast<D3DX_BC3 *>(pBC);
    float fAlpha[NUM_PIXELS_PER_BLOCK];
    float fError[NUM_PIXELS_PER_BLOCK];
    float fMinAlpha = Color[0].a;
    float fMaxAlpha = Color[0].a;
    for(size_t i = 0; i < NUM_PIXELS_PER_BLOCK; ++i)
    {
        float fAlph = Color[i].a;
        fAlpha[i] = static_cast<int32_t>(fAlph * 255.0f + 0.5f) * (1.0f / 255.0f);
        if(fAlpha[i] < fMinAlpha) fMinAlpha = fAlpha[i];
        else if(fAlpha[i] > fMaxAlpha) fMaxAlpha = fAlpha[i];
    }
    EncodeBC1(&pBC3->bc1, Color, false, 0.f, flags);
    size_t uSteps = ((0.0f == fMinAlpha) || (1.0f == fMaxAlpha)) ? 6 : 8;
    float fAlphaA, fAlphaB;
    OptimizeAlpha<false>(&fAlphaA, &fAlphaB, fAlpha, uSteps);
    uint8_t bAlphaA = (uint8_t) static_cast<int32_t>(fAlphaA * 255.0f + 0.5f);
    uint8_t bAlphaB = (uint8_t) static_cast<int32_t>(fAlphaB * 255.0f + 0.5f);
    fAlphaA = (float) bAlphaA * (1.0f / 255.0f);
    fAlphaB = (float) bAlphaB * (1.0f / 255.0f);
    static const size_t pSteps6[] = { 0, 2, 3, 4, 5, 1 };
    static const size_t pSteps8[] = { 0, 2, 3, 4, 5, 6, 7, 1 };
    const size_t *pSteps;
    float fStep[8];
    if(6 == uSteps)
    {
        pBC3->alpha[0] = bAlphaA;
        pBC3->alpha[1] = bAlphaB;
        fStep[0] = fAlphaA;
        fStep[1] = fAlphaB;
        for(size_t i = 1; i < 5; ++i) fStep[i + 1] = (fStep[0] * (5 - i) + fStep[1] * i) * (1.0f / 5.0f);
        fStep[6] = 0.0f;
        fStep[7] = 1.0f;
        pSteps = pSteps6;
    }
    else
    {
        pBC3->alpha[0] = bAlphaB;
        pBC3->alpha[1] = bAlphaA;
        fStep[0] = fAlphaB;
        fStep[1] = fAlphaA;
        for(size_t i = 1; i < 7; ++i) fStep[i + 1] = (fStep[0] * (7 - i) + fStep[1] * i) * (1.0f / 7.0f);
        pSteps = pSteps8;
    }
    float fSteps = (float) (uSteps - 1);
    float fScale = (fStep[0] != fStep[1]) ? (fSteps / (fStep[1] - fStep[0])) : 0.0f;
    for(size_t iSet = 0; iSet < 2; iSet++)
    {
        uint32_t dw = 0;
        size_t iMin = iSet * 8;
        size_t iLim = iMin + 8;
        for(size_t i = iMin; i < iLim; ++i)
        {
            float fAlph = Color[i].a;
            float fDot = (fAlph - fStep[0]) * fScale;
            uint32_t iStep;
            if(fDot <= 0.0f) iStep = ((6 == uSteps) && (fAlph <= fStep[0] * 0.5f)) ? 6 : 0;
            else if(fDot >= fSteps) iStep = ((6 == uSteps) && (fAlph >= (fStep[1] + 1.0f) * 0.5f)) ? 7 : 1;
            else iStep = static_cast<uint32_t>( pSteps[static_cast<size_t>(fDot + 0.5f)] );
            dw = (iStep << 21) | (dw >> 3);
        }
        pBC3->bitmap[0 + iSet * 3] = ((uint8_t *) &dw)[0];
        pBC3->bitmap[1 + iSet * 3] = ((uint8_t *) &dw)[1];
        pBC3->bitmap[2 + iSet * 3] = ((uint8_t *) &dw)[2];
    }
}

null_dxt1 proc
		ret
null_dxt1 endp

decomp_dxt3 proc
		ret
decomp_dxt3 endp

decomp_dxt5 proc
		ret
decomp_dxt5 endp

asm_decompress_colors proc
		ret
asm_decompress_colors endp

end

dxtUnpack565 proc USES rcx rdx
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000000011111b
		and ecx, 00000000000000000000011111100000b
		and edx, 00000000000000001111100000000000b
		shl eax, 3
		shl ecx, 5
		shl edx, 8
		or eax, ecx
		or eax, edx
		ret
dxtUnpack565 endp


dxt3cAlpha proc
		xor rcx, rcx
@@:		mov eax, [r14 + rcx]
		mov edx, [r14 + rcx + 4]
		and eax, 15
		and edx, 15
		shl edx, 4
		or eax, edx
		mov [r8], al
		inc r8
		add rcx, 8
		cmp rcx, 64
		jb @b
		ret
dxt3cAlpha endp

; edx = max ebx = min ecx = it
asmFixRange proc
		mov eax, edx
		sub eax, ebx			; tmp1 = max - min
		cmp eax, ecx			; tmp1 < it
		jge @f
		lea eax, [ebx + ecx]	; min + it
		mov edx, 255
		cmp eax, edx			; 
		cmovl edx, eax
@@:		mov eax, edx
		sub eax, ebx
		cmp eax, ecx
		jge @f
		mov eax, edx
		sub eax, ecx
		xor ebx, ebx
		cmp eax, ebx
		cmovg ebx, eax
@@:		ret
asmFixRange endp

;rsi = codes rdi = indices
asmFitCodes proc USES rbx rdx
		xor r11, r11				; err = 0
		xor rcx, rcx				; i = 0
_loop:	mov eax, [r14 + rcx * 4]
		push rcx
		push rdi
		xor rbx, rbx				; index = 0
		mov rdi, 2147483647			; least = INT_MAX
		xor rdx, rdx				; j = 0
@@:		mov r10, rax				; value
		movzx rcx, byte ptr [rsi + rdx]; codes[j]
		sub rax, rcx				; dist = value - codes[j]
		imul rax, rax				; dist *= dist
		cmp rax, rdi				; dist < least
		cmovl rdi, rax
		cmovl rbx, rdx
		mov rax, r10
		inc rdx
		cmp rdx, 8
		jb @b
		add r11, rdi				; err += least
		pop rdi
		pop rcx
		mov [rdi + rcx], bl			; indices[i] = index
		inc rcx
		cmp rcx, 16
		jb _loop
		mov rax, r11				; err
		ret
asmFitCodes endp

dxt5cAlpha7 proc USES rcx r8 r9 r10 r11
LOCAL @@codes7[8]:BYTE
		xor rdx, rdx				; max7
		mov rbx, 255				; min7
		xor rcx, rcx				; i = 0
@@:		mov eax, dword ptr [rcx * 4 + r14]	; val
		cmp eax, ebx				; val < min7
		cmovb ebx, eax
		cmp eax, edx				; val > max7
		cmova edx, eax
		inc rcx
		cmp rcx, 16
		jb @b
		cmp ebx, edx				; min7 > max7
		cmova ebx, edx
		mov rcx, 7
		call asmFixRange
		lea rsi, @@codes7
		mov [rsi + 0], bl			; codes7[0] = min7
		mov [rsi + 1], dl			; codes7[1] = max7
		mov rcx, 1
		mov r9, 7
		mov r10, rdx
@@:		mov rax, rcx
		imul rax, r10				; tmp1 = i * max7
		mov r8, r9
		sub r8, rcx
		imul r8, rbx				; tmp2 = (7 - i) * min7
		add rax, r8					; tmp1 += tmp2
		xor rdx, rdx				
		div r9						; tmp1 /= 7
		mov [rsi + rcx + 1], al		; codes7[1 + i] = tmp1
		inc rcx
		cmp rcx, 7
		jb @b
		mov rdx, r10
		mov rdi, offset dxtIndices7Alpha
		call asmFitCodes
		ret
dxt5cAlpha7 endp

dxt5cAlpha5 proc USES rcx r8 r9 r10 r11
LOCAL @@codes5[8]:BYTE
		xor rdx, rdx				; max5
		mov rbx, 255				; min5
		xor rcx, rcx				; i = 0
_loop0:	mov eax, dword ptr [rcx * 4 + r14]	; val
		test eax, eax
		jz @f
		cmp eax, ebx				; val < min5
		cmovb ebx, eax
@@:		cmp rax, 255
		jz @f
		cmp eax, edx				; val > max5
		cmova edx, eax
@@:		inc rcx
		cmp rcx, 16
		jb _loop0
		cmp ebx, edx				; min5 > max5
		cmova ebx, edx
		mov rcx, 5
		call asmFixRange
		lea rsi, @@codes5
		mov [rsi + 0], bl			; codes5[0] = min5
		mov [rsi + 1], dl			; codes5[1] = max5
		mov byte ptr [rsi + 6], 0	; codes5[6] = 0
		mov byte ptr [rsi + 7], 255	; codes5[7] = 255
		mov rcx, 1
		mov r9, 5
		mov r10, rdx
@@:		mov rax, rcx
		imul rax, r10				; tmp1 = i * max5
		mov r8, r9
		sub r8, rcx
		imul r8, rbx				; tmp2 = (5 - i) * min5
		add rax, r8					; tmp1 += tmp2
		xor rdx, rdx				
		div r9						; tmp1 /= 5
		mov [rsi + rcx + 1], al		; codes5[1 + i] = tmp1
		inc rcx
		cmp rcx, 5
		jb @b
		mov rdx, r10
		mov rdi, offset dxtIndices5Alpha
		call asmFitCodes
		ret
dxt5cAlpha5 endp

dxt5cAlpha proc USES rbx r10 r11
LOCAL @@swapped[16]:BYTE
		call dxt5cAlpha5
		mov rcx, rax			; err5
		mov r10, rdx			; max5
		mov r11, rbx			; min5
		call dxt5cAlpha7
		cmp rcx, rax
		ja _7
		;WriteAlphaBlock5(min5, max5, indices5, block);
		mov rdi, offset dxtIndices5Alpha
		mov rdx, r10
		mov rbx, r11
		cmp rbx, rdx
		jbe _next
		xchg rbx, rdx
		lea rsi, @@swapped
		xor rcx, rcx
_loop0:	mov al, [rdi + rcx]
		xor ah, al
		cmp al, 2
		jb @f
		mov ah, al
		cmp al, 5
		ja @f
		mov ah, 7
		sub ah, al
@@:		mov [rsi + rcx], ah
		inc rcx
		cmp rcx, 16
		jb _loop0
		mov rdi, rsi
		jmp _next
_7:		;WriteAlphaBlock7(min7, max7, indices7, block);
		mov rdi, offset dxtIndices7Alpha
		cmp rbx, rdx
		jae _next
		xchg rbx, rdx
		lea rsi, @@swapped
		xor rcx, rcx
_loop1:	mov al, [rdi + rcx]
		xor ah, al
		cmp al, 2
		jb @f
		mov ah, 9
		sub ah, al
@@:		mov [rsi + rcx], ah
		inc rcx
		cmp rcx, 16
		jb _loop1
		mov rdi, rsi
_next:	mov [r8], bl
		mov [r8 + 1], dl
		add r8, 2
		xor rdx, rdx			; i = 0
_loop:	xor rax, rax			; value = 0
		mov rcx, 8
@@:		shl rax, 3
		mov bl, [rdi + rcx - 1]
		or al, bl
		loop @b
		add rdi, 8
		mov [r8], ax
		shr eax, 16
		mov [r8 + 2], al
		add r8, 3
		inc rdx
		cmp rdx, 2
		jb _loop
		ret
dxt5cAlpha endp

dxt1Alpha proc
		ret
dxt1Alpha endp

dxt3dAlpha proc USES rcx
		mov rsi, r8
		xor rdi, rdi
		mov rcx, 4
@@:		mov ax, [r9]
		mov dx, ax
		and ax, 0000111100001111b
		and dx, 1111000011110000b
		mov r14w, ax
		mov r15w, dx
		shl ax, 4
		shr dx, 4
		or ax, r14w
		or dx, r15w
		mov [rsi + rdi + 3], al
		mov [rsi + rdi + 7], dl
		mov [rsi + rdi + 11], ah
		mov [rsi + rdi + 15], dh
		add r9, 2
		add rdi, r12
		loop @b
		ret
dxt3dAlpha endp

dxt5dAlpha proc USES rcx
LOCAL @@codes[8]:BYTE
LOCAL @@indices[16]:BYTE
		lea rsi, @@codes
		mov ax, [r9]
		add r9, 2
		mov [rsi], ax
		mov word ptr [rsi + 6], 0ff00h
		movzx rdi, al
		mov al, ah
		movzx rbx, al
		mov rcx, 1
		mov r15, 5
		cmp rdi, rbx						; alpha1 <= alpha2
		mov r14, 7
		cmovbe r14, r15
@@:		mov rax, rdi
		mov r15, r14
		sub r15, rcx						; tmp1 = 5(7) - i
		imul r15, rax						; tmp1 *= alpha0
		mov rax, rcx
		imul rax, rbx						; tmp2 = i * alpha1
		add rax, r15						; tmp2 += tmp1
		xor rdx, rdx
		div r14								; tmp2 /= 5(7)
		mov [rsi + rcx + 1], al				; codes[i + 1] = tmp2
		inc rcx
		cmp rcx, r14
		jb @b
		lea rdi, @@indices
		xor rcx, rcx						; i = 0
		mov r14, rdi
_loop:	mov eax, [r9]
		and eax, 0ffffffh					; value
		add r9, 3
		xor rdx, rdx						; j = 0
@@:		mov bl, al
		and bl, 7
		shr rax, 3
		mov [r14 + rdx], bl
		inc rdx
		cmp rdx, 8
		jb @b
		add r14, 8
		inc rcx
		cmp rcx, 2
		jb _loop
		xor rcx, rcx
		xor rdx, rdx
@@:		movzx rax, byte ptr [rdi + rcx]		; index
		mov al, [rsi + rax]					; alpha
		mov byte ptr [r8 + rdx + 3], al
		add rdx, 4
		inc rcx
		test rcx, 3
		jnz @b
		sub rdx, 12
		add rdx, r12
		cmp rcx, 16
		jb @b
		ret
dxt5dAlpha endp

asm_ssh_bc_x proc public
		movsxd rdx, dword ptr [rdx + 4]
		shr rdx, 2
		jz _fin
		lea r12, [rcx * 4]
		shr rcx, 2
		jle _fin
		test r11, r11					; проверка на компрессию
		jnz _dxtC
		; декомпрессор
		movaps xmm14, _fpDiv2_0
		movaps xmm13, _fpDiv3_0
		movaps xmm12, _or255x4
		mov r10, offset dDxt
		shl rax, 4
		mov r11, [rax + r10 + 8]		; смещение к индексам
		mov r10, [rax + r10]			; адрес функции распаковки альфа канала
		mov rax, r12
		lea r13, [r12 * 2 + rax]
_height:push rcx
		push rdx
_width:	mov eax, [r9 + r11 + 4]			; взять индексы цветов
		mov byte ptr [_shufd1 + 4], al
		mov byte ptr [_shufd2 + 4], ah
		shr eax, 16
		mov byte ptr [_shufd3 + 4], al
		mov byte ptr [_shufd4 + 4], ah
		mov ax, [r9 + r11]				; первый цвет
		mov bx, ax
		call dxtUnpack565				; распаковываем
		movd xmm0, eax
		pmovzxbd xmm2, xmm0
		cvtdq2ps xmm2, xmm2
		mov ax, [r9 + r11 + 2]			; второй цвет
		call dxtUnpack565				; распаковываем
		movd xmm1, eax
		pmovzxbd xmm3, xmm1
		cvtdq2ps xmm3, xmm3
		vaddps xmm4, xmm2, xmm3
		divps xmm4, xmm14
		xorps xmm5, xmm5
		test r11, r11				; проверка на отсутствие альфа канала
		jnz @f
		cmp bx, [r9 + r11 + 2]
		jb @f
		vmulps xmm4, xmm2, xmm14
		addps xmm4, xmm3
		vmulps xmm5, xmm3, xmm14
		addps xmm5, xmm2
		divps xmm4, xmm13
		divps xmm5, xmm13
		cvtps2dq xmm5, xmm5
		packssdw xmm5, xmm5
		packuswb xmm5, xmm5
@@:		cvtps2dq xmm4, xmm4
		packssdw xmm4, xmm4
		packuswb xmm4, xmm4
		unpcklps xmm0, xmm1
		unpcklps xmm4, xmm5
		movlhps xmm0, xmm4
		orps xmm0, xmm12
_shufd1:pshufd xmm1, xmm0, 0
		movaps [r8 + 00], xmm1
_shufd2:pshufd xmm2, xmm0, 0
		movaps [r8 + r12], xmm2
_shufd3:pshufd xmm3, xmm0, 0
		movaps [r8 + r12 * 2], xmm3
_shufd4:pshufd xmm4, xmm0, 0
		movaps [r8 + r13], xmm4
		; распаковываем альфа канал
		call r10
		; следующий блок
		add r9, 8
		add r8, 16
		dec rcx
		jnz _width
		pop rdx
		pop rcx
		add r8, r13
		dec rdx
		jnz _height
_fin:	ret
		; компрессор
_dxtC:	movaps xmm10, dxtGrid
		movaps xmm11, dxtHalf
		movaps xmm13, _fp0001
		movaps xmm14, _255x4
		mov r10, offset cDxt
		mov r10, [rax * 8 + r10]					; адрес функции запаковки альфа канала
		mov r11, offset dxtColors
		mov r14, offset dxtWeights
height:	push rcx
		push rdx
		push r9
width:	push rcx
		; создаем объект colorSet
		call asmMakeColorSet
		; пакуем альфа канал
		call r10
		; создаем объект rangeFit
		call asmMakeCovariance
		call asmComputePrinciple
		; найти первый и последний цвет из диапазона
		movaps xmm5, [r11]				; start
		movaps xmm6, xmm5				; end
		movaps xmm0, xmm5
		dpps xmm0, xmm7, 01110001b		; min
		movss xmm1, xmm0				; max
		mov rax, 16
_l0:	movaps xmm2, [rax + r11]		; colors[i]
		movaps xmm3, xmm2
		dpps xmm3, xmm7, 01110001b		; val
		ucomiss xmm3, xmm0
		jae @f
		movaps xmm5, xmm2
		movss xmm0, xmm3
		jmp _l1
@@:		ucomiss xmm3, xmm1
		jbe _l1
		movaps xmm6, xmm2
		movss xmm1, xmm3
_l1:	add rax, 16
		cmp rax, 256
		jb _l0
		; пакуем цвет
		call dxtColorCompress
		add r9, 16
		pop rcx
		loop width
		pop r9
		pop rdx
		pop rcx
		lea r9, [r9 + r12 * 4]
		dec rdx
		jnz height
		ret
asm_ssh_bc_x endp

