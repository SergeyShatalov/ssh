
include asm_ssh.inc

.const

align 16
msk			db -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
f_0_33x3_1	dd 0.33, 0.33, 0.33, 1.0
f_0_66x3_1	dd 0.66, 0.66, 0.66, 1.0
f_0_5x3_1	dd 0.5, 0.5, 0.5, 1.0
grid		dd 31.0, 63.0, 31.0, 0.0
half		dd 0.5, 0.5, 0.5, 0.0
psteps3		dq 0, 2, 1, 0
psteps4		dq 0, 2, 3, 1
psteps6		dq 0, 2, 3, 4, 5, 1
psteps8		dq 0, 2, 3, 4, 5, 6, 7, 1
f_1_0		dd 1.0
f_0_0		dd 0.0

.code

null_bc1 proc
		ret
null_bc1 endp

asm_ssh_bc_x proc USES rdi rsi rbx r12 r13 r14 r15
LOCAL points[64]:DWORD
LOCAL alpha[16]:DWORD
		vzeroupper
		lea r15, [rcx * 4]						; pitch
		shr rcx, 2
		jz fin
		shr rdx, 2
		jz fin
		mov r12, offset alpha_func
		shl rax, 5
		shl r11, 4
		add r12, rax
		add r12, r11							; адрес функции работы с альфой
		mov rbx, [r12 + 8]
		test r11, r11
		jz _decompress
		; компрессор
		vmovaps xmm10, grid
		vmovaps xmm11, half
		vmovaps ymm12, f_255x8
		lea r10, points
		lea r11, alpha
_compress:
		push r9
		push rcx
@@:		call asm_set_colors
		call qword ptr [r12]					; пакуем альфу
		call asm_compress_colors
		add r9, 4
		loop @b
		pop rcx
		pop r9
		lea r9, [r9 + r15 * 4]
		dec rdx
		jnz _compress
		ret
_decompress:
		vmovaps xmm10, f_0_33x3_1
		vmovaps xmm11, f_0_66x3_1
		vmovaps xmm12, f_0_5x3_1
_loop_:	push r8
		push rcx
@@:		call asm_decompress_colors
		call qword ptr [r12]					; распаковывем альфу
		add r8, 4
		add r9, 8
		loop @b
		pop rcx
		pop r8
		lea r8, [r8 + r15 * 4]
		dec rdx
		jnz _loop_
fin:	ret
alpha_func	dq null_bc1, 3, null_bc1, 3, decomp_bc2, 4, comp_bc2, 4, decomp_bc3, 4, comp_bc3, 4
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

asm_set_colors proc public USES r9 rcx rdx
		mov rdi, r10
		mov rsi, r11
		mov rcx, 4
__loop:	mov r13, r9
		mov rdx, 4
@@:		vpmovzxbd xmm0, dword ptr [r13]
		vpshufd xmm0, xmm0, 11000110b
		vcvtdq2ps xmm0, xmm0
		vpshufd xmm0, xmm0, 11100100b
		vdivps xmm0, xmm0, xmm12				; f_255x4
		vmovaps [rdi], xmm0						; points
		vpshufd xmm1, xmm0, 00000011b
		vmovss dword ptr [rsi], xmm1			; alpha
		add rsi, 4
		add r13, 4
		add rdi, 16
		dec rdx
		jnz @b
		add r9, r15
		loop __loop
		ret
asm_set_colors endp

; r10 - points, r11 - weights(alpha), rbx(3 or 4)
asm_compress_colors proc public USES rcx rdx
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
		mov r13, offset psteps4
		cmp rbx, 4
		jz @f
		mov r13, offset psteps3
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
		mov rax, [r13 + rax * 8]			; iStep
@@:		shl rax, 30
		shr rdx, 2
		or rdx, rax
		add rdi, 16
		loop _loop_
		mov [r8 + 4], edx
		add r8, 8
		ret
asm_compress_colors endp

comp_bc2 proc USES rcx
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
comp_bc2 endp

comp_bc3 proc USES rcx
		movss xmm0, flt_max							; alpha0
		vxorps xmm1, xmm1, xmm1						; alpha1
		xor rcx, rcx
@@:		vmovss xmm2, dword ptr [r11 + rcx * 4]
		minss xmm0, xmm2
		maxss xmm1, xmm2
		inc rcx
		cmp rcx, 16
		jb @b
		mov r14, 6									; uSteps
		mov r13, offset psteps6
		vptest xmm0, xmm0
		jz @f
		vucomiss xmm1, f_1_0
		jz @f
		mov r13, offset psteps8
		mov r14, 8
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
		vcvtsi2ss xmm2, xmm2, rdx					; fSteps
		vsubss xmm3, xmm1, xmm0						; fScale = fStep[1] - fStep[0]
		vptest xmm3, xmm3
		jz @f
		vdivss xmm3, xmm2, xmm3						; fScale = fSteps / fScale
@@:		vmulss xmm4, xmm0, xmm11
		vaddss xmm5, xmm1, f_1_0
		vmulss xmm5, xmm5, xmm11
		xor rsi, rsi								; iSet
_loop_:	xor rdx, rdx								; dw = 0
		lea rdi, [rsi * 8]
		mov rcx, 8
