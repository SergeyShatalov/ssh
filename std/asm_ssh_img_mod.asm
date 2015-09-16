
include asm_ssh.inc

.code

; преобразование из процентного задания координат в абсолютные
;static Bar<int> absolute_bar(const Bar<int>& bar, const Range<int>& clip, Coord coord)
;{
;	if(coord != Coord::percent) return bar;
;	float x(bar.x / 100.0f), y(bar.y / 100.0f), w(bar.w / 100.0f), h(bar.h / 100.0f);
;	return Bar<int>((int)(x * clip.w), (int)(y * clip.h), (int)(w * clip.w), (int)(h * clip.h));
;}

asm_ssh_absolute_bar proc
		ret
asm_ssh_absolute_bar endp

;rcx(bar), rdx(clip) r8(dst) r11(offset stk_clip)-> out rcx(width) rdx(height) r10(pitch) r8(dst)
asm_clip_bar proc USES rax rdi rsi rbx
		mov qword ptr [_clip + 0], 0		; стираем область выхода за пределы клипа
		mov qword ptr [_clip + 8], 0
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
		mov _clip.x, rax					; -xb -> clip.x
		xor rax, rax						; xb = 0
@@:		lea r10, [rax + rcx]				; ww = xb + xw
		sub r10, rsi						; ww -= wc <= 0 -> skip
		jle @f
		sub rcx, r10						; wb -= ww <= 0 -> error
		jle _err
		mov _clip.w, r10					; ww -> clip.w
@@:		test rbx, rbx						; yb >= 0 -> skip
		jge @f
		add rdx, rbx						; hb += yb <= 0 -> error
		jle _err
		mov _clip.y, rbx					; -yb -> clip.y
		xor rbx, rbx						; yb = 0
@@:		lea r10, [rbx + rdx]				; hh = yb + hb
		sub r10, rdi						; hh -= wh <= 0 -> skip
		jle @f
		sub rdx, r10						; wh -= hh <= 0 -> error
		jle _err
		mov _clip.h, r10					; hh -> clip.h
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
asm_ssh_v_flip proc public USES r11
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
		jg @b
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
		ret
asm_ssh_gradient endp

 ;(const Range<int>& vals, const Range<int>& msks, void* pix, const Range<int>& clip);
asm_ssh_replace proc
		ret
asm_ssh_replace endp

; (const Range<int>& tmp, ImgMod* modify, void* buf);
asm_ssh_histogramm proc
		ret
asm_ssh_histogramm endp

; (const Range<int>& clip, const Range<int>& rn, void* pix, ImgMod::Histogramms type);
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

end
