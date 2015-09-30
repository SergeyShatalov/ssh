
include asm_ssh.inc

.const

align 16
fp_256x4 dd 256.0, 256.0, 256.0, 256.0

.data?

.code

asm_ssh_copy proc USES rbx rsi rdi r12 r13 r14 r15 src_bar:QWORD, src_wh:QWORD, src:QWORD, dst:QWORD, dst_bar:QWORD, dst_wh:QWORD, modify:QWORD
local w_clamp:QWORD, h_clamp:QWORD, addressing:QWORD, filter:QWORD, operation:QWORD, alpha0:QWORD, alpha1:QWORD
		call init_base_modify								; инициализация основных параметров модификатора
		; обрезка координат
		call asm_clip_bar
		jc _fin
		mov r14, rcx										; w_src
		mov r15, rdx										; h_src
		mov rbx, r10										; pitch_src
		dec rcx
		dec rdx
		mov w_clamp, rcx
		mov h_clamp, rdx
		lea rcx, dst_bar
		mov rdx, dst_wh
		mov rax, r8
		mov r8, r9
		vcvtsi2ss xmm14, xmm14, dword ptr [rcx].stk_bar.w
		vcvtsi2ss xmm15, xmm15, dword ptr [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		vcvtsi2ss xmm0, xmm0, rcx
		vcvtsi2ss xmm1, xmm0, rdx
		mov r9, rax											; r8 - dst, r9 - src, r10 - pitch_dst, rbx - pitch_src
		call init_addressing								; инициализация адресации координат(mirror, repeat, clamp)
		vdivss xmm0, xmm0, xmm14							; dx
		vdivss xmm1, xmm1, xmm15							; dy
		; сдвинуть на clip.x, clip.y
		vshufps xmm15, xmm1, xmm0, 00000000b				; dy|dy|dx|dx
		vcvtsi2ss xmm14, xmm14, _clip.y
		vmulss xmm14, xmm14, xmm1							; y
		vshufps xmm15, xmm15, xmm14, 10000000b				; dy|dx|y|y
		vcvtsi2ss xmm14, xmm14, _clip.x
		vmulss xmm14, xmm14, xmm0							; x
		call init_filters
		call init_operation
		; главный цикл
_loop_:	push rcx
		push rdx
		push r8
		vmovss xmm15, xmm15, xmm14							; dy|dx|y|x
@@:		call filter
		call operation
		vmovhlps xmm0, xmm0, xmm15							; 0|0|dy|dx
		vaddss xmm15, xmm15, xmm0							; dy|dx|y|x+dx
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
OPTION PROLOGUE:NONE
sub_alpha dw 256, 256, 256, 256
init_base_modify:
		mov rsi, modify
		vmovaps xmm13, _gamma
		vmovaps xmm12, f_255x8
		vmovaps xmm11, [rsi].stk_modify.flt_vec
		vxorps xmm10, xmm10, xmm10
		vpshufd xmm0, [rsi].stk_modify.alpha, 00000000b
		vmulps xmm0, xmm0, f_256x8
		movq mm1, qword ptr sub_alpha
		cvtps2pi mm0, xmm0
		psubusw mm1, mm0
		movq alpha0, mm0
		movq alpha1, mm1
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
;		movq mm4, mm0
;		movd mm1, dword ptr [r8]
;		movq mm6, _mm1000
;		call r13											; func_ops
;		movd mm2, msk
;		pand mm1, mm2
;		pandn mm2, mm0
;		por mm1, mm2
;		movd dword ptr [r8], mm1
		mov r11, offset func_ops
		movsxd rax, [rsi].stk_modify.src_ops
		lea rax, [r11 + rax * 8]
		mov operation, rax
		ret
func_ops	dq _add, _sub, _set, _xor, _and, _or, _lum, _not, _alph, _fix_alph, _mul, _lum_add, _lum_sub, _norm
init_addressing:
		mov r11, offset func_addr
		movsxd rax, [rsi].stk_modify.type_address
		lea r11, [r11 + rax * 8]
		mov rax, [r11 + 48]
		mov addressing, rax
		call qword ptr [r11]
		ret
func_addr	dq init_lc, init_lm, init_lp, init_nr, init_nr, init_nr
			dq l_clamp, l_mirror, l_repeat, n_clamp, n_mirror, n_repeat
init_nr:
init_nn:
		ret
init_lm:
init_lp:vdivss xmm0, xmm0, [rsi].stk_modify.wh_repeat.stk_range.w
		vdivss xmm1, xmm1, [rsi].stk_modify.wh_repeat.stk_range.h
		ret
init_lc:vcvtsi2ss xmm0, xmm0, r14
		vcvtsi2ss xmm1, xmm1, r15
		ret
l_clamp:
l_mirror:
l_repeat:
n_clamp:
n_mirror:
n_repeat:
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
		ret
		; операции
_add:
_sub:
_set:
_xor:
_and:
_or:
_lum:
_not:
_alph:
_fix_alph:
_mul:
_lum_add:
_lum_sub:
_norm:
		ret

asm_ssh_copy endp

end

asmCopy proc USES rbx rsi rdi r12 r13 r14 r15 dstBar:QWORD, dstClip:QWORD, dst:QWORD, src:QWORD, srcBar:QWORD, srcClip:QWORD, ops:QWORD, adr:QWORD, filter:QWORD, msk:QWORD, nWrap:QWORD, fltVec:QWORD, alpha:DWORD, whMtx:QWORD
LOCAL @@width:QWORD, @@height:QWORD, @@mtxA:QWORD, @@mtxB:QWORD, @@div:QWORD, @@tmpBuf:QWORD
		; проверка на копирование в "себя"
		mov @@tmpBuf, 0
		cmp r8, r9
		jnz @f
		push rcx
		push rdx
		push r8
		mov rsi, r9
		mov rdx, srcClip
		mov rax, [rdx + 00]
		imul rax, [rdx + 08]
		push rax
		lea rcx, [rax * 4]
		sub rsp, 32
		call malloc
		add rsp, 32
		mov @@tmpBuf, rax
		mov r9, rax
		pop rcx
		pop r8
		mov rdi, rax
		rep movsd
		pop rdx
		pop rcx
@@:		; начальная инициализация
		mov rax, fltVec
		xorps xmm15, xmm15
		movaps xmm14, _fp255x4
		movaps xmm13, _gamma
		movups xmm12, [rax]
		; корректировка данных
		mov rax, whMtx
		or rax, 1
		cmp rax, 11
		jb @f
		mov whMtx, 11
@@:		movss xmm0, alpha
		mulps xmm0, _fp256x4
		movq mm1, qword ptr _alpha2
		cvtps2pi mm0, xmm0
		pshufw mm0, mm0, 0
		movq qword ptr _alpha0, mm0
		psubusw mm1, mm0
		movq qword ptr _alpha1, mm1
		; обрезка
		; реальные габариты
		push rcx
		push rdx
		push r8
		mov rcx, srcBar
		mov rdx, srcClip
		mov r8, r9
		call asmClipBar
		pushfq
		mov r14, rcx
		mov r15, rdx
		mov r10, rbx
		dec rcx
		dec rdx
		mov @@width, rcx
		mov @@height, rdx
		mov r9, r8
		popfq
		pop r8
		pop rdx
		pop rcx
		jnc _fin
		cvtsi2ss xmm10, dword ptr [rcx + 16]
		cvtsi2ss xmm11, dword ptr [rcx + 24]
		call asmClipBar
		jnc _fin
		xchg r10, rbx
		cvtsi2ss xmm0, rcx
		cvtsi2ss xmm1, rdx
		mov rax, adr
		mov r11, offset f_i_tex
		mov r12, offset f_i_flt
		mov r13, offset f_ops
		lea r11, [rax * 8 + r11]
		call qword ptr [r11]								; init_tex
		divss xmm2, xmm10									; приращение
		divss xmm3, xmm11
		; перемотать то что вылезло за пределы слева и сверху
		movss xmm11, xmm3
		pslldq xmm11, 4
		movss xmm11, xmm2
		pslldq xmm11, 4
		mov rax, _clip + 08
		neg rax
		cvtsi2ss xmm0, rax
		mulss xmm0, xmm3
		movss xmm11, xmm0
		pslldq xmm11, 4
		mov rax, _clip + 00
		neg rax
		cvtsi2ss xmm7, rax
		mulss xmm7, xmm2
		mov rax, filter
		lea r12, [rax * 8 + r12]
		call qword ptr [r12]								; init_flt
		mov r11, [r11 + 8 * 8]								; func_tex
		mov r12, [r12 + 8 * 16]								; func_flt
		mov rax, ops
		mov r13, [rax * 8 + r13]							; func_ops
		pxor mm3, mm3
_height:push rcx
		push rdx
		push r8
		movss xmm11, xmm7
@@:		push rcx
		call r12											; func_filter
		movq mm4, mm0
		movd mm1, dword ptr [r8]
		movq mm6, _mm1000
		call r13											; func_ops
		movd mm2, msk
		pand mm1, mm2
		pandn mm2, mm0
		por mm1, mm2
		movd dword ptr [r8], mm1
		movhlps xmm0, xmm11
		addss xmm11, xmm0
		pop rcx
		add r8, 4
		loop @b
		pop r8
		pop rdx
		pop rcx
		movlhps xmm0, xmm11
		addps xmm0, xmm11
		movhlps xmm11, xmm0
		add r8, r10
		dec rdx
		jnz _height
		emms
_fin:	mov rcx, @@tmpBuf
		sub rsp, 32
		call free
		add rsp, 32
		ret
OPTION EPILOGUE:NONE




iNr:	movss xmm2, xmm0
		movss xmm3, xmm1
iNn:	ret
iLM:	cvtsi2ss xmm2, nWrap
		divss xmm0, xmm2
		divss xmm1, xmm2
iLC:	cvtsi2ss xmm2, r14
		cvtsi2ss xmm3, r15
		ret

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
nMirror:cvtps2pi mm7, xmm11
		pextrw rax, mm7, 0
		xor rdx, rdx
		mov rcx, r14
		shl rcx, 1
		div rcx
		cmp rdx, r14
		jb @f
		inc rdx
		sub rdx, rcx
		neg rdx
@@:		push rdx
		pextrw rax, mm7, 2
		xor rdx, rdx
		mov rcx, r15
		shl rcx, 1
		div rcx
		cmp rdx, r15
		jb @f
		inc rdx
		sub rdx, rcx
		neg rdx
@@:		pop rcx
		imul rdx, rbx
		add rdx, r9
		movd mm0, dword ptr [rdx + rcx * 4]
		jmp unpack
nRepeat:cvttps2pi mm7, xmm11
		pextrw rax, mm7, 0
		xor rdx, rdx
		div r14
		mov rcx, rdx
		pextrw rax, mm7, 2
		xor rdx, rdx
		div r15
		imul rdx, rbx
		add rdx, r9
		movd mm0, dword ptr [rdx + rcx * 4]
		jmp unpack
nClamp:	cvttps2pi mm7, xmm11
		pextrw rax, mm7, 0
		pextrw rcx, mm7, 2
		xor rdx, rdx
		cmp rax, rdx
		cmovl rax, rdx
		cmp rcx, rdx
		cmovl rcx, rdx
		cmp rax, @@width
		cmovg rax, @@width
		cmp rcx, @@height
		cmovg rcx, @@height
		imul rcx, rbx
		add rcx, r9
		movd mm0, dword ptr [rcx + rax * 4]
		jmp unpack
copyMirror:
		push rdx
		xor rdx, rdx
		mov rsi, rbx
		shl rsi, 1
		div rsi
		cmp rdx, rbx
		jb @f
		inc rdx
		sub rdx, rsi
		neg rdx
@@:		mov rsi, rdx
		pop rdx
		ret
lMirror:cvttps2pi mm7, xmm11
		pextrw rax, mm7, 0
		push rbx
		push rax
		mov rbx, r14
		call copyMirror
		mov rcx, rsi
		pop rax
		inc rax
		call copyMirror
		mov rdi, rsi
		pextrw rax, mm7, 2
		push rax
		mov rbx, r15
		call copyMirror
		mov rdx, rsi
		pop rax
		inc rax
		call copyMirror
		mov rax, rdi
		mov rdi, rsi
		pop rbx
		jmp linear
lRepeat:cvttps2pi mm7, xmm11
		pextrw rax, mm7, 0
		push rax
		xor rdx, rdx
		div r14
		mov rcx, rdx
		pextrw rax, mm7, 2
		push rax
		xor rdx, rdx
		div r15
		mov rdi, rdx
		pop rax
		inc rax
		xor rdx, rdx
		div r15
		mov rsi, rdx
		pop rax
		inc rax
		xor rdx, rdx
		div r14
		mov rax, rdx
		mov rdx, rdi
		mov rdi, rsi
		jmp linear
lClamp:	cvttps2pi mm7, xmm11
		pextrw rcx, mm7, 0
		pextrw rdx, mm7, 2
		xor rsi, rsi
		lea rax, [rcx + 1]
		lea rdi, [rdx + 1]
		cmp rcx, rsi
		cmovl rcx, rsi
		cmp rdx, rsi
		cmovl rdx, rsi
		cmp rcx, @@width
		cmovg rcx, @@width
		cmp rdx, @@height
		cmovg rdx, @@height
		cmp rax, rsi
		cmovl rax, rsi
		cmp rdi, rsi
		cmovl rdi, rsi
		cmp rax, @@width
		cmovg rax, @@width
		cmp rdi, @@height
		cmovg rdi, @@height
linear:	movaps xmm0, xmm11
		cvtpi2ps xmm1, mm7
		subps xmm0, xmm1
		mulps xmm0, xmm14
		cvtps2pi mm7, xmm0
		pslld mm7, 8
		imul rdx, rbx
		imul rdi, rbx
		add rdx, r9
		add rdi, r9
		movd mm1, dword ptr [rdi + rcx * 4]
		movd mm0, dword ptr [rdi + rax * 4]
		punpcklbw mm0, mm3
		punpcklbw mm1, mm3
		psubsw mm0, mm1
		movd mm5, dword ptr [rdx + rcx * 4]
		movd mm4, dword ptr [rdx + rax * 4]
		punpcklbw mm4, mm3
		punpcklbw mm5, mm3
		psubsw mm4, mm5
		pshufw mm6, mm7, 10101010b	; yy
		pshufw mm7, mm7, 0			; xx
		pmulhw mm0, mm7
		pmulhw mm4, mm7
		paddsw mm0, mm1
		paddsw mm4, mm5
		psubsw mm0, mm4
		pmulhw mm0, mm6
		paddsw mm0, mm4
		packuswb mm0, mm0
unpack:	movq2dq xmm0, mm0
		punpcklbw xmm0, xmm15
		punpcklwd xmm0, xmm15
		cvtdq2ps xmm0, xmm0
		divps xmm0, xmm14
		ret
pixNone:ret
pixLumAdd:	pand mm0, mm6
			movq2dq xmm6, mm4
			punpcklbw xmm6, xmm15
			punpcklwd xmm6, xmm15
			cvtdq2ps xmm6, xmm6
			dpps xmm6, xmm13, 01110001b
			cvtps2pi mm4, xmm6
			pshufw mm4, mm4, 11000000b
			packuswb mm4, mm4
			por mm0, mm4
			movq mm4, mm0
pixAdd:		pand mm0, mm6
			paddusb mm4, mm1
			pandn mm6, mm4
			por mm0, mm6
			ret
pixLumSub:	pand mm0, mm6
			movq2dq xmm6, mm4
			punpcklbw xmm6, xmm15
			punpcklwd xmm6, xmm15
			cvtdq2ps xmm6, xmm6
			dpps xmm6, xmm13, 01110001b
			cvtps2pi mm4, xmm6
			pshufw mm4, mm4, 11000000b
			packuswb mm4, mm4
			por mm0, mm4
			movq mm4, mm0
pixSub:		pand mm0, mm6
			psubusb mm4, mm1
			pandn mm6, mm4
			por mm0, mm6
			ret
pixLum:		pand mm0, mm6
			movq2dq xmm6, mm4
			punpcklbw xmm6, xmm15
			punpcklwd xmm6, xmm15
			cvtdq2ps xmm6, xmm6
			dpps xmm6, xmm13, 01110001b
			cvtps2pi mm6, xmm6
			pshufw mm6, mm6, 11000000b
			packuswb mm6, mm6
			por mm0, mm6
			ret
pixMull:	pand mm0, mm6				; сохраняем альфу
			punpcklbw mm4, mm3
			punpcklbw mm1, mm3
			pmullw mm4, mm1
			packuswb mm4, mm4
			packuswb mm1, mm1
			pandn mm6, mm4
			por mm0, mm6
pixSet:		ret
pixAnd:		pand mm0, mm6
			pand mm4, mm1
			pandn mm6, mm4
			por mm0, mm6
			ret
pixOr:		pand mm0, mm6
			por mm4, mm1
			pandn mm6, mm4
			por mm0, mm6
			ret
pixNot:		pxor mm0, qword ptr _not
			ret
pixXor:		pand mm0, mm6
			pxor mm4, mm1
			pandn mm6, mm4
			por mm0, mm6
			ret
pixAlpha:	pand mm0, mm6
			movq mm5, mm1
			punpcklbw mm4, mm3
			punpcklbw mm5, mm3
			pshufw mm2, mm4, 11111111b
			pmullw mm4, mm2
			movq mm7, qword ptr _alpha
			psubusw mm7, mm2
			pmullw mm5, mm7
			paddusw mm4, mm5
			psraw mm4, 8
			packsswb mm4, mm4
			pandn mm6, mm4
			por mm0, mm6
			ret
;((src * alpha) + (dst * (256 - alpha))) / 256;
pixFixed:	pand mm0, mm6
			movq mm5, mm1
			punpcklbw mm4, mm3
			punpcklbw mm5, mm3
			pmullw mm4, qword ptr _alpha0
			pmullw mm5, qword ptr _alpha1
			paddusw mm4, mm5
			psraw mm4, 8
			packsswb mm4, mm4
			pandn mm6, mm4
			por mm0, mm6
			ret
pixNorm:	pand mm0, mm6
			pandn mm6, mm4
			movq2dq xmm1, mm6
			punpcklbw xmm1, xmm15
			punpcklwd xmm1, xmm15
			pshufd xmm1, xmm1, 11011110b
			cvtdq2ps xmm1, xmm1
			movaps xmm0, xmm1
			dpps xmm1, xmm1, 01110111b
			rsqrtps xmm1, xmm1
			mulps xmm0, xmm1
			mulps xmm0, _fp0_5x3
			addps xmm0, _fp0_5x3
			mulps xmm0, xmm14
			cvtps2dq xmm0, xmm0
			packssdw xmm0, xmm0
			packuswb xmm0, xmm0
			movdq2q mm4, xmm0
			por mm0, mm4
			ret
asmCopy endp

end