@@:		vmovss xmm6, dword ptr [r11 + rdi * 4]		; fAlph
		vsubss xmm7, xmm6, xmm0						; tmp = fAlph - alpha0
		vmulss xmm7, xmm7, xmm3						; fDot = tmp * fScale
		vucomiss xmm7, f_0_0						; fDot <= 0.0
		ja _g
 		xor rax, rax
		cmp r14, 6
		jnz _next
		vucomiss xmm6, xmm4
		ja _next
		mov rax, 6
		jmp _next
_g:		vucomiss xmm7, xmm2							; fDot >= fSteps
		jb _l
		mov rax, 1
		cmp r14, 6
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
		loop @b
		mov [r8], edx
		add r8, 3
		inc rsi
		cmp rsi, 2
		jb _loop_
		ret
comp_bc3 endp

decomp_bc2 proc USES r8 rcx
		mov rax, [r9]
		mov r13, 4
_loop_:	xor rcx, rcx
@@:		mov rdx, rax
		and rdx, 15
		mov r10, rdx
		shl r10, 4
		or rdx, r10
		mov [r8 + rcx * 4 + 3], dl
		shr rax, 4
		inc rcx
		cmp rcx, 4
		jb @b
		add r8, r15
		dec r13
		jnz _loop_
		add r9, 8
		ret
decomp_bc2 endp

decomp_bc3 proc USES r8 rbx rcx rdx
LOCAL steps[8]:DWORD
		lea r10, steps
		movzx r11, byte ptr [r9]			; alpha0
		movzx r13, byte ptr [r9 + 1]		; alpha1
		mov rbx, 7							; iSteps
		mov rax, 5
		cmp r11, r13
		cmovbe rbx, rax
		mov [r10 + 0], r11b
		mov [r10 + 1], r13b
		mov byte ptr [r10 + 6], 0
		mov byte ptr [r10 + 7], 255
		mov rcx, 1							; i = 1
@@:		mov rax, rbx
		sub rax, rcx						; tmp1 = (iSteps - i)
		imul rax, r11						; tmp1 *= alpha0
		mov r14, r13
		imul r14, rcx						; tmp2 = alpha1 * i
		add rax, r14						; tmp1 += tmp2
		xor rdx, rdx
		div rbx								; tmp1 /= iSteps
		mov [r10 + rcx + 1], al
		inc rcx
		cmp rcx, rbx
		jb @b
		mov rax, [r9 + 2]					; index alpha (16 * 3 bits)
		mov r14, 4
_loop_0:xor rcx, rcx
@@:		mov rdx, rax
		and rdx, 7
		mov dl, [r10 + rdx]
		mov [r8 + rcx * 4 + 3], dl
		shr rax, 3
		inc rcx
		cmp rcx, 4
		jb @b
		add r8, r15
		dec r14
		jnz _loop_0
		add r9, 8
		ret
decomp_bc3 endp

asm_decode565 proc USES rcx rdx
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
		or eax, 0ff000000h
		vmovd xmm15, eax
		vpmovzxbd xmm15, xmm15
		vcvtdq2ps xmm15, xmm15
;		vdivps xmm2, xmm2, xmm10
		ret
asm_decode565 endp

asm_decompress_colors proc USES r9 rcx rdx
		cmp rbx, 3						; пропустить, если это не bc1
		jz @f
		add r9, 8
@@:		mov eax, [r9 + 4]
		mov rdi, offset idx
		mov [rdi + 04], al
		mov [rdi + 09], ah
		shr rax, 16
		mov [rdi + 14], al
		mov [rdi + 19], ah
		mov cx, [r9]
		mov ax, cx
		call asm_decode565
		vmovd xmm0, eax
		vmovaps xmm2, xmm15
		mov dx, [r9 + 2]
		mov ax, dx
		call asm_decode565
		vmovd xmm1, eax
		vmovaps xmm3, xmm15
		cmp rbx, 4
		jz @f
		vmovaps xmm15, xmm12
		vxorps xmm5, xmm5, xmm5
		cmp cx, dx
		jbe _nbc1
@@:		vmovaps xmm15, xmm10
		vsubps xmm5, xmm3, xmm2
		vmulps xmm5, xmm5, xmm11
		vaddps xmm5, xmm5, xmm2
		vcvtps2dq xmm5, xmm5
		vpackusdw xmm5, xmm5, xmm5
		vpackuswb xmm5, xmm5, xmm5
_nbc1:	vsubps xmm4, xmm3, xmm2
		vmulps xmm4, xmm4, xmm15
		vaddps xmm4, xmm4, xmm2
		vcvtps2dq xmm4, xmm4
		vpackusdw xmm4, xmm4, xmm4
		vpackuswb xmm4, xmm4, xmm4
		vunpcklps xmm0, xmm0, xmm1
		vunpcklps xmm4, xmm4, xmm5
		vmovlhps xmm0, xmm0, xmm4
idx:	vpshufd xmm1, xmm0, 0
		vpshufd xmm2, xmm0, 0
		vpshufd xmm3, xmm0, 0
		vpshufd xmm4, xmm0, 0
		lea rax, [r15 * 2+ r15]
		vmovaps [r8], xmm1
		vmovaps [r8 + r15], xmm2
		vmovaps [r8 + r15 * 2], xmm3
		vmovaps [r8 + rax], xmm4
		ret
asm_decompress_colors endp

end
