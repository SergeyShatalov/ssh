
include asm_ssh.inc

public _alpha_not, _mm_gamma

.const
align 16
_gamma		dd 0.3, 0.59, 0.11, 1.0, 0.3, 0.59, 0.11, 1.0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
f_256x8		dd 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0
f_inv255x8	dd 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862
f_inv256x8	dd 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625
f_0_5x8		dd 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
f_3_0x8		dd 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0
f_1_0x8		dd 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
f_2_0x8		dd 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0
f_100x4		dd 100.0, 100.0, 100.0, 100.0
flt_max		dd 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F
_fabs		dd 7fffffffh, 7fffffffh, 7fffffffh, 7fffffffh
_mm_not		dd -1, -1, -1, -1
_alpha_not	dw 255, 255, 255, 255
_mm_gamma	dw 77, 151, 28, 1, 77, 151, 28, 1

.data?
_clip		stk_bar<?>
histogramm	dd 256 dup(?)

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

asm_ssh_v_flip proc public USES r11 r12 bar:QWORD, clip:QWORD, dst:QWORD
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

asm_ssh_h_flip proc public USES r11 bar:QWORD, clip:QWORD, dst:QWORD
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

asm_ssh_flip_90 proc USES rbx wh:QWORD, dst:QWORD, src:QWORD
		mov r9, rdx
		movsxd rdx, [rcx].stk_range.h		; w->h
		movsxd rcx, [rcx].stk_range.w		; h->w
		jrcxz _fin
		lea rbx, [rdx * 4]					; pitch
		lea r9, [r9 + rbx - 4]
_loop:	mov r10, r9
		mov r11, rcx
@@:		mov eax, [r8]
		mov [r10], eax
		add r10, rbx
		add r8, 4
		loop @b
		mov rcx, r11
		sub r9, 4
		dec rdx
		jg _loop
_fin:	ret
asm_ssh_flip_90 endp

asm_ssh_figure proc bar:QWORD, clip:QWORD, pix:QWORD, modify:QWORD
		ret
asm_ssh_figure endp

