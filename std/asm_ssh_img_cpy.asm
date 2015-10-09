
include asm_ssh.inc

coor_l_mirror macro
		xor rdx, rdx
		lea rsi, [rbx * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, rbx
		cmovge rdx, rsi
endm

unpack_pix macro
		movq2dq xmm0, mm2
		pmovzxbd xmm0, xmm0
		cvtdq2ps xmm0, xmm0
		divps xmm0, xmm12
endm

.const

sub_alpha	dw 256, 256, 256, 256
_mm_alpha	dq 00000000ff000000h
_alpha_not	dw 255,255,255,255
_mm_not		dq -1
_func_ops	dq _add, _sub, _set, _xor, _and, _or, _lum, _not, _alph, _fix_al, _mul, _lum_add, _lum_sub, _norm

mtxPrewit	dd -1, 0, 1
			dd -1, 0, 1
			dd -1, 0, 1
			dd -1, -1, -1
			dd 0, 0, 0
			dd 1, 1, 1
mtxPrewit1	dd -1.0, 0.0, 1.0
			dd -1.0, 0.0, 1.0
			dd -1.0, 0.0, 1.0
mtxPrewit2	dd -1.0, -1.0, -1.0
			dd 0.0,  0.0,  0.0
			dd 1.0,  1.0,  1.0
mtxEmboss	dd 0.0,  1.0,  0.0
			dd -1.0, 0.0, 1.0
			dd 0.0, -1.0, 0.0
mtxLaplacian dd -1.0,-2.0,-1.0
			 dd -2.0,12.0,-2.0
			 dd -1.0,-2.0,-1.0
mtxSobel3x3_1 dd 1.0,0.0,-1.0
			  dd 2.0,0.0,-2.0
			  dd 1.0,0.0,-1.0
mtxSobel3x3_2 dd 1.0,2.0,1.0
			  dd 0.0,0.0,0.0
			  dd -1.0,-2.0,-1.0
mtxSobel5x5_1 dd 1.0,2.0,0.0,-2.0,-1.0
			  dd 2.0,3.0,0.0,-3.0,-2.0
			  dd 3.0,4.0,0.0,-4.0,-3.0
			  dd 2.0,3.0,0.0,-3.0,-2.0
			  dd 1.0,2.0,0.0,-2.0,-1.0
mtxSobel5x5_2 dd 1.0,2.0,3.0,2.0,1.0
			  dd 2.0,3.0,4.0,3.0,2.0
			  dd 0.0,0.0,0.0,0.0,0.0
			  dd -2.0,-3.0,-4.0,-3.0,-2.0
			  dd -1.0,-2.0,-3.0,-2.0,-1.0
mtxSobel7x7_1 dd 1.0,2.0,3.0,0.0,-3.0,-2.0,-1.0
			  dd 2.0,3.0,4.0,0.0,-4.0,-3.0,-2.0
			  dd 3.0,4.0,5.0,0.0,-5.0,-4.0,-3.0
			  dd 4.0,5.0,6.0,0.0,-6.0,-5.0,-4.0
			  dd 3.0,4.0,5.0,0.0,-5.0,-4.0,-3.0
			  dd 2.0,3.0,4.0,0.0,-4.0,-3.0,-2.0
			  dd 1.0,2.0,3.0,0.0,-3.0,-2.0,-1.0
mtxSobel7x7_2 dd 1.0,2.0,3.0,4.0,3.0,2.0,1.0
			  dd 2.0,3.0,4.0,5.0,4.0,3.0,2.0
			  dd 3.0,4.0,5.0,6.0,5.0,4.0,3.0
			  dd 0.0,0.0,0.0,0.0,0.0,0.0,0.0
			  dd -3.0,-4.0,-5.0,-6.0,-5.0,-4.0,-3.0
			  dd -2.0,-3.0,-4.0,-5.0,-4.0,-3.0,-2.0
			  dd -1.0,-2.0,-3.0,-4.0,-3.0,-2.0,-1.0
mtxSobel9x9_1 dd 1.0,2.0,3.0,4.0,0.0,-4.0,-3.0,-2.0,-1.0
			  dd 2.0,3.0,4.0,5.0,0.0,-5.0,-4.0,-3.0,-2.0
			  dd 3.0,4.0,5.0,6.0,0.0,-6.0,-5.0,-4.0,-3.0
			  dd 4.0,5.0,6.0,7.0,0.0,-7.0,-6.0,-5.0,-4.0
			  dd 5.0,6.0,7.0,8.0,0.0,-8.0,-7.0,-6.0,-5.0
			  dd 4.0,5.0,6.0,7.0,0.0,-7.0,-6.0,-5.0,-4.0
			  dd 3.0,4.0,5.0,6.0,0.0,-6.0,-5.0,-4.0,-3.0
			  dd 2.0,3.0,4.0,5.0,0.0,-5.0,-4.0,-3.0,-2.0
			  dd 1.0,2.0,3.0,4.0,0.0,-4.0,-3.0,-2.0,-1.0
mtxSobel9x9_2 dd 1.0,2.0,3.0,4.0,5.0,4.0,3.0,2.0,1.0
			  dd 2.0,3.0,4.0,5.0,6.0,5.0,4.0,3.0,2.0
			  dd 3.0,4.0,5.0,6.0,7.0,6.0,5.0,4.0,3.0
			  dd 4.0,5.0,6.0,7.0,8.0,7.0,6.0,5.0,4.0
			  dd 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0
			  dd -4.0,-5.0,-6.0,-7.0,-8.0,-7.0,-6.0,-5.0,-4.0
			  dd -3.0,-4.0,-5.0,-6.0,-7.0,-6.0,-5.0,-4.0,-3.0
			  dd -2.0,-3.0,-4.0,-5.0,-6.0,-5.0,-4.0,-3.0,-2.0
			  dd -1.0,-2.0,-3.0,-4.0,-5.0,-4.0,-3.0,-2.0,-1.0

.data?
_alpha0		dq ?
_alpha1		dq ?

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
		vpackssdw xmm0, xmm0, xmm0
		vpackuswb xmm0, xmm0, xmm0
		movdq2q mm5, xmm0
		por mm2, mm5
		pand mm2, mm0
		pand mm3, mm1
		por mm2, mm3
		ret

asm_ssh_copy proc USES rbx rsi rdi r12 r13 r14 r15 src_bar:QWORD, src_wh:QWORD, src:QWORD, dst:QWORD, dst_bar:QWORD, dst_wh:QWORD, modify:QWORD
local w_clamp:QWORD, h_clamp:QWORD, addressing:QWORD, filter:QWORD, operation:QWORD
		call init_base_modify								; инициализация основных параметров модификатора
		; обрезка координат
		call asm_clip_bar
		jc _fin
		mov r14, rcx										; w_src
		mov r15, rdx										; h_src
		mov r13, r10										; pitch_src
		dec rcx
		dec rdx
		mov w_clamp, rcx
		mov h_clamp, rdx
		mov rcx, dst_bar
		mov rdx, dst_wh
		mov rax, r8
		mov r8, r9
		vcvtsi2ss xmm14, xmm14, dword ptr [rcx].stk_bar.w
		vcvtsi2ss xmm15, xmm15, dword ptr [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		vcvtsi2ss xmm0, xmm0, rcx							; для nearest адресации
		vcvtsi2ss xmm1, xmm1, rdx
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
_fin:	ret
OPTION EPILOGUE:NONE
init_base_modify:
		mov rsi, modify
		vmovaps xmm13, _gamma
		vmovaps xmm12, f_255x8
		vmovaps xmm11, [rsi].stk_modify.flt_vec
		vxorps xmm10, xmm10, xmm10
		vmovss xmm0, [rsi].stk_modify.alpha
		vmulss xmm0, xmm0, f_256x8
		movq mm1, qword ptr sub_alpha
		cvtps2pi mm0, xmm0
		psubusw mm1, mm0
		movq _alpha0, mm0
		movq _alpha1, mm1
		ret
init_filters:
		mov r11, offset func_flt
		movsxd rax, [rsi].stk_modify.type_filter
		lea r11, [r11 + rax * 8]
		mov rax, [r11 + 128]
		mov filter, rax
		call qword ptr [r11]
		ret
func_flt	dq null,	i_sobel, i_laplac, i_prewit, i_emboss, i_normal, i_hi, i_low, i_median, i_roberts,	i_max, i_min, i_contrast, i_binary, i_gamma, i_scale_bias
			dq f_none,	f_sobel, f_laplac, f_prewit, f_emboss, f_normal, f_hi, f_low, f_median, f_roberts,	f_max, f_min, f_contrast, f_binary,	f_famma, f_Scale_bias
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
func_addr	dq init_lc, init_lm, init_lr, init_nr, init_nr, init_nr
			dq l_clamp, l_mirror, l_repeat, n_clamp, n_mirror, n_repeat
init_nr:ret
init_lm:
init_lr:vmovss xmm2, [rsi].stk_modify.wh_repeat.stk_range.w
		vmovss xmm3, [rsi].stk_modify.wh_repeat.stk_range.h
init_lc:vcvtsi2ss xmm0, xmm0, r14
		vcvtsi2ss xmm1, xmm1, r15
		vmulss xmm0, xmm0, xmm2
		vmulss xmm1, xmm1, xmm3
		ret
; адресация
l_mirror:cvttps2pi mm2, xmm15
		pextrw r11, mm2, 0
		pextrw r12, mm2, 2
		mov rax, r11
		mov rbx, r14
		coor_l_mirror
		mov rcx, rdx			; x1
		lea rax, [r11 + 1]
		coor_l_mirror
		mov r11, rdx			; x2
		lea rax, [r12 + 1]
		mov rbx, r15
		coor_l_mirror
		mov rdi, rdx			; y2
		mov rax, r12
		coor_l_mirror			; y1
		mov rax, r11			; x2
		jmp linear
l_repeat:cvttps2pi mm2, xmm15
		pextrw r11, mm2, 0
		pextrw r12, mm2, 2
		mov rax, r11
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
l_clamp:cvttps2pi mm2, xmm15
		pextrw rcx, mm2, 0
		pextrw rdx, mm2, 2
		mov r11, w_clamp
		mov r12, h_clamp
		xor rsi, rsi
		lea rax, [rcx + 1]
		lea rdi, [rdx + 1]
		cmp rcx, rsi
		cmovl rcx, rsi
		cmp rdx, rsi
		cmovl rdx, rsi
		cmp rcx, r11
		cmovg rcx, r11
		cmp rdx, r12
		cmovg rdx, r12
		cmp rax, rsi
		cmovl rax, rsi
		cmp rdi, rsi
		cmovl rdi, rsi
		cmp rax, r11
		cmovg rax, r11
		cmp rdi, r12
		cmovg rdi, r12
linear:	movd r11, mm6
		pxor mm7, mm7
		cvtpi2ps xmm0, mm2
		vsubps xmm0, xmm15, xmm0
		vmulps xmm0, xmm0, xmm12
		cvtps2pi mm6, xmm0
		pslld mm6, 8
		imul rdx, r13
		imul rdi, r13
		add rdx, r9
		add rdi, r9
		movd mm2, dword ptr [rdi + rax * 4]
		movd mm3, dword ptr [rdi + rcx * 4]
		punpcklbw mm2, mm7
		punpcklbw mm3, mm7
		psubsw mm2, mm3
		movd mm4, dword ptr [rdx + rax * 4]
		movd mm5, dword ptr [rdx + rcx * 4]
		punpcklbw mm4, mm7
		punpcklbw mm5, mm7
		psubsw mm4, mm5
		pshufw mm7, mm6, 10101010b	; yy
		pshufw mm6, mm6, 0			; xx
		pmulhw mm2, mm6
		pmulhw mm4, mm6
		paddsw mm2, mm3
		paddsw mm4, mm5
		psubsw mm2, mm4
		pmulhw mm2, mm7
		paddsw mm2, mm4
		packuswb mm2, mm2
		movd mm6, r11
		unpack_pix
		ret
n_clamp:cvttps2pi mm2, xmm15
		pextrw rcx, mm2, 0
		pextrw rdx, mm2, 2
		mov r11, w_clamp
		mov r12, h_clamp
		xor rax, rax
		cmp rcx, rax
		cmovl rcx, rax
		cmp rdx, rax
		cmovl rdx, rax
		cmp rcx, r11
		cmovg rcx, r11
		cmp rdx, r12
		cmovg rdx, r12
		imul rdx, r13
		add rdx, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		unpack_pix
		ret
n_mirror:cvtps2pi mm2, xmm15
		pextrw rax, mm2, 0			; x
		xor rdx, rdx
		lea rcx, [r14 * 2]
		div rcx
		lea rax, [rdx + 1]
		sub rcx, rax
		cmp rdx, r14
		cmovge rdx, rcx
@@:		mov rcx, rdx
		pextrw rax, mm2, 2			; y
		xor rdx, rdx
		lea r11, [r15 * 2]
		div r11
		lea rax, [rdx + 1]
		sub r11, rax
		cmp rdx, r15
		cmovge rdx, r11
		imul rdx, r13
		add rdx, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		unpack_pix
		ret
n_repeat:cvttps2pi mm2, xmm15
		pextrw rax, mm2, 0
		xor rdx, rdx
		div r14
		mov rcx, rdx
		pextrw rax, mm2, 2
		xor rdx, rdx
		div r15
		imul rdx, r13
		add rdx, r9
		movd mm2, dword ptr [rdx + rcx * 4]
		unpack_pix
		ret
		; фильтры
null:
i_sobel:
i_laplac:
i_prewit:
i_emboss:
i_normal:
i_hi:
i_low:
i_median:
i_roberts:
i_max:
i_min:
i_contrast:
i_binary:
i_gamma:
i_scale_bias:
		ret
f_none:
f_sobel:
f_laplac:
f_prewit:
f_emboss:
f_normal:
f_hi:
f_low:
f_median:
f_roberts:
f_max:
f_min:
f_contrast:
f_binary:
f_famma:
f_Scale_bias:
		call addressing
		ret
OPTION EPILOGUE:EPILOGUEDEF
asm_ssh_copy endp

end

iSobelN:mov rax, whMtx
		mov rsi, offset mtxSobel3x3_1
		mov rdi,offset mtxSobel3x3_2
		cmp rax,3
		jbe @f
		mov rsi, offset mtxSobel5x5_1
		mov rdi, offset mtxSobel5x5_2
		cmp rax, 5
		jz @f
		mov rsi, offset mtxSobel7x7_1
		mov rdi, offset mtxSobel7x7_2
		cmp rax, 7
		jz @f
		mov rsi, offset mtxSobel9x9_1
		mov rdi, offset mtxSobel9x9_2
@@:		mov @@mtxA, rsi
		mov @@mtxB, rdi
		movss xmm10, fltFactor
		ret
iEmboss:movaps xmm10, _fp0_5x4
iLP:	mov whMtx, 3
		ret
iLowHI:	mov rax, whMtx
		mov rsi, offset _fp1_0
		cmp rax, 1
		jz @f
		mov rsi, offset _fp9_0
		cmp rax, 3
		jz @f
		mov rsi, offset _fp25_0
		cmp rax, 5
		jz @f
		mov rsi, offset _fp49_0
		cmp rax, 7
		jz @f
		mov rsi, offset _fp81_0
		cmp rax, 9
		jz @f
		mov rsi, offset _fp121_0
@@:		mov @@div, rsi
		movaps xmm10, _fp3_0x4
		ret
iRoberts:mov whMtx, 2
		ret
iGamma:	movaps xmm10, _fp1_0
		movaps xmm9, _fp2_0x4
		movaps xmm8, _fp1_0
		ret
iContr:	push rcx
		xorps xmm10, xmm10
		mov rsi, r9
		mov rax, r15
		imul rax, r14
		cvtsi2ss xmm0, rax
		shufps xmm0, xmm0, 0
		mov rax, r15
iContr0:mov rdi, rsi
		mov rcx, r14
@@:		movd xmm1, dword ptr [rdi]
		punpcklbw xmm1, xmm15
		punpcklwd xmm1, xmm15
		cvtdq2ps xmm1, xmm1
		divps xmm1, xmm14
		add rdi, 4
		addps xmm10, xmm1
		loop @b
		add rsi, rbx
		dec rax
		jnz iContr0
		divps xmm10, xmm0
		;shufps xmm12, xmm12, 0
		pop rcx
		ret
iScaleB:pshufd xmm2, xmm12, 01010101b
		shufps xmm12, xmm12, 0
		ret
fMtx:	push r8
		movaps xmm6, xmm11
		xorps xmm2, xmm2
		mov rax, whMtx
		shr rax, 1
		cvtsi2ss xmm0, eax
		movhlps xmm2, xmm11
		shufps xmm0, xmm0, 0
		mulps xmm0, xmm2
		subps xmm11, xmm0
		movlhps xmm11, xmm11
		mov r8, offset mtx
		mov rdx, whMtx
_mtx:	movhlps xmm0, xmm11
		movss xmm11, xmm0
		push rdx
		mov rcx, whMtx
@@:		push rcx
		call r11
		movaps [r8], xmm0
		dpps xmm0, xmm12, 01110001b
		movss dword ptr [r8 + 12], xmm0
		add r8, 16
		pop rcx
		addss xmm11, xmm2
		loop @b
		pop rdx
		addps xmm11, xmm2
		dec rdx
		jnz _mtx
		movaps xmm11, xmm6
		pop r8
		mov rsi, whMtx
		mov rdi, offset mtx + 12
		ret
fNone:	call r11
pckXMM0:mulps xmm0, xmm14
		cvtps2dq xmm0, xmm0
		packssdw xmm0, xmm0
		packuswb xmm0, xmm0
		movdq2q mm0, xmm0
		ret
fSobel:	call fMtx
		xorps xmm0, xmm0
		xorps xmm3, xmm3
		mov rdx, @@mtxA
		mov rax, @@mtxB
		imul rsi, rsi
		mov rcx, rsi
@@:		movss xmm1, dword ptr [rdi]
		movss xmm2, xmm1
		mulss xmm1, dword ptr [rdx]
		mulss xmm2, dword ptr [rax]
		addss xmm0, xmm1
		addss xmm3, xmm2
		add rdi, 16
		add rdx, 4
		add rax, 4
		loop @b
		mulss xmm0, xmm0
		mulss xmm3, xmm3
		addss xmm0, xmm3
		sqrtss xmm0, xmm0
		shufps xmm0, xmm0, 0
		jmp pckXMM0
fNormal:call fMtx
		xorps xmm0, xmm0
		xorps xmm2, xmm2
		mov rdx, @@mtxA
		mov rax, @@mtxB
		imul rsi, rsi
		mov rcx, rsi
@@:		movss xmm1, dword ptr [rdi]
		mulss xmm1, dword ptr [rdx]
		addss xmm2, xmm1					;du
		movss xmm1, dword ptr [rdi]
		mulss xmm1, dword ptr [rax]
		addss xmm0, xmm1					;dv
		add rdi, 16
		add rdx, 4
		add rax, 4
		loop @b
		shufps xmm0, xmm2, 11000000b		;00|00|20|??
		movss xmm0, xmm3					;30|00|20|??
		jmp pckXMM0
fLaplac:call fMtx
		xorps xmm0, xmm0
		mov rsi, offset mtxLaplacian
		mov rcx, 9
@@:		movss xmm1, dword ptr [rdi]
		mulss xmm1, dword ptr [rsi]
		addss xmm0, xmm1
		add rdi, 16
		add rsi, 4
		loop @b
		shufps xmm0, xmm0, 0
		jmp pckXMM0
fPrewit:call fMtx
		xorps xmm0, xmm0
		xorps xmm2, xmm2
		mov rsi, offset mtxPrewit1
		mov rax, rdi
		mov rcx, 9
@@:		movss xmm1, dword ptr [rdi]
		mulss xmm1, dword ptr [rsi]
		addss xmm0, xmm1
		add rdi, 16
		add rsi, 4
		loop @b
		mov rcx, 9
@@:		movss xmm1, dword ptr [rax]
		mulss xmm1, dword ptr [rsi]
		addss xmm2, xmm1
		add rax, 16
		add rsi, 4
		loop @b
		maxss xmm0, xmm2
		shufps xmm0, xmm0, 0
		jmp pckXMM0
fEmboss:call fMtx
		xorps xmm0, xmm0
		mov rsi, offset mtxEmboss
		mov ecx, 9
@@:		movss xmm1, dword ptr [rdi]
		mulss xmm1, dword ptr [rsi]
		addss xmm0, xmm1
		add rdi, 16
		add rsi, 4
		loop @b
		shufps xmm0, xmm0, 0
		addps xmm0, xmm10
		jmp pckXMM0
fHi:	call fMtx
		imul rsi, rsi
		mov rdi, offset mtx
		xorps xmm1, xmm1
		mov rcx, rsi
@@:		addps xmm1, [rdi]
		add rdi, 16
		loop @b
		shr rsi, 1
		shl rsi, 4
		mov rax, @@div
		mov rdi, offset mtx
		movaps xmm0, [rsi + rdi]
		divps xmm1, [rax]
		subps xmm0, xmm1
		mulps xmm0, xmm3
		addps xmm0, xmm1
		jmp pckXMM0
fLow:	call fMtx
		imul rsi, rsi
		mov rdi, offset mtx
		xorps xmm0, xmm0
		mov rcx, rsi
@@:		addps xmm0, [rdi]
		add rdi, 16
		loop @b
		mov rax, @@div
		divps xmm0, [rax]
		jmp pckXMM0
fMedian:call fMtx
		imul rsi, rsi
		shr rsi, 1
		inc rsi
		xor rcx, rcx
median0:movss xmm0, dword ptr [rcx * 4 + rdi]
		movss xmm2, xmm0
		mov rax, rcx
		lea rdx, [rcx + 1]
median1:cmp rdx, rsi
		jae median3
		movss xmm1, dword ptr [rdx * 4 + rdi]
		ucomiss xmm0, xmm1
		jnc @f
		movss xmm0, xmm1
		mov rax, rdx
@@:		inc rdx
		jmp median1
median3:movss dword ptr [rcx * 4 + rdi], xmm0
		movss dword ptr [rax * 4 + rdi], xmm2
		inc rcx
		cmp rcx, rsi
		jl median0
		shl rax, 4
		movaps xmm0, [rax + rdi - 12]
		jmp pckXMM0
fRoberts:call fMtx
		movss xmm0, dword ptr [rdi + 0 * 16]
		subss xmm0, dword ptr [rdi + 3 * 16]
		movss xmm1, dword ptr [rdi + 2 * 16]
		subss xmm1, dword ptr [rdi + 1 * 16]
		mulss xmm0, xmm0
		mulss xmm1, xmm1
		addss xmm0, xmm1
		sqrtss xmm0, xmm0
		shufps xmm0, xmm0, 0
		jmp pckXMM0
fMax:	call fMtx
		imul rsi, rsi
		movss xmm0, dword ptr [rdi]
		xor rax, rax
		mov rcx, 1
		add rdi, 16
max0:	movss xmm1, dword ptr [rdi]
		ucomiss xmm0, xmm1
		jnc @f
		movss xmm0, xmm1
		mov rax, rcx
@@:		add rdi, 16
		inc rcx
		cmp rcx, rsi
		jl max0
		shl rax, 4
		mov rdi, offset mtx
		movaps xmm0, [rdi + rax]
		jmp pckXMM0
fMin:	call fMtx
		imul rsi, rsi
		movss xmm0, dword ptr [rdi]
		xor rax, rax
		mov rcx, 1
		add rdi, 16
min0:	movss xmm1, dword ptr [rdi]
		ucomiss xmm0, xmm1
		jc @f
		movss xmm0, xmm1
		mov rax, rcx
@@:		add rdi, 16
		inc rcx
		cmp rcx, rsi
		jl min0
		shl rax, 4
		mov rdi, offset mtx
		movaps xmm0, [rdi + rax]
		jmp pckXMM0
;NewY:=K*(OldY-AveY)+AveY
fContr:	call r11
		subps xmm0, xmm10
		mulps xmm0, xmm12
		addps xmm0, xmm10
		jmp pckXMM0
fBin:	call r11
		dpps xmm0, xmm13, 01110001b
		ucomiss xmm0, xmm12
		setnc al
		movzx rax, al
		shl rax, 4
		mov rdi, offset dataBin
		movaps xmm0, [rax + rdi]
		jmp pckXMM0
fGamma:	call r11
		movaps xmm1, xmm0
		rcpss xmm2, xmm12
		shufps xmm2, xmm2, 0
		movaps xmm0, xmm10
		mov rcx, 15
@@:		sqrtps xmm1, xmm1
		cvttps2dq xmm3, xmm2
		cvtdq2ps xmm3, xmm3
		subps xmm2, xmm3
		mulps xmm2, xmm9
		movaps xmm3, xmm2
		cmpps xmm3, xmm8, 5
		movaps xmm4, xmm1
		andps xmm4, xmm3
		andnps xmm3, xmm10
		orps xmm3, xmm4
		mulps xmm0, xmm3
		loop @b
		jmp pckXMM0
fScaleB:call r11
		mulps xmm0, xmm12		; scale
		addps xmm0, xmm2		; bias
		jmp pckXMM0
