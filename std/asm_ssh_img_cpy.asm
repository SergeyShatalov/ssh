
include asm_ssh.inc

XMM_LINEAR		= 0


coord_l_mirror macro p1
		xor rdx, rdx
		lea rsi, [p1 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, p1
		cmovge rdx, rsi
endm

unpack_pix macro
		movq2dq xmm0, mm2
		vpmovzxbd xmm0, xmm0
		vcvtdq2ps xmm0, xmm0
		vdivps xmm0, xmm0, xmm12
endm

pack_pix macro
		vmulps xmm0, xmm0, xmm12
		vcvtps2dq xmm0, xmm0
		vpackusdw xmm0, xmm0, xmm0
		movdq2q mm2, xmm0
		packuswb mm2, mm2
endm

.const
align 16
dataBin		dd 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0
sub_alpha	dw 256, 256, 256, 256
_mm_alpha	dq 00000000ff000000h
_alpha_not	dw 255,255,255,255
_mm_not		dq -1
_func_ops	dq _add, _sub, _set, _xor, _and, _or, _lum, _not, _alph, _fix_al, _mul, _lum_add, _lum_sub, _norm

mtx_prewit1	dd -1.0, 0.0, -1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0
mtx_prewit2	dd -1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0
mtx_emboss	dd 0.0, 1.0, 0.0, -1.0, 0.0, 1.0, 0.0, -1.0, 0.0

mtx_laplac1	dd 0.0, -1.0, 0.0, -1.0, 4.0, -1.0, 0.0, -1.0, 0.0
mtx_laplac2	dd -1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0
mtx_laplac3	dd 1.0, -2.0, 1.0, -2.0, 4.0, -2.0, 1.0, -2.0, 1.0
mtx_laplac4	dd 0.0, -1.0, 0.0, -1.0, 5.0, -1.0, 0.0, -1.0, 0.0
mtx_laplac5	dd -1.0, -1.0, -1.0, -1.0, 9.0, -1.0, -1.0, -1.0, -1.0
mtx_laplac6	dd 1.0, -2.0, 1.0, -2.0, 5.0, -2.0, 1.0, -2.0, 1.0

_sobel			dq mtx_sobel3_1, mtx_sobel3_2, mtx_sobel5_1, mtx_sobel5_2, mtx_sobel7_1, mtx_sobel7_2, mtx_sobel9_1, mtx_sobel9_2 
mtx_sobel3_1	dd 1.0, 0.0, -1.0, 2.0, 0.0, -2.0, 1.0, 0.0, -1.0
mtx_sobel3_2	dd 1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0
mtx_sobel5_1	dd 1.0, 2.0, 0.0, -2.0, -1.0, 2.0, 3.0, 0.0, -3.0, -2.0, 3.0, 4.0, 0.0, -4.0, -3.0, 2.0, 3.0, 0.0, -3.0, -2.0, 1.0, 2.0, 0.0, -2.0, -1.0
mtx_sobel5_2	dd 1.0, 2.0, 3.0, 2.0, 1.0, 2.0, 3.0, 4.0, 3.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, -2.0, -3.0, -4.0, -3.0, -2.0, -1.0, -2.0, -3.0, -2.0, -1.0
mtx_sobel7_1	dd 1.0, 2.0, 3.0, 0.0, -3.0, -2.0, -1.0, 2.0, 3.0, 4.0, 0.0, -4.0, -3.0, -2.0, 3.0, 4.0, 5.0, 0.0, -5.0, -4.0, -3.0
				dd 4.0, 5.0, 6.0, 0.0, -6.0, -5.0, -4.0, 3.0, 4.0, 5.0, 0.0, -5.0, -4.0, -3.0, 2.0, 3.0, 4.0, 0.0, -4.0, -3.0, -2.0, 1.0, 2.0, 3.0, 0.0, -3.0, -2.0, -1.0
mtx_sobel7_2	dd 1.0, 2.0, 3.0, 4.0, 3.0, 2.0, 1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 3.0, 4.0, 5.0, 6.0, 5.0, 4.0, 3.0
				dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -3.0, -4.0, -5.0, -6.0, -5.0, -4.0, -3.0, -2.0, -3.0, -4.0, -5.0, -4.0, -3.0, -2.0, -1.0, -2.0, -3.0, -4.0, -3.0, -2.0, -1.0
mtx_sobel9_1	dd 1.0, 2.0, 3.0, 4.0, 0.0, -4.0, -3.0, -2.0, -1.0, 2.0, 3.0, 4.0, 5.0, 0.0, -5.0, -4.0, -3.0, -2.0, 3.0, 4.0, 5.0, 6.0, 0.0, -6.0, -5.0, -4.0, -3.0
				dd 4.0, 5.0, 6.0, 7.0, 0.0, -7.0, -6.0, -5.0, -4.0, 5.0, 6.0, 7.0, 8.0, 0.0, -8.0, -7.0, -6.0, -5.0, 4.0, 5.0, 6.0, 7.0, 0.0, -7.0, -6.0, -5.0, -4.0
				dd 3.0, 4.0, 5.0, 6.0, 0.0, -6.0, -5.0, -4.0, -3.0, 2.0, 3.0, 4.0, 5.0, 0.0, -5.0, -4.0, -3.0, -2.0, 1.0, 2.0, 3.0, 4.0, 0.0, -4.0, -3.0, -2.0, -1.0
mtx_sobel9_2	dd 1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 5.0, 4.0, 3.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 6.0, 5.0, 4.0, 3.0
				dd 4.0, 5.0, 6.0, 7.0, 8.0, 7.0, 6.0, 5.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -4.0, -5.0, -6.0, -7.0, -8.0, -7.0, -6.0, -5.0, -4.0
				dd -3.0, -4.0, -5.0, -6.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -3.0, -4.0, -5.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0, -2.0, -3.0, -4.0, -5.0, -4.0, -3.0, -2.0, -1.0

.data?
align 16
tmp_mtx		dd 15 * 15 * 16 dup(?)
tmp_median	dw 256 dup(?)
_alpha0		dq ?
_alpha1		dq ?
w_mtx		dq ?
__tmp dd 0

.code

; операции
_add:	paddusb mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_sub:	psubusb mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_set:	pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_xor:	pxor mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_and:	pand mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_or:	por mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_lum:	movq2dq xmm0, mm2
		vpmovzxbd xmm0, xmm0
		vcvtdq2ps xmm0, xmm0
		vdpps xmm0, xmm0, _gamma, 01110001b
		cvtps2pi mm2, xmm0
		pshufw mm2, mm2, 0
		packuswb mm2, mm2
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_not:	pxor mm2, _mm_not
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_mul:	pxor mm7, mm7
		punpcklbw mm2, mm7
		punpcklbw mm3, mm7
		pmullw mm2, mm3
		packuswb mm2, mm2
		packuswb mm3, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_lum_add:movq2dq xmm0, mm2
		vpmovzxbd xmm0, xmm0
		vcvtdq2ps xmm0, xmm0
		vdpps xmm0, xmm0, _gamma, 01110001b
		cvtps2pi mm2, xmm0
		pshufw mm2, mm2, 0
		packuswb mm2, mm2
		paddusb mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_lum_sub:movq2dq xmm0, mm2
		vpmovzxbd xmm0, xmm0
		vcvtdq2ps xmm0, xmm0
		vdpps xmm0, xmm0, _gamma, 01110001b
		cvtps2pi mm2, xmm0
		pshufw mm2, mm2, 0
		packuswb mm2, mm2
		psubusb mm2, mm3
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_alph:	movq mm5, qword ptr _alpha_not
		pxor mm7, mm7
		punpcklbw mm2, mm7
		punpcklbw mm3, mm7
		pshufw mm4, mm2, 11111111b
		psubusw mm5, mm4
		pmullw mm4, mm2
		pmullw mm5, mm3
		paddusw mm4, mm5
		psraw mm4, 8
		packsswb mm4, mm4
		packuswb mm2, mm2
		packuswb mm3, mm3
		movq mm5, mm6
		pand mm2, mm6
		pandn mm5, mm4
		por mm2, mm5
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
;((src * alpha) + (dst * (256 - alpha))) / 256;
_fix_al:movq mm4, qword ptr _alpha0
		movq mm5, qword ptr _alpha1
		pxor mm7, mm7
		punpcklbw mm2, mm7
		punpcklbw mm3, mm7
		pmullw mm4, mm2
		pmullw mm5, mm3 
		paddusw mm4, mm5
		psraw mm4, 8
		packsswb mm4, mm4
		packuswb mm2, mm2
		packuswb mm3, mm3
		movq mm5, mm6
		pand mm2, mm6
		pandn mm5, mm4
		por mm2, mm5
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret
_norm:	movq mm5, mm6
		movq mm4, mm2
		pand mm2, mm6
		pandn mm5, mm4
		movq2dq xmm1, mm5
		vpmovzxbd xmm1, xmm1
		vpshufd xmm1, xmm1, 11011110b
		vcvtdq2ps xmm1, xmm1
		vmovaps xmm0, xmm1
		vdpps xmm1, xmm1, xmm1, 01110111b
		vrsqrtps xmm1, xmm1
		vmulps xmm0, xmm0, xmm1
		vmulps xmm0, xmm0, f_0_5x8
		vaddps xmm0, xmm0, f_0_5x8
		vmulps xmm0, xmm0, f_255x8
		vcvtps2dq xmm0, xmm0
		vpackusdw xmm0, xmm0, xmm0
		vpackuswb xmm0, xmm0, xmm0
		movdq2q mm5, xmm0
		por mm2, mm5
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret

asm_ssh_copy proc USES rbx rsi rdi r12 r13 r14 r15 src_bar:QWORD, src_wh:QWORD, src:QWORD, dst:QWORD, dst_bar:QWORD, dst_wh:QWORD, modify:QWORD
local w_clamp:DWORD, h_clamp:DWORD, addressing:QWORD, filter:QWORD, operation:QWORD, mtx_a:QWORD, mtx_b:QWORD, buf:QWORD
		; проверка на копирование в "себя"
		mov buf, 0
		cmp r8, r9
		jnz @f
		push rcx
		push rdx
		push r9
		mov rsi, r8
		movsxd rax, [rdx].stk_range.w
		movsxd rdx, [rdx].stk_range.h
		imul rax, rdx
		push rax
		lea rcx, [rax * 4]
		sub rsp, 32
		call malloc
		add rsp, 32
		mov buf, rax
		mov r8, rax
		pop rcx
		mov rdi, rax
		rep movsd
		pop r9
		pop rdx
		pop rcx
@@:		call init_base_modify								; инициализация основных параметров модификатора
		; обрезка координат
		call asm_clip_bar
		jc _fin
		mov r14, rcx										; w_src
		mov r15, rdx										; h_src
		mov r13, r10										; pitch_src
		dec rcx
		dec rdx
		mov w_clamp, ecx
		mov h_clamp, edx
		mov rcx, dst_bar
		mov rdx, dst_wh
		mov rax, r8
		mov r8, r9
		vcvtsi2ss xmm14, xmm14, dword ptr [rcx].stk_bar.w
		vcvtsi2ss xmm15, xmm15, dword ptr [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		mov r9, rax											; r8 - dst, r9 - src, r10 - pitch_dst, rbx - pitch_src
		call init_addressing								; инициализация адресации координат(mirror, repeat, clamp)
		vdivss xmm0, xmm0, xmm14							; dx
		vdivss xmm1, xmm1, xmm15							; dy
		; сдвинуть на clip.x, clip.y
		vshufps xmm15, xmm1, xmm0, 00000000b				; dx|dx|dy|dy
		vcvtsi2ss xmm14, xmm14, _clip.y
		vmulss xmm14, xmm14, xmm1							; y
		vshufps xmm15, xmm14, xmm15, 00100000b				; dy|dx|y|y
		vcvtsi2ss xmm14, xmm14, _clip.x
		vmulss xmm14, xmm14, xmm0							; x
		call init_filters
		call init_operation
		; главный цикл
_loop_:	push rcx
		push rdx
		push r8
		vmovss xmm15, xmm15, xmm14							; dy|dx|y|x
@@:		push rcx
		call filter
		movd mm3, dword ptr [r8]
		call operation
		movd dword ptr [r8], mm2
		vmovhlps xmm0, xmm0, xmm15							; 0|0|dy|dx
		vaddss xmm15, xmm15, xmm0							; dy|dx|y|x+dx
		pop rcx
		add r8, 4
		loop @b
		pop r8
		pop rdx
		pop rcx
		vmovlhps xmm0, xmm0, xmm15							; y|x|0|0
		vaddps xmm0, xmm0, xmm15							; y+dy|x+dx|y|x
		vmovhlps xmm15, xmm15, xmm0							; dy|dx|y+dy|x+dx
		add r8, r10
		dec rdx
		jnz _loop_
		emms
_fin:	mov rcx, buf
		sub rsp, 32
		call free
		add rsp, 32
		ret
OPTION EPILOGUE:NONE
init_base_modify:
		mov rsi, modify
		vmovaps xmm8, f_1_0x8
		vmovaps xmm11, [rsi].stk_modify.flt_vec
		vmovaps xmm12, f_255x8
		vmovaps xmm13, _gamma
		vmovss xmm0, [rsi].stk_modify.alpha
		vmulss xmm0, xmm0, f_256x8
		movq mm1, qword ptr sub_alpha
		cvtps2pi mm0, xmm0
		pshufw mm0, mm0, 0
		psubusw mm1, mm0
		movq _alpha0, mm0
		movq _alpha1, mm1
		ret
init_filters:
		mov r11, offset func_flt
		mov r12, 15
		movsxd rax, [rsi].stk_modify.type_filter
		lea r11, [r11 + rax * 8]
		mov rax, [r11 + 128]
		mov filter, rax
		movsxd rax, [rsi].stk_modify.w_mtx
		cmp rax, r12
		cmova rax, r12
		call qword ptr [r11]
		ret
func_flt	dq null,	i_sobel, i_laplac, i_prewit, i_emboss, i_normal,i_hi, i_low, null,	   i_rbrts,	null,  null,  i_contr, null, i_gamma, i_scl_b
			dq f_none,	f_sobel, f_laplac, f_prewit, f_emboss, f_nrml,	f_hi, f_low, f_median, f_rbrts,	f_max, f_min, f_contr, f_bin,f_gamma, f_scl_b
init_operation:
		movd mm0, [rsi].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, _mm_not
		movq mm6, qword ptr _mm_alpha
		mov r11, offset _func_ops
		movsxd rax, [rsi].stk_modify.src_ops
		mov rax, [r11 + rax * 8]
		mov operation, rax
		ret
f_rep_x	dd 1.0
f_rep_y	dd 1.0
init_addressing:
		vmovss xmm2, f_rep_x
		vmovss xmm3, f_rep_y
		mov r11, offset func_addr
		movsxd rax, [rsi].stk_modify.type_address
		lea r11, [r11 + rax * 8]
		mov rax, [r11 + 48]
		mov addressing, rax
		call qword ptr [r11]
		ret
func_addr	dq init_lc, init_lm, init_lr, init_lc, init_lm, init_lr
			dq l_clamp, l_mirror, l_repeat, n_clamp, n_mirror, n_repeat
init_lm:
init_lr:vmovss xmm2, [rsi].stk_modify.wh_repeat.stk_range.w
		vmovss xmm3, [rsi].stk_modify.wh_repeat.stk_range.h
init_lc:vcvtsi2ss xmm0, xmm0, r14
		vcvtsi2ss xmm1, xmm1, r15
		vmulss xmm0, xmm0, xmm2
		vmulss xmm1, xmm1, xmm3
		ret
; адресация
l_mirror:mov rax, r11
		coord_l_mirror r14
		mov rcx, rdx			; x1
		lea rax, [r11 + 1]
		coord_l_mirror r14
		mov r11, rdx			; x2
		lea rax, [r12 + 1]
		coord_l_mirror r15
		mov rdi, rdx			; y2
		mov rax, r12
		coord_l_mirror r15		; y1
		mov rax, r11			; x2
		jmp linear
l_repeat:mov rax, r11
		xor rdx, rdx
		div r14
		mov rcx, rdx			; x1
		mov rax, r12
		xor rdx, rdx
		div r15
		mov rsi, rdx			; y1
		lea rax, [r12 + 1]
		xor rdx, rdx
		div r15
		mov rdi, rdx			; y2
		lea rax, [r11 + 1]
		xor rdx, rdx
		div r14
		mov rax, rdx			; x2
		mov rdx, rsi			; y1
		jmp linear
l_clamp:mov rcx, r11
		mov rdx, r12
		movsxd r11, w_clamp
		movsxd r12, h_clamp
		lea eax, [ecx + 1]
		lea edi, [edx + 1]
		cmp ecx, r11d
		cmovg rcx, r11
		cmp eax, r11d
		cmovg rax, r11
		cmp edx, r12d
		cmovg rdx, r12
		cmp edi, r12d
		cmovg rdi, r12
linear:	
if XMM_LINEAR
		; дает более четкий результат
		vroundps xmm0, xmm15, 11b
		vsubps xmm0, xmm15, xmm0			; fracXY
		vsubps xmm1, xmm8, xmm0				; 1.0 - fracXY
		imul rdx, r13
		imul rdi, r13
		add rdx, r9
		add rdi, r9
		pmovzxbd xmm2, dword ptr [rdx + rcx * 4]	; f1
		pmovzxbd xmm3, dword ptr [rdx + rax * 4]	; f2
		pmovzxbd xmm4, dword ptr [rdi + rcx * 4]	; f3
		pmovzxbd xmm5, dword ptr [rdi + rax * 4]	; f4
		cvtdq2ps xmm2, xmm2
		cvtdq2ps xmm3, xmm3
		cvtdq2ps xmm4, xmm4
		cvtdq2ps xmm5, xmm5
		pshufd xmm6, xmm0, 00000000b				; tx = fracX
		pshufd xmm7, xmm1, 00000000b				; sx = 1.0 - fracX
		mulps xmm2, xmm6							; f1 *= tx
		vfmadd231ps xmm2, xmm3, xmm7				; i1 = f1 + f2 * sx
		mulps xmm4, xmm6							; f3 *= tx
		vfmadd231ps xmm4, xmm5, xmm7				; i2 = f3 + f4 * sx
		pshufd xmm6, xmm0, 01010101b				; ty = fracY
		pshufd xmm7, xmm1, 01010101b				; sy = 1.0 - fracY
		mulps xmm2, xmm6							; i1 *= ty
		vfmadd231ps xmm2, xmm4, xmm7				; res = i1 + i2 * sy
		vdivps xmm0, xmm2, xmm12
		ret ; 26 инструкций
else
		; дает более сглаженный результат и выполняется быстрее в разы
		pxor mm7, mm7
		imul rdx, r13
		imul rdi, r13
		add rdx, r9
		add rdi, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		movd mm3, dword ptr [rdx + rax * 4]
		movd mm4, dword ptr [rdi + rcx * 4]
		movd mm5, dword ptr [rdi + rax * 4]
		punpcklbw mm2, mm7
		punpcklbw mm3, mm7
		punpcklbw mm4, mm7
		punpcklbw mm5, mm7
		paddsw mm2, mm3
		paddsw mm4, mm5
		paddsw mm2, mm4
		psraw mm2, 2
		packuswb mm2, mm2
		unpack_pix
		ret ; 22 инструкции
endif

n_clamp:movsxd rcx, w_clamp
		movsxd rdx, h_clamp
		cmp r11d, ecx
		cmovg r11, rcx
		cmp r12d, edx
		cmovg r12, rdx
		imul r12, r13
		add r12, r9
		movd mm2, dword ptr [r12 + r11 * 4]
		unpack_pix
		ret
n_mirror:mov rax, r11
		xor rdx, rdx
		lea rcx, [r14 * 2]
		div rcx
		lea rax, [rdx + 1]
		sub rcx, rax
		cmp rdx, r14
		cmovge rdx, rcx
@@:		mov rcx, rdx
		mov rax, r12
		xor rdx, rdx
		lea rsi, [r15 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, r15
		cmovge rdx, rsi
		imul rdx, r13
		add rdx, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		unpack_pix
		ret
n_repeat:mov rax, r11
		xor rdx, rdx
		div r14
		mov rcx, rdx
		xor rdx, rdx
		mov rax, r12
		div r15
		imul rdx, r13
		add rdx, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		unpack_pix
		ret
; фильтры
null:	ret
i_normal:
		vrcpss xmm10, xmm10, xmm11
		vmovaps xmm9, f_0_5x8
		mov rax, 3
i_sobel:mov r11, offset _sobel
		or rax, 3
		mov w_mtx, rax
		bsr rax, rax
		shl rax, 4
		lea r11, [r11 + rax - 16]
		mov r12, [r11]
		mov r11, [r11 + 8]
		mov mtx_a, r12
		mov mtx_b, r11
		ret
i_emboss:vmovaps xmm10, f_0_5x8
i_laplac:imul rax, 36
		mov r11, offset mtx_laplac1
		add rax, r11
		mov mtx_a, rax
i_prewit:mov w_mtx, 3
		ret
i_low:	;and rax, -1
i_hi:	mov w_mtx, rax
		imul rax, rax
		vcvtsi2ss xmm10, xmm10, rax
		vpshufd xmm10, xmm10, 0
		vmovaps xmm9, f_3_0x8
		ret
i_rbrts:mov w_mtx, 2
		ret
i_contr:mov r12, r9
		vxorps xmm10, xmm10, xmm10
		mov rax, r15
		imul rax, r14
		vcvtsi2ss xmm0, xmm0, rax
		vshufps xmm0, xmm0, xmm0, 0
		mov rax, r15
i_cont0:mov rdi, r12
		mov rsi, r14
@@:		vpmovzxbd xmm1, dword ptr [rdi]
		vcvtdq2ps xmm1, xmm1
		vdivps xmm1, xmm1, xmm12
		add rdi, 4
		vaddps xmm10, xmm10, xmm1
		dec rsi
		jnz @b
		add r12, r13
		dec rax
		jnz i_cont0
		vdivps xmm10, xmm10, xmm0
		ret
i_gamma:vmovaps xmm10, f_1_0x8
		vmovaps xmm9, f_2_0x8
		vrcpss xmm11, xmm11, xmm11
		vshufps xmm11, xmm11, xmm11, 0
		ret
i_scl_b:vpshufd xmm10, xmm11, 01010101b
		vshufps xmm11, xmm11, xmm11, 0
		ret
clc_mtx:vmovaps xmm7, xmm15
		vxorps xmm1, xmm1, xmm1
		mov rax, w_mtx
		mov rdx, rax
		shr rax, 1
		vcvtsi2ss xmm0, xmm0, rax
		vshufps xmm0, xmm0, xmm0, 0				; 1|1|1|1
		vmovhlps xmm1, xmm1, xmm15				; 0|0|dy|dx
		vmulps xmm0, xmm0, xmm1					; 0|0|dy|dx
		vsubps xmm15, xmm15, xmm0				; dy|dx|y-dy|x-dx
		vmovlhps xmm15, xmm15, xmm15			; y-dy|x-dx|y-dy|x-dx
		mov rbx, offset tmp_mtx
clc_mt0:vmovhlps xmm0, xmm0, xmm15				; 0|0|y-dy|x-dx
		vmovss xmm15, xmm15, xmm0				; y-dy|x-dx|y-dy|x-dx
		push rdx
		mov rcx, w_mtx
@@:		push rcx
		cvtps2dq xmm0, xmm15
		xor rsi, rsi
		vpextrd r11, xmm0, 0
		vpextrd r12, xmm0, 1
		cmp r11d, esi
		cmovl r11, rsi
		cmp r12d, esi
		cmovl r12, rsi
		call addressing
		vmovaps [rbx], xmm0
		vdpps xmm0, xmm0, xmm11, 01110001b
		vmovss dword ptr [rbx + 12], xmm0
		add rbx, 16
		pop rcx
		vaddss xmm15, xmm15, xmm1
		loop @b
		pop rdx
		vaddps xmm15, xmm15, xmm1
		dec rdx
		jnz clc_mt0
		vmovaps xmm15, xmm7
		mov rsi, w_mtx
		imul rsi, rsi
		mov rdi, offset tmp_mtx
		ret
f_none:	cvtps2dq xmm0, xmm15
		vpextrd r11, xmm0, 0
		vpextrd r12, xmm0, 1
		call addressing
		pack_pix
		ret
f_sobel:call clc_mtx
		vxorps xmm1, xmm1, xmm1
		vxorps xmm2, xmm2, xmm2
		mov rdx, mtx_a
		mov rax, mtx_b
		xor rcx, rcx
@@:		vmovss xmm0, dword ptr [rdi + 12]
		vfmadd231ss xmm1, xmm0, dword ptr [rdx + rcx * 4]
		vfmadd231ss xmm2, xmm0, dword ptr [rax + rcx * 4]
		add rdi, 16
		inc rcx
		cmp rcx, rsi
		jb @b
		vmulss xmm1, xmm1, xmm1
		vmulss xmm2, xmm2, xmm2
		vaddss xmm1, xmm1, xmm2
		vsqrtss xmm0, xmm1, xmm1
		vshufps xmm0, xmm0, xmm0, 0
		pack_pix
		ret
f_laplac:call clc_mtx
		vxorps xmm1, xmm1, xmm1
		mov rsi, mtx_a
		mov rcx, 9
@@:		vmovss xmm0, dword ptr [rdi + 12]
		vfmadd231ss xmm1, xmm0, dword ptr [rsi]
		add rdi, 16
		add rsi, 4
		loop @b
		vshufps xmm0, xmm1, xmm1, 0
		pack_pix
		ret
f_prewit:call clc_mtx
		vxorps xmm1, xmm1, xmm1
		vxorps xmm2, xmm2, xmm2
		mov rsi, offset mtx_prewit1
		mov rcx, 9
@@:		vmovss xmm0, dword ptr [rdi + 12]
		vfmadd231ss xmm1, xmm0, dword ptr [rsi]
		vfmadd231ss xmm2, xmm0, dword ptr [rsi + 36]
		add rdi, 16
		add rsi, 4
		loop @b
		vmaxss xmm0, xmm1, xmm2
		vshufps xmm0, xmm0, xmm0, 0
		pack_pix
		ret
f_emboss:call clc_mtx
		vxorps xmm1, xmm1, xmm1
		mov rsi, offset mtx_emboss
		mov rcx, 9
@@:		vmovss xmm0, dword ptr [rdi + 12]
		vfmadd231ss xmm1, xmm0, dword ptr [rsi]
		add rdi, 16
		add rsi, 4
		loop @b
		vshufps xmm0, xmm1, xmm1, 0
		vaddps xmm0, xmm0, xmm10
		pack_pix
		ret
f_nrml:	call clc_mtx
		vxorps xmm1, xmm1, xmm1
		vxorps xmm2, xmm2, xmm2
		mov rdx, mtx_a
		mov rax, mtx_b
		xor rcx, rcx
@@:		vmovss xmm0, dword ptr [rdi + 12]
		vfmadd231ss xmm1, xmm0, dword ptr [rdx + rcx * 4]
		vfmadd231ss xmm2, xmm0, dword ptr [rax + rcx * 4]
		add rdi, 16
		inc rcx
		cmp rcx, rsi
		jb @b
		vshufps xmm0, xmm1, xmm2, 00000000b	; dv|dv|du|du
		vpsrldq xmm0, xmm0, 4				; 0|dv|dv|du
		vshufps xmm0, xmm0, xmm10, 00000100b; 0.0625|0.0625|dv|du
		vdpps xmm1, xmm0, xmm0, 01110001b	; tmp = dot
		vrsqrtss xmm1, xmm1, xmm1			; tmp = 1 / sqrt(tmp)
		vpshufd xmm1, xmm1, 0
		vmulps xmm0, xmm0, xmm1				; v *= tmp
		vmulps xmm0, xmm0, xmm9				; v *= 0.5
		vaddps xmm0, xmm0, xmm9				; v += 0.5
		pack_pix
		ret
f_hi:	call clc_mtx
		mov r11, rdi
		vxorps xmm1, xmm1, xmm1
		mov rcx, rsi
@@:		vaddps xmm1, xmm1, [rdi]
		add rdi, 16
		loop @b
		shr rsi, 1
		shl rsi, 4
		vdivps xmm1, xmm1, xmm10
		vmovaps xmm0, [rsi + r11]
		vsubps xmm0, xmm0, xmm1
		vmulps xmm0, xmm0, xmm9
		vaddps xmm0, xmm0, xmm1
		pack_pix
		ret
f_low:	call clc_mtx
		mov r11, rdi
		vxorps xmm1, xmm1, xmm1
		mov rcx, rsi
@@:		vaddps xmm1, xmm1, [rdi]
		add rdi, 16
		loop @b
		vdivps xmm0, xmm1, xmm10
		pack_pix
		ret
f_median:call clc_mtx
		lea rax, [rsi * 8]
		lea r12, [rdi + rax * 2]
		mov rbx, rsi
		shr rbx, 1
		lea rax, [rbx * 8]
		lea rbx, [rdi + rax * 2]
median0:vmovss xmm0, flt_max
		lea rax, [rdi + 16]
		mov r11, rdi
@@:		cmp r11, r12
		jae @f
		vmovss xmm1, dword ptr [r11 + 12]
		add r11, 16
		vucomiss xmm0, xmm1
		jbe @b
		vmovss xmm0, xmm0, xmm1
		mov rax, r11
		jmp @b
@@:		sub rax, 16
		vmovaps xmm0, [rax]
		vmovaps xmm1, [rdi]
		vmovaps [rax], xmm1
		vmovaps [rdi], xmm0
		add rdi, 16
		cmp rdi, r12
		jb median0
		movaps xmm0, [rbx]
		pack_pix
		ret
f_rbrts:call clc_mtx
		vmovss xmm0, dword ptr [rdi + 0 * 16 + 12]
		vsubss xmm0, xmm0, dword ptr [rdi + 3 * 16 + 12]
		vmovss xmm1, dword ptr [rdi + 1 * 16 + 12]
		vsubss xmm1, xmm1, dword ptr [rdi + 2 * 16 + 12]
		vmulss xmm0, xmm0, xmm0
		vmulss xmm1, xmm1, xmm1
		vaddss xmm0, xmm0, xmm1
		vsqrtss xmm0, xmm0, xmm0
		vshufps xmm0, xmm0, xmm0, 0
		pack_pix
		ret
f_max:	call clc_mtx
		vxorps xmm0, xmm0, xmm0
		mov r11, rdi
		mov rax, 1
		xor rcx, rcx
@@:		cmp rcx, rsi
		jae @f
		vmovss xmm1, dword ptr [rdi + 12]
		add rdi, 16
		inc rcx
		vucomiss xmm0, xmm1
		jae @b
		vmovss xmm0, xmm0, xmm1
		mov rax, rcx
		jmp @b
@@:		shl rax, 4
		vmovaps xmm0, [r11 + rax - 16]
		pack_pix
		ret
f_min:	call clc_mtx
		lea r11, [rdi - 12]
		vmovss xmm0, flt_max
		mov r11, rdi
		mov rax, 1
		xor rcx, rcx
@@:		cmp rcx, rsi
		jae @f
		vmovss xmm1, dword ptr [rdi + 12]
		add rdi, 16
		inc rcx
		vucomiss xmm0, xmm1
		jbe @b
		vmovss xmm0, xmm0, xmm1
		mov rax, rcx
		jmp @b
@@:		shl rax, 4
		vmovaps xmm0, [r11 + rax - 16]
		pack_pix
		ret
;NewY:=K*(OldY-AveY)+AveY
f_contr:call f_none
		vsubps xmm0, xmm0, xmm10
		vmulps xmm0, xmm0, xmm11
		vaddps xmm0, xmm0, xmm10
		pack_pix
		ret
f_bin:	call f_none
		vdpps xmm0, xmm0, xmm13, 01110001b
		vucomiss xmm0, xmm11
		setnc al
		movzx rax, al
		shl rax, 4
		mov rdi, offset dataBin
		vmovaps xmm0, [rax + rdi]
		pack_pix
		ret
; rgba = pow(rgba, 1/gamma)
f_gamma:call f_none
		vmovaps xmm1, xmm0
		vmovaps xmm2, xmm11
		vmovaps xmm0, xmm10
		mov rcx, 12						; 12 - точность до 3 знака, 20 - до 6 знака
@@:		vsqrtps xmm1, xmm1
		vroundps xmm3, xmm2, 11b
		vsubps xmm2, xmm2, xmm3
		vmulps xmm2, xmm2, xmm9
		vcmpps xmm3, xmm2, xmm10, 5
		vandps xmm4, xmm1, xmm3
		vandnps xmm3, xmm3, xmm10
		vorps xmm3, xmm3, xmm4
		vmulps xmm0, xmm0, xmm3
		loop @b
		pack_pix
		ret
f_scl_b:call f_none
		vmulps xmm0, xmm0, xmm11	; scale
		vaddps xmm0, xmm0, xmm10	; bias
		pack_pix
		ret
OPTION EPILOGUE:EPILOGUEDEF
asm_ssh_copy endp

end