asm_ssh_gradient proc USES r11 r12 r13 r14 r15 rsi rbx bar:QWORD, clip:QWORD, pix:QWORD, modify:QWORD
		vzeroupper
		movsxd r14, [rcx].stk_bar.w
		movsxd r15, [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		pxor mm7, mm7
		movd mm0, [r9].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, qword ptr _mm_not
		mov r11, offset _func_ops
		mov r12, offset _func_tex
		movsxd rax, [r9].stk_modify.src_ops
		mov r11, [r11 + rax * 8]						; функция пиксельной операции
		movsxd rax, [r9].stk_modify.type_address
		mov r12, [r12 + rax * 8]						; функция адресации координат
		; преобразовать начальный и конечный цвет в плавающий формат
		vmovaps xmm12, f_255x8
		vmovaps xmm13, f_1_0x8
		vpmovzxbd xmm14, dword ptr [r9].stk_modify.src_val
		vpmovzxbd xmm15, dword ptr [r9].stk_modify.dst_val
		vcvtdq2ps xmm14, xmm14
		vcvtdq2ps xmm15, xmm15
		vdivps xmm14, xmm14, xmm12						; color1
		vdivps xmm15, xmm15, xmm12						; color2
		; произвести предрасчет градиента
		vcvtsi2ss xmm0, xmm0, r14						; plane_x
		vcvtsi2ss xmm1, xmm1, r15						; plane_y
		vdivss xmm6, xmm0, [r9].stk_modify.wh_repeat.w
		vdivss xmm7, xmm1, [r9].stk_modify.wh_repeat.h
		vdivss xmm8, xmm0, xmm6							; dx
		vdivss xmm9, xmm1, xmm7							; dy
		vmulss xmm2, xmm0, xmm0							; xx = plane_x * plane_x
		vfmadd231ss xmm2, xmm1, xmm1					; xx += plane_y * plane_y
		vrsqrtss xmm4, xmm2, xmm2						; length = 1.0f / sqrtf(xx)
		vmulss xmm2, xmm4, xmm4							; length *= length
		vmulss xmm0, xmm0, xmm2
		vmulss xmm1, xmm1, xmm2
		; сдвинуть координаты на clip.x, clip.y
		vshufps xmm7, xmm9, xmm8, 00000000b				; dx|dx|dy|dy
		vcvtsi2ss xmm6, xmm6, _clip.y
		vmulss xmm6, xmm6, xmm9							; y
		vshufps xmm7, xmm6, xmm7, 00100000b				; dy|dx|y|y
		vcvtsi2ss xmm6, xmm6, _clip.x
		vmulss xmm6, xmm6, xmm8							; x
_h:		push rdx
		push rcx
		vmovss xmm7, xmm7, xmm6							; dy|dx|y|x
		vcvtps2dq xmm5, xmm7
		vpextrd r13, xmm5, 1
		mov r9, r8
@@:		vcvtss2si rbx, xmm7
		call r12										; вернуть координаты, в соответствии с адресацией координат
		vcvtsi2ss xmm2, xmm2, rbx
		vcvtsi2ss xmm3, xmm3, rdx
		vmulss xmm2, xmm2, xmm0							; x *= plane_x
		vfmadd231ss xmm2, xmm3, xmm1					; x += y * plane_y
		vsubss xmm2, xmm2, xmm4							; x -= plane_z
		vshufps xmm2, xmm2, xmm2, 0
		vsubps xmm3, xmm13, xmm2						; tmp2 = 1 - color1
		vmulps xmm2, xmm15, xmm2						; tmp1 = color2 * color1
		vfmadd231ps xmm2, xmm3, xmm14					; tmp1 += tmp2 * color1
		vmulps xmm2, xmm2, xmm12
		vcvtps2dq xmm2, xmm2							; пакуем цвет
		vpackssdw xmm2, xmm2, xmm2
		movdq2q mm2, xmm2
		packuswb mm2, mm2
		movd mm3, dword ptr [r9]
		call r11										; пиксельная операция
		movd dword ptr [r9], mm2
		add r9, 4
		vmovhlps xmm2, xmm2, xmm7						; 0|0|dy|dx
		vaddss xmm7, xmm7, xmm2							; dy|dx|y|x+dx
		loop @b
		vmovlhps xmm2, xmm2, xmm7						; y|x|0|0
		vaddps xmm2, xmm2, xmm7							; y+dy|x+dx|y|x
		vmovhlps xmm7, xmm7, xmm2						; dy|dx|y+dy|x+dx
		pop rcx
		pop rdx
		add r8, r10
		dec rdx
		jnz _h
_fin:	emms
		ret
_func_tex	dq _clamp,_mirror,_repeat,_clamp, _mirror, _repeat
epilog_none
_clamp:	mov rdx, r13
		ret
_repeat:xor rdx, rdx
		mov rax, rbx
		div r14
		mov rbx, rdx
		xor rdx, rdx
		mov rax, r13
		div r15
		ret
_mirror:mov rax, rbx
		xor rdx, rdx
		lea rsi, [r14 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, r14
		cmovge rdx, rsi
		mov rbx, rdx
		mov rax, r13
		xor rdx, rdx
		lea rsi, [r15 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, r15
		cmovge rdx, rsi
		ret
epilog_def
asm_ssh_gradient endp

asm_ssh_replace proc vals:QWORD, msks:QWORD, pix:QWORD, clip:QWORD
		mov r10, r8
		mov r8d, [rcx].stk_range.w				; src_val
		movd mm0, dword ptr [rcx].stk_range.h	; dst_val
		movsxd rax, [r9].stk_range.w
		movsxd rcx, [r9].stk_range.h
		imul rcx, rax							; длина изображения
		jrcxz _fin
		mov r9d, [rdx].stk_range.w				; src_msk
		movd mm1, dword ptr [rdx].stk_range.h	; dst_msk
		movq mm2, mm1
		pandn mm2, mm0							; dst_val
_loop:	movd mm0, dword ptr [r10]
		movd edx, mm0
		and edx, r9d
		cmp edx, r8d
		jnz @f
		pand mm0, mm1
		por mm0, mm2
		movd dword ptr [r10], mm0
@@:		add r10, 4
		loop _loop
		emms
_fin:	ret
asm_ssh_replace endp

histogramm_rgb			= 0
histogramm_red			= 1
histogramm_green		= 2
histogramm_blue			= 3
histogramm_alpha		= 4
histogramm_rgb_v		= 5
histogramm_red_v		= 6
histogramm_green_V		= 7
histogramm_blue_v		= 8
histogramm_alpha_v		= 9

asm_ssh_histogramm proc USES rsi rdi rbx wh:QWORD, modify:QWORD, buf:QWORD
		vmovaps xmm1, _gamma
		mov r11, offset _addr
		mov eax, [rdx].stk_modify.type_histogramm
		mov r11, [r11 + rax * 8]					; процедура извлечения канала
		mov rdi, offset histogramm					; буфер гистограммы
		mov rsi, rcx
		mov r9, [rdx].stk_modify.rgba.buf
		movsxd rcx, [rdx].stk_modify.wh_rgba.w
		imul ecx, [rdx].stk_modify.wh_rgba.h
		mov r10, rdi
		mov rbx, rcx
		mov rcx, 128
		xor rax, rax								; заполняем нулями буфер
		rep stosq
		mov rcx, rbx
@@:		call r11
		inc dword ptr [rax * 4 + r10]
		add r9, 4
		loop @b
		mov rax, 1									; максимальное значение
		mov rcx, rax
@@:		cmp eax, dword ptr [r10 + rcx * 4]
		cmovl eax, dword ptr [r10 + rcx * 4]
		inc rcx
		cmp rcx, 254
		jb @b
		cmp [rdx].stk_modify.type_histogramm, histogramm_rgb_v
		jb img
		; найти мин и макс диапазона
		mov rcx, 1
		mov rdx, 254
@@:		cmp dword ptr [r10 + rcx * 4], 15
		jae @f
		inc rcx
		cmp rcx, rdx
		jb @b
@@:		cmp dword ptr [r10 + rdx * 4], 15
		jae @f
		dec rdx
		jnz @b
@@:		mov [r8], ecx
		mov [r8 + 4], rdx
		lea rdi, [r8 + 8]
		mov rsi, r10
		mov rcx, 128
		rep movsq
		ret
img:	movsxd rbx, [rsi].stk_range.w				; w
		movsxd rsi, [rsi].stk_range.h				; h
		mov rcx, rbx
		imul rcx, rsi
		vcvtsi2ss xmm0, xmm0, rbx
		vdivss xmm0, xmm0, f_256x8					; dx
		vcvtsi2ss xmm1, xmm1, rsi
		vcvtsi2ss xmm2, xmm2, rax					; max_h
		vdivss xmm1, xmm1, xmm2						; dh
		shl rbx, 2									; pitch
		; заполнить картинку цветом гистограммы
		mov eax, [rdx]. stk_modify.cols_histogramm.h; f_rgba
		mov rdi, r8
		rep stosd
		mov eax, [rdx]. stk_modify.cols_histogramm.w; b_rgba
		; находим x1, x2
		xor rdx, rdx
_loop:	mov rdi, rdx
@@:		cmp rdi, 255
		jae @f
		inc rdi
		movsxd rcx, dword ptr [r10 + rdi * 4]
		jrcxz @b
@@:		cmp rdx, rdi
		jnz @f
		cmp rdx, 255
		jz _fin
		mov rdi, 255
@@:		vcvtsi2ss xmm2, xmm2, dword ptr [r10 + rdx * 4]	; v1
		vcvtsi2ss xmm3, xmm3, dword ptr [r10 + rdi * 4]	; v2
		vmulss xmm2, xmm2, xmm1						; h1
		vmulss xmm3, xmm3, xmm1						; h2
		vcvtsi2ss xmm4, xmm4, rdx
		vcvtsi2ss xmm5, xmm5, rdi
		vmulss xmm4, xmm4, xmm0						; x1
		vmulss xmm5, xmm5, xmm0						; x2
		vsubss xmm6, xmm5, xmm4						; w_length
		vsubss xmm7, xmm3, xmm2						; h_length
		vdivss xmm7, xmm7, xmm6						; dy = h_length / w_length
		vcvtss2si r9, xmm4
		vcvtss2si rcx, xmm6
		cmp rcx, 1
		adc rcx, 0
		mov rdx, rdi
		lea r9, [r8 + r9 * 4]
_loop1:	vcvtss2si r11, xmm2							; h
		sub r11, rsi
		neg r11
		jle _no
		mov rdi, r9
@@:		mov [rdi], eax
		add rdi, rbx
		dec r11
		jnz @b
_no:	vaddss xmm2, xmm2, xmm7
		add r9, 4
		loop _loop1
		jmp _loop
_fin:	ret
_addr	dq _rgb, _red, _green, _blue, _alpha, _rgb, _red, _green, _blue, _alpha
epilog_none
_red:	movzx rax, byte ptr [r9 + 0]
		ret
_green:	movzx rax, byte ptr [r9 + 1]
		ret
_blue:	movzx rax, byte ptr [r9 + 2]
		ret
_alpha:	movzx rax, byte ptr [r9 + 3]
		ret
_rgb:	vpmovzxbd xmm0, dword ptr [r9]
		vcvtdq2ps xmm0, xmm0
		vdivps xmm0, xmm0, f_255x8
		vdpps xmm0, xmm0, xmm1, 01110001b
		vmulps xmm0, xmm0, f_255x8
		vcvtss2si rax, xmm0
		ret
epilog_def
asm_ssh_histogramm endp

asm_ssh_correct proc USES rdi clip:QWORD, wh:QWORD, type:DWORD, pix:QWORD
		movsxd r11, [rcx].stk_range.w
		imul r11d, [rcx].stk_range.h			; длина изображения
		; фиксируем диапазон
		movsxd rcx, [rdx].stk_range.w			; start
		movsxd rdx, [rdx].stk_range.h			; end
		xor rax, rax
		mov r10, 255
		cmp rcx, rax
		cmovl rcx, rax
		cmp rdx, rax
		cmovl rdx, rax
		cmp rcx, r10
		cmovg rcx, r10
		cmp rdx, r10
		cmovg rdx, r10							; clamp(start, 0, 255), clamp(end, 0, 255)
		mov rax, rcx
		cmp rcx, rdx
		cmovg rcx, rdx
		cmp rdx, rax
		cmovl rdx, rax							; start > end -> swap(start, end)
		mov r10, offset _addr1
		mov r10, [r10 + r8 * 8]
		mov r8, offset histogramm
		mov rdi, r8
		xor rax, rax
		sub rdx, rcx
		rep stosb								; стереть до начальной позиции диапазона
		mov rcx, rdx
		jrcxz _fin
		vcvtsi2ss xmm1, xmm1, rcx
		vmovaps xmm0, f_256x8
		vdivss xmm1, xmm0, xmm1					; dx
		vxorps xmm0, xmm0, xmm0					; x
@@:		vcvtss2si rax, xmm0
		vaddss xmm0, xmm0, xmm1
		stosb
		loop @b
		mov rcx, rdi
		sub rcx, r8
		sub rcx, 256
		neg rcx
		mov rax, 255
		rep stosb								; заполнить конец диапазона значениями 255
		mov rcx, r11							; корректировать изображение
@@:		call r10
		add r9, 4
		loop @b
_fin:	ret
_addr1	dq _rgb, _red, _green, _blue, _alpha, _rgb, _red, _green, _blue, _alpha
epilog_none
_rgb:	mov al, [r9 + 0]
		mov al, [rax + r8]
		mov [r9 + 0], al
		mov al, [r9 + 1]
		mov al, [rax + r8]
		mov [r9 + 1], al
		mov al, [r9 + 2]
		mov al, [rax + r8]
		mov [r9 + 2], al
		ret
_red:	mov al, [r9 + 0]
		mov al, [rax + r8]
		mov [r9 + 0], al
		ret
_green:	mov al, [r9 + 1]
		mov al, [rax + r8]
		mov [r9 + 1], al
		ret
_blue:	mov al, [r9 + 2]
		mov al, [rax + r8]
		mov [r9 + 2], al
		ret
_alpha:	mov al, [r9 + 3]
		mov al, [rax + r8]
		mov [r9 + 3], al
		ret
epilog_def
asm_ssh_correct endp

; (const Range<int>& clip, int vals, void* pix, float scale);
asm_ssh_noise_perlin proc
comment $
function Noise1(integer x, integer y)
    n = x + y * 57
    n = (n<<13) ^ n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 7fffffff) / 1073741824.0);    
  end function

  function SmoothNoise_1(float x, float y)
    corners = ( Noise(x-1, y-1)+Noise(x+1, y-1)+Noise(x-1, y+1)+Noise(x+1, y+1) ) / 16
    sides   = ( Noise(x-1, y)  +Noise(x+1, y)  +Noise(x, y-1)  +Noise(x, y+1) ) /  8
    center  =  Noise(x, y) / 4
    return corners + sides + center
  end function

 function Linear_Interpolate(a, b, x)
	return  a*(1-x) + b*x
  end of function
  
  function Cosine_Interpolate(a, b, x)
	ft = x * 3.1415927
	f = (1 - cos(ft)) * .5
	return  a*(1-f) + b*f
  end of function
  
  function Cubic_Interpolate(v0, v1, v2, v3,x)
	P = (v3 - v2) - (v0 - v1)
	Q = (v0 - v1) - P
	R = v2 - v0
	S = v1

	return Px3 + Qx2 + Rx + S
  end of function
  
  function InterpolatedNoise_1(float x, float y)
      integer_X    = int(x)
      fractional_X = x - integer_X
      integer_Y    = int(y)
      fractional_Y = y - integer_Y
      v1 = SmoothedNoise1(integer_X,     integer_Y)
      v2 = SmoothedNoise1(integer_X + 1, integer_Y)
      v3 = SmoothedNoise1(integer_X,     integer_Y + 1)
      v4 = SmoothedNoise1(integer_X + 1, integer_Y + 1)
      i1 = Interpolate(v1 , v2 , fractional_X)
      i2 = Interpolate(v3 , v4 , fractional_X)
      return Interpolate(i1 , i2 , fractional_Y)
  end function

frequencyx  = flt_vec.x
frequencyy  = flt_vec.y
amplitude  = scale

  function PerlinNoise_2D(float x, float y)
      total = 0
      p = persistence
      n = Number_Of_Octaves - 1
      loop i from 0 to n
          frequency = 2i
          amplitude = pi
          total = total + InterpolatedNoisei(x * frequencyx, y * frequencyy) * amplitude
      end of i loop
      return total
  end function$
		ret
asm_ssh_noise_perlin endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_noise_terrain proc
		ret
asm_ssh_noise_terrain endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_border3d proc
		ret
asm_ssh_border3d endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_group proc
		ret
asm_ssh_group endp

; (const Bar<int>& bar, const Range<int>& clip, void* pix, ImgMod* modify);
asm_ssh_table proc
		ret
asm_ssh_table endp

asm_ssh_border2d proc bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
		
		ret
comment $
; rcx(bar) rdx(clip), r8(src), r9(widthBorder), val, msk, side, pixOps
asmBorder proc USES rbx r12 r13 r14 bar:QWORD, clip:QWORD, src:QWORD, width:QWORD, val:QWORD, msk:QWORD, side:QWORD, pixOps:BYTE
		movzx rax, pixOps
		mov r12, offset pix_ops
		mov r12, [rax * 8 + r12]
		mov r14, r12
		mov r13, side
		movd mm0, val
		movd mm1, msk
		movq mm2, mm1
		pandn mm2, qword ptr _mmNot
		call drawBorder
		ret
asmBorder endp
$
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
