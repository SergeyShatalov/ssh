
include asm_ssh.inc

.const
align 16
_gamma		dd 0.3, 0.59, 0.11, 0.0, 0.3, 0.59, 0.11, 0.0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
f_256x8		dd 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0
f_0_5x8		dd 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
f_3_0x8		dd 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0
f_1_0x8		dd 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
f_2_0x8		dd 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0
f_100x4		dd 100.0, 100.0, 100.0, 100.0
flt_max		dd 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F
_fabs		dd 7fffffffh, 7fffffffh, 7fffffffh, 7fffffffh
_mm_not		dq -1

.data?
_clip		stk_bar<>

.code

; rcx(bar), rdx(clip)
asm_absolute_bar proc
local tmp:stk_bar
		lea rax, tmp
		vcvtdq2ps xmm0, xmmword ptr [rcx]
		vmovlps xmm1, xmm1, qword ptr [rdx]
		vmovlhps xmm1, xmm1, xmm1
		vcvtdq2ps xmm1, xmm1
		vdivps xmm0, xmm0, f_100x4
		vmulps xmm0, xmm0, xmm1
		vcvtps2dq xmm0, xmm0
		vmovups [rax], xmm0
		ret
asm_absolute_bar endp

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
asm_clip_bar proc USES rax rdi rsi rbx
		mov qword ptr [_clip + 00], 0		; стираем область выхода за пределы клипа
		mov qword ptr [_clip + 08], 0
		movsxd rsi, dword ptr [rdx + 00]	; wc
		movsxd rdi, dword ptr [rdx + 04]	; hc
		movsxd rax, dword ptr [rcx + 00]	; xb
		movsxd rbx, dword ptr [rcx + 04]	; yb
		movsxd rdx, dword ptr [rcx + 12]	; hb
		movsxd rcx, dword ptr [rcx + 08]	; wb
		test rdx, rdx						; для неопределенной области
		cmovle rdx, rdi
		test rcx, rcx
		cmovle rcx, rsi
		test rax, rax						; xb >= 0 -> skip
		jge @f
		add rcx, rax						; wb += xb <= 0 -> error
		jle _err
		neg rax
		mov _clip.x, eax					; -xb -> clip.x
		xor rax, rax						; xb = 0
@@:		lea r10, [rax + rcx]				; ww = xb + xw
		sub r10, rsi						; ww -= wc <= 0 -> skip
		jle @f
		sub rcx, r10						; wb -= ww <= 0 -> error
		jle _err
		mov _clip.w, r10d					; ww -> clip.w
@@:		test rbx, rbx						; yb >= 0 -> skip
		jge @f
		add rdx, rbx						; hb += yb <= 0 -> error
		jle _err
		neg rbx
		mov _clip.y, ebx					; -yb -> clip.y
		xor rbx, rbx						; yb = 0
@@:		lea r10, [rbx + rdx]				; hh = yb + hb
		sub r10, rdi						; hh -= wh <= 0 -> skip
		jle @f
		sub rdx, r10						; wh -= hh <= 0 -> error
		jle _err
		mov _clip.h, r10d					; hh -> clip.h
@@:		shl rsi, 2							; wc *= 4
		push rsi
		imul rsi, rbx						; wc *= yb
		lea rsi, [rsi + rax * 4]			; wc += xb * 4
		add r8, rsi							; r8 += wc -> начальный адрес области
		pop r10								; r10 = pitch
		clc
		ret
_err:	stc
		ret
asm_clip_bar endp

;rcx(bar) rdx(clip) r8(dst)
asm_ssh_v_flip proc public USES r11 r12
		call asm_clip_bar
		jc _fin
		dec rdx
		jle _fin
		mov r11, r10
		imul r11, rdx
_loop:	mov r9, r8
		mov r12, rcx
@@:		mov rax, [r9]
		xchg rax, [r9 + r11]
		mov [r9], rax
		add r9, 8
		sub r12, 2
		ja @b
		jz @f
		mov eax, [r9]			; если ширина не четная
		xchg rax, [r9 + r11]
		mov [r9], eax
@@:		add r8, r10
		sub r11, r10
		sub r11, r10
		sub rdx, 2
		jg _loop
_fin:	ret
asm_ssh_v_flip endp

;rcx(bar) rdx(clip) r8(dst)
asm_ssh_h_flip proc public USES r11
		call asm_clip_bar
		jc _fin
		shr rcx, 1
		jle _fin
		movq mm2, qword ptr swap_h_flip
_loop:	mov r9, r8
		lea r11, [rcx * 8 - 8]
		push rcx
		shr rcx, 1
@@:		movq mm0, [r9]						; a2 b2 g2 r2 a1 b1 g1 r1 -> a1 b1 g1 r1 a2 b2 g2 r2
		movq mm1, [r9 + r11]
		pshufb mm0, mm2
		pshufb mm1, mm2
		movq [r9 + r11], mm0
		movq [r9], mm1
		sub r11, 16
		add r9, 8
		loop @b
		pop rcx
		add r8, r10
		dec rdx
		jnz _loop
		emms
