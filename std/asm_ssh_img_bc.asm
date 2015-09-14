
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
_tmp		dd 0, 0, 0, 0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
msk			db -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
grid		dd 31.0, 63.0, 31.0, 0.0
half		dd 0.5, 0.5, 0.5, 0.0
psteps3		dq 0, 2, 1, 0
psteps4		dq 0, 2, 3, 1
psteps6		dq 0, 2, 3, 4, 5, 1
psteps8		dq 0, 2, 3, 4, 5, 6, 7, 1
flt_max		dd 3.402823466e+38F
f_1_0		dd 1.0
f_0_0		dd 0.0
alpha_func	dq null_dxt1, 3, null_dxt1, 3
			dq decomp_dxt3, 4, comp_dxt3, 4
			dq decomp_dxt5, 4, comp_dxt5, 4

.data?

points		dd 64 dup(?)
weights		dd 16 dup(?)
steps		dd 8 dup(?)

.data

.code

asm_ssh_bc_x proc USES rdi rsi rbx r12
		lea r15, [rcx * 4]						; pitch
		shr rcx, 2
		jz fin
		shr rdx, 2
		jz fin
		vmovaps xmm10, grid
		vmovaps xmm11, half
		vmovaps ymm12, f_255x8
		mov r12, offset alpha_func
		shl rax, 5
		shl r11, 4
		add r12, rax
		add r12, r11							; адрес функции работы с альфой
		mov rbx, [r12 + 8]
		test r11, r11
		mov r10, offset points
		mov r11, offset weights
		jz _decompress
		; компрессор
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

asm_set_colors proc public USES r9 r12 rcx rdx
		mov rdi, r10
		mov rsi, r11
		mov rcx, 4
__loop:	mov r12, r9
		mov rdx, 4
@@:		vpmovzxbd xmm0, dword ptr [r12]
		vpshufd xmm0, xmm0, 11000110b
		vcvtdq2ps xmm0, xmm0
		vpshufd xmm1, xmm0, 00000011b
		vmovss dword ptr [rsi], xmm1			; alpha
		vpshufd xmm0, xmm0, 11100100b
		vdivps xmm0, xmm0, xmm12				; f_255x4
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
asm_compress_colors proc public USES r12 rcx rdx
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
		lea rax, [rbx - 1]
		vcvtsi2ss xmm3, xmm3, rax			; fStep = uSteps - 1
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
		vcvttss2si rax, xmm4
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
@@:		vmovaps ymm0, [rsi]
		vmulps ymm0, ymm0, ymm12
		vcvtps2dq ymm0, ymm0
		vpshufd ymm1, ymm0, 00001000b
		vpshufd ymm2, ymm0, 00001101b
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
comp_dxt3 endp

comp_dxt5 proc public USES rbx r12 r13 r14
		movss xmm0, flt_max		; fMinAlpha
		vxorps xmm1, xmm1, xmm1	; fMaxAlpha
		xor rcx, rcx
@@:		vmovss xmm2, dword ptr [rsi + rcx * 4]
		minss xmm0, xmm2
		maxss xmm1, xmm2
		inc rcx
		cmp rcx, 16
		jb @b
		mov rbx, 6					; uSteps
		mov r13, offset psteps6
		vptest xmm0, xmm0
		jz @f
		vucomiss xmm1, f_1_0
		jz @f
		mov r13, offset psteps8
		mov rbx, 8
		vmovss xmm15, xmm15, xmm0
		vmovss xmm0, xmm0, xmm1
		vmovss xmm1, xmm1, xmm15
@@:		vmulss xmm15, xmm0, xmm12
		vmulss xmm14, xmm1, xmm12
		vcvttss2si rax, xmm15
		mov [r8 + 0], al
		vcvttss2si rax, xmm14
		mov [r8 + 1], al
		add r8, 2
		mov r12, offset steps
		vmovss dword ptr [r12 + 00], xmm0
		vmovss dword ptr [r12 + 04], xmm1
		mov dword ptr [r12 + 24], 0
		mov dword ptr [r12 + 28], 3f800000h
		lea rdx, [rbx - 1]
		vcvtsi2ss xmm2, xmm2, rdx					; fSteps
		mov rcx, 1
@@:		vcvtsi2ss xmm4, xmm4, rcx
		vsubss xmm3, xmm2, xmm4
		vmulss xmm5, xmm0, xmm3						; tmp = fStep[0] * (x - i)
		vfmadd231ss xmm5, xmm1, xmm4				; tmp += fStep[1] * i
		vdivss xmm5, xmm5, xmm2						; tmp /= fSteps
		vmovss dword ptr [r12 + rcx * 4 + 4], xmm5
		inc rcx
		cmp rcx, rdx
		jb @b
		vxorps xmm3, xmm3, xmm3						; fScale
		vucomiss xmm0, xmm1
		jz @f
		vsubss xmm4, xmm1, xmm0						; tmp = fStep[1] - fStep[0]
		vdivss xmm3, xmm2, xmm4						; fScale = fSteps / tmp
@@:		vmulss xmm4, xmm0, xmm11
		vaddss xmm5, xmm1, f_1_0
		vmulss xmm5, xmm5, xmm11
		xor rcx, rcx								; iSet
_loop_:	push rcx
		xor rdx, rdx								; dw = 0
		shl rcx, 3									; iMin
		lea rdi, [rcx + 8]							; iLim
@@:		vmovss xmm6, dword ptr [r11 + rcx * 4]		; fAlph
		vsubss xmm7, xmm6, xmm0
		vmulss xmm7, xmm7, xmm3						; fDot
		vucomiss xmm7, f_0_0						; fDot <= 0.0
		ja _g
 		xor rax, rax
		cmp rbx, 6
		jnz _next
		vucomiss xmm6, xmm4
		ja _next
		mov rax, rbx
		jmp _next
_g:		vucomiss xmm7, xmm2							; fDot >= fSteps
		jb _l
		mov rax, 1
		cmp rbx, 6
		jnz _next
		vucomiss xmm6, xmm5
		jb _next
		mov rax, 7
		jmp _next
_l:		vaddss xmm7, xmm7, xmm11
		vcvttss2si rax, xmm7
		mov rax, [r13 + rax * 8]
_next:	shl rax, 21
		shr rdx, 3
		or rdx, rax
		inc rcx
		cmp rcx, rdi
		jb @b
		pop rcx
		mov [r8], edx
		add r8, 3
		inc rcx
		cmp rcx, 2
		jb _loop_
		ret
comp_dxt5 endp

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
asm_ssh_bc_x endp