_fin:	ret
swap_h_flip db 4, 5, 6, 7, 0, 1, 2, 3
asm_ssh_h_flip endp

; rcx(range), rdx(dst), r8(src)
asm_ssh_flip_90 proc
		mov r9, rdx
		mov rdx, [rcx + 08]		; w->h
		mov rcx, [rcx + 00]		; h->w
		jrcxz _fin
		lea rbx, [rdx * 4]		; pitch
		lea r9, [r9 + rdx * 4 - 4]
_loop:	mov r10, r9
		push rcx
@@:		mov eax, [r8]
		mov [r10], eax
_11:	add r10, rbx
		add r8, 4
		loop @b
		pop rcx
		sub r9, 4
		dec rdx
		jg _loop
_fin:	ret
asm_ssh_flip_90 endp

;(const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_figure proc
		ret
asm_ssh_figure endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_gradient proc
		vzeroupper
		movsxd r14, [rcx].stk_bar.w
		movsxd r15, [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		pxor mm7, mm7
		movd mm0, [r9].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, _mm_not
		mov r11, offset _func_ops
		mov r12, offset _func_tex
		movsxd rax, [r9].stk_modify.src_ops
		mov r11, [r11 + rax * 8]						; функция пиксельной операции
		movsxd rax, [r9].stk_modify.coord
		mov r12, [r12 + rax * 8]						; функция адресации координат
		; преобразовать начальный и конечный цвет в плавающий формат
		vxorps xmm10, xmm10, xmm10
		vmovaps xmm11, f_2_0x8
		vmovaps xmm12, f_255x8
		vmovaps xmm13, f_1_0x8
		vpmovzxbd xmm14, dword ptr [r9].stk_modify.src_val
		vpmovzxbd xmm15, dword ptr [r9].stk_modify.dst_val
		vcvtdq2ps xmm14, xmm14
		vcvtdq2ps xmm15, xmm15
		vdivps xmm14, xmm14, xmm12
		vdivps xmm15, xmm15, xmm12
		; произвести предрасчет градиента
		vcvtsi2ss xmm0, xmm0, r14						; plane_x
		vcvtsi2ss xmm1, xmm0, r15						; plane_y
		vdivss xmm0, xmm0, [r9].stk_modify.wh_repeat.w
		vdivss xmm1, xmm1, [r9].stk_modify.wh_repeat.h
		vmulss xmm2, xmm0, xmm0							; xx = plane_x * plane_x
		vfmadd231ss xmm2, xmm1, xmm1					; xx += plane_y * plane_y
		vrsqrtss xmm4, xmm2, xmm2						; length = 1.0f / sqrtf(xx)
		vmulss xmm2, xmm4, xmm4							; length *= length
		vmulss xmm0, xmm0, xmm2
		vmulss xmm1, xmm1, xmm2
		; сдвинуть координаты
		movsxd rdx, _clip.stk_bar.y
		neg rdx
_h:		mov r9, r8
		movsxd rcx, _clip.stk_bar.x
		neg rcx
@@:		vcvtsi2ss xmm2, xmm2, rcx
		vcvtsi2ss xmm3, xmm3, rdx
		vmulss xmm2, xmm2, xmm0							; x *= plane_x
		vfmadd231ss xmm2, xmm3, xmm1					; x += y * plane_y
		vsubss xmm2, xmm2, xmm4							; x -= plane_z
		call r12										; вернуть цвет в xmm5, в соответствии с адресацией координат
		vshufps xmm5, xmm5, xmm5, 0
		vmulps xmm2, xmm15, xmm5						; tmp1 = color2 * color
		vsubps xmm3, xmm13, xmm5						; tmp2 = 1 - color
		vfmadd231ps xmm2, xmm3, xmm14					; tmp1 += tmp2 * color1
		vcvtps2dq xmm2, xmm2							; пакуем цвет
		vpackssdw xmm2, xmm2, xmm2
		movdq2q mm2, xmm1
		packuswb mm2, mm2
		movd mm3, dword ptr [r9]
		call r11										; пиксельная операция
		movd dword ptr [r9], mm2
		add r9, 4
		inc rcx
		cmp rcx, r14
		jb @b
		add r8, r10
		inc rdx
		cmp rdx, r15
		jb _h
_fin:	emms
		ret
_func_tex	dq _clamp,_mirror,_repeat,_clamp, _mirror, _repeat, 0, 0
OPTION EPILOGUE:NONE
_clamp:	vmaxss xmm5, xmm2, xmm10
		vminss xmm5, xmm5, xmm13
		ret
_repeat:vroundss xmm3, xmm2, xmm2, 00001011b
		vsubss xmm5, xmm2, xmm3
		ret
_mirror:vandps xmm2, xmm2, _fabs		; v = |v|
		vdivss xmm3, xmm2, xmm11		; tmp = v / 2.0
		vroundss xmm3, xmm3, xmm3, 00001011b
		vmulps xmm3, xmm3, xmm11		; tmp *= 2.0
		vsubps xmm4, xmm2, xmm3			; flag = v - tmp (остаток от деления)
		vroundss xmm4, xmm4, xmm4, 00001011b
		vroundss xmm3, xmm2, xmm2, 00001011b
		vsubss xmm2, xmm2, xmm3			; v = modf(v, 1.0)
		vmulss xmm3, xmm4, xmm11		; f = flag * 2.0
		vsubss xmm3, xmm13, xmm3		; f = 1.0 - f
		vaddss xmm4, xmm4, xmm3			; flag += f
		vmulss xmm5, xmm2, xmm4			; res = v * flag
		ret
OPTION EPILOGUE:EPILOGUEDEF
asm_ssh_gradient endp

 ;(const Range<int>& vals, const Range<int>& msks, void* pix, const Range<int>& clip);
asm_ssh_replace proc USES rdi
		mov rdi, r8
		mov r10d, [rcx].stk_range.w		; src_val
		mov r11d, [rcx].stk_range.h		; dst_val
		movsxd rax, [r9].stk_range.w
		movsxd rcx, [r9].stk_range.h
		imul rcx, rax					; длина изображения
		jrcxz _fin
		mov r8d, [rdx].stk_range.w		; src_msk
		mov r9d, [rdx].stk_range.h		; dst_msk
_loop:	mov eax, [rdi]
		mov edx, eax
		and edx, r8d
		cmp edx, r10d
		jnz @f
		and eax, r9d
		or eax, r11d
@@:		stosd
		loop _loop
_fin:	ret
asm_ssh_replace endp

; (const Range<int>& tmp, ImgMod* modify, void* buf);
asm_ssh_histogramm proc
		ret
asm_ssh_histogramm endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod::Histogramms type);
asm_ssh_correct proc
		ret
asm_ssh_correct endp

; (const Range<int>& clip, int vals, void* pix, float scale);
asm_ssh_noise_perlin proc
		ret
asm_ssh_noise_perlin endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_noise_terrain proc
		ret
asm_ssh_noise_terrain endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_border_3d proc
		ret
asm_ssh_border_3d endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_group proc
		ret
asm_ssh_group endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_table proc
		ret
asm_ssh_table endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_border2d proc
		ret
asm_ssh_border2d endp

; (const Range<int>& _wh, ImgMod* modify, void* cells, void* ptr)
asm_ssh_mosaik proc
		ret
asm_ssh_mosaik endp

;rdx(pos) r8(clip)
asm_clip_quad proc private USES r10
		xorps xmm7, xmm7
		mov r10, [r8 + 00]		; xc
		mov r11, [r8 + 08]		; yc
		mov r12, [r8 + 16]		; wc
		mov r13, [r8 + 24]		; hc
		mov rax, [rdx + 00]		; xp
		mov rbx, [rdx + 16]		; wp
		mov rcx, [rdx + 24]		; hp
		mov rdx, [rdx + 08]		; yp
		mov r14, rax
		sub r14, r10
		jge @f
		add rbx, r14
		jle _err
		mov rax, r10
		pinsrd xmm7, r14d, 0
@@:		mov r14, rdx
		sub r14, r11
		jge @f
		add rcx, r14
		jle _err
		mov rdx, r11
		pinsrd xmm7, r14d, 1
@@:		lea r14, [rax + rbx]
		sub r14, r12
		jle @f
		sub rbx, r14
		jle _err
		pinsrd xmm7, r14d, 2
@@:		lea r14, [rdx + rcx]
		sub r14, r13
		jle @f
		sub rcx, r14
		jle _err
		pinsrd xmm7, r14d, 3
@@:		cvtdq2ps xmm7, xmm7
		stc
		ret
_err:	clc
		ret
asm_clip_quad endp

;rcx(bar) rdx(out_bar2) r8(clip)
asm_ssh_get_clipbar proc USES rbx
local tmp:stk_bar
		mov r9, rdx
		call asm_clip_quad
		lea r8, tmp
		test r9, r9
		cmovz r9, r8
		mov [r9].stk_bar.x, eax
		mov [r9].stk_bar.y, edx
		mov [r9].stk_bar.w, ebx
		mov [r9].stk_bar.h, ecx
		mov rax, r9
		ret
asm_ssh_get_clipbar endp
		
;QUAD* asm_ssh_make_quad(const Bar<float>& tex, const Bar<int>& pos, const Bar<int>& clip, const Range<int>& screen, int bgra);
asm_ssh_make_quad proc
		
		ret
asm_ssh_make_quad endp

end
