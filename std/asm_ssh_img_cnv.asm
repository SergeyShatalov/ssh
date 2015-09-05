
grey	= 8
rgb		= 4
idx		= 2
rle		= 1

extern asm_ssh_bc_x:near

OPTION NOKEYWORD:<width type>
;OPTION PROLOGUE:NONE
;OPTION EPILOGUE:NONE
;OPTION PROLOGUE:PrologueDef
;OPTION EPILOGUE:EpilogueDef

gif_fmt struct
	nMask	dd 0
	rowDict	dd 0
	nShift	dd 0
	CC		dd 0
	EOI		dd 0
gif_fmt ends

.data?

.const
ALIGN 16
_gamma		dd 0.3, 0.59, 0.11, 0.0, 0.3, 0.59, 0.11, 0.0
cnvFuncs	dq bgra8_,	_bgra8
			dq a8_,		_a8
			dq l8_,		_l8
			dq rgba8_,	_rgba8
			dq rgb8_,	_rgb8
			dq bgr8_,	_bgr8
			dq r5g6b5_,	_r5g6b5
			dq rgb5a1_,	_rgb5a1
			dq rgba4_,	_rgba4
			dq font_,	0

.data

.code

bgra8_swap	db 2, 1, 0, 3, 6, 5, 4, 7
bgra8_:	; a2 r2 g2 b2 a1 r1 g1 b1 -> a2 b2 g2 r2 a1 b1 g1 r1
_bgra8:	; a2 b2 g2 r2 a1 b1 g1 r1 -> a2 r2 g2 b2 a1 r1 g1 b1
		movq mm0, [r9]
		pshufb mm0, qword ptr bgra8_swap
		movq [r8], mm0
		add r9, 8
		add r8, 8
		ret
rgba8_:
_rgba8: mov rax, [r9]
		mov [r8], rax
		add r9, 8
		add r8, 8
		ret
rgb8_unpack	db 0, 1, 2, -1, 3, 4, 5, -1
rgb8_alpha	db 0, 0, 0, -1, 0, 0, 0, -1
rgb8_:	; y x b2 g2 r2 b1 g1 r1 -> 0 b2 g2 r2 0 b1 g1 r1
		movq mm0, [r9]
		pshufb mm0, qword ptr rgb8_unpack
		por mm0, qword ptr rgb8_alpha
		movq [r8], mm0
		add r9, 6
		add r8, 8
		ret
rgb8_pack db 0, 1, 2, 4, 5, 6, -1, -1
_rgb8:	; a2 b2 g2 r2 a1 b1 g1 r1 -> 00 00 b2 g2 r2 b1 g1 r1
		movq mm0, [r9]
		pshufb mm0, qword ptr rgb8_pack
		movd rax, mm0
		mov [r8], eax
		shr rax, 32
		mov [r8 + 4], ax
		add r9, 8
		add r8, 6
		ret
bgr8_unpack db 2, 1, 0, -1, 5, 4, 3, -1
bgr8_alpha	db 0, 0, 0, -1, 0, 0, 0, -1
bgr8_:	; y x r2 g2 b2 r1 g1 b1 -> 0 b2 g2 r2 0 b1 g1 r1
		movq mm0, [r9]
		mov eax, [r9]
		pshufb mm0, qword ptr bgr8_unpack
		por mm0, qword ptr bgr8_alpha
		movq [r8], mm0
		add r9, 6
		add r8, 8
		ret
bgr8_pack db 2, 1, 0, 6, 5, 4, -1, -1
_bgr8:	; a2 b2 g2 r2 a1 b1 g1 r1 -> 00 00 r2 g2 b2 r1 g1 b1
		movq mm0, [r9]
		pshufb mm0, qword ptr bgr8_pack
		movd rax, mm0
		mov [r8], eax
		shr rax, 32
		mov [r8 + 4], ax
		add r9, 8
		add r8, 6
		ret
a8_unpack db 1, 1, 1, 1, 0, 0, 0, 0
a8_alpha db 0, 0, 0, -1, 0, 0, 0, -1
a8_:	; x x x x x x a2 a1 -> a2 a2 a2 a2 a1 a1 a1 a1
		movzx rax, word ptr [r9]
		movd mm0, rax
		pshufb mm0, qword ptr a8_unpack
		por mm0, qword ptr a8_alpha
		movq [r8], mm0
		add r9, 2
		add r8, 8
		ret
a8_pack db 2, 7, -1, -1, -1, -1, -1, -1
_a8:	; a2 r2 g2 b2 a1 r1 g1 b1 -> 0 0 0 0 0 0 a2 a1
		movq mm0, [r9]
		pshufb mm0, qword ptr a8_pack
		movd rax, mm0
		mov [r8], ax
		add r9, 8
		add r8, 2
		ret
l8_unpack db 0, 0, 0, -1, 1, 1, 1, -1
l8_alpha  db 0, 0, 0, -1, 0, 0, 0, -1
l8_:	; x x x x x x l2 l1 -> 0 l2 l2 l2 0 l1 l1 l1
		movzx rax, word ptr [r9]
		movd mm0, rax
		pshufb mm0, qword ptr l8_unpack
		por mm0, qword ptr l8_alpha
		movq [r8], mm0
		add r9, 2
		add r8, 8
		ret
l8_pack db 2, 6, -1, -1, -1, -1, -1, -1
_l8:	; a2 r2 g2 b2 a1 r1 g1 b1 -> 0 0 0 0 0 0 l2 l1
		movq xmm0, qword ptr [r9]
		vpmovzxbd ymm0, xmm0
		vcvtdq2ps ymm0, ymm0
		vdpps ymm0, ymm0, ymm1, 01110111b
		vcvtps2dq ymm0, ymm0
		vpackusdw ymm0, ymm0, ymm0
		vpackuswb ymm0, ymm0, ymm0
		movd rax, xmm0
		mov [r8], al
		vextracti128 xmm0, ymm0, 1
		movd rax, xmm0
		mov [r8 + 1], al
		add r9, 8
		add r8, 2
		ret
font_:	movq xmm0, qword ptr [r9]
		vpmovzxbd ymm0, xmm0
		vcvtdq2ps ymm0, ymm0
		vdpps ymm0, ymm0, ymm1, 01110111b
		vcvtps2dq ymm0, ymm0
		vpackssdw ymm0, ymm0, ymm0
		vpackuswb ymm0, ymm0, ymm0
		movd dword ptr [r8], xmm0
		vextracti128 xmm0, ymm0, 1
		movd dword ptr [r8 + 4], xmm0
		add r9, 8
		add r8, 8
		ret
common_cnv:
		push rcx
		pext eax, r11d, [r12]			; b
		pext ecx, r11d, [r12 + 4]		; g
		pext edx, r11d, [r12 + 8]		; r
		pext ebx, r11d, [r12 + 12]		; a
		pdep eax, eax, [r12 + 16]
		pdep ecx, ecx, [r12 + 20]
		pdep edx, edx, [r12 + 24]
		pdep ebx, ebx, [r12 + 28]
		or eax, ecx
		or eax, edx
		test ebx, ebx
		pop rcx
		ret
r5g6b5_unpack	dd 00000000000000000000000000011111b, 00000000000000000000011111100000b, 00000000000000001111100000000000b, 0
				dd 00000000000000000000000011111000b, 00000000000000001111110000000000b, 00000000111110000000000000000000b, 0
r5g6b5_:mov r12, offset r5g6b5_unpack
		mov r11w, [r9]
		call common_cnv
		or eax, 0ff000000h
		mov [r8], eax
		mov r11d, [r9 + 2]
		call common_cnv
		or eax, 0ff000000h
		mov [r8 + 4], eax
		add r9, 4
		add r8, 8
		ret
r5g6b5_pack dd 00000000000000000000000011111000b, 00000000000000001111110000000000b, 00000000111110000000000000000000b, 0
			dd 00000000000000000000000000011111b, 00000000000000000000011111100000b, 00000000000000001111100000000000b, 0
_r5g6b5:mov r12, offset r5g6b5_pack
		mov r11d, [r9]
		call common_cnv
		mov [r8], ax
		mov r11d, [r9 + 4]
		call common_cnv
		mov [r8 + 2], ax
		add r9, 8
		add r8, 4
		ret
rgb5a1_unpack	dd 00000000000000000000000000011111b, 00000000000000000000001111100000b, 00000000000000000111110000000000b, 00000000000000001000000000000000b
				dd 00000000000000000000000011111000b, 00000000000000001111100000000000b, 00000000111110000000000000000000b, 00000000000000001000000000000000b
rgb5a1_:mov r12, offset rgb5a1_unpack
		mov r11d, [r9]
		call common_cnv
		or eax, 0ff000000h
@@:		mov [r8], eax
		mov r11w, [r9 + 2]
		call common_cnv
		or eax, 0ff000000h
@@:		mov [r8 + 4], eax
		add r9, 4
		add r8, 8
		ret
rgb5a1_pack dd 00000000000000000000000011111000b, 00000000000000001111100000000000b, 00000000111110000000000000000000b, 11100000000000000000000000000000b
			dd 00000000000000000000000000011111b, 00000000000000000000001111100000b, 00000000000000000111110000000000b, 00000000000000001000000000000000b
_rgb5a1:mov r12, offset rgb5a1_pack
		mov r11d, [r9]
		call common_cnv
		or eax, ebx
@@:		mov [r8], ax
		mov r11d, [r9 + 4]
		call common_cnv
		or eax, ebx
@@:		mov [r8 + 2], ax
		add r9, 8
		add r8, 4
		ret
rgba4_unpack	dd 00000000000000000000000000001111b, 00000000000000000000000011110000b, 00000000000000000000111100000000b, 00000000000000001111000000000000b
				dd 00000000000000000000000011110000b, 00000000000000001111000000000000b, 00000000111100000000000000000000b, 00000000000000001111000000000000b
rgba4_:	mov r12, offset rgba4_unpack
		mov r11w, [r9]
		call common_cnv
		jz @f
		or eax, 0ff000000h
@@:		mov [r8], eax
		mov r11w, [r9 + 2]
		call common_cnv
		jz @f
		or eax, 0ff000000h
@@:		mov [r8 + 4], eax
		add r9, 4
		add r8, 8
		ret
rgba4_pack	dd 00000000000000000000000011110000b, 00000000000000001111000000000000b, 00000000111100000000000000000000b, 11110000000000000000000000000000b
			dd 00000000000000000000000000001111b, 00000000000000000000000011110000b, 00000000000000000000111100000000b, 00000000000000001111000000000000b
_rgba4:	mov r12, offset rgba4_pack
		mov r11d, [r9]
		call common_cnv
		or eax, ebx
		mov [r8], ax
		mov r11d, [r9 + 4]
		call common_cnv
		or eax, ebx
		mov [r8 + 2], ax
		add r9, 8
		add r8, 4
		ret

;rcx(fmt), rdx(wh), r8(dst), r9(src), is
asm_ssh_cnv proc public USES r10 r11 r12 rbx rsi rdi r13 r14 r15 fmt:DWORD, wh:QWORD, dst:QWORD, src:QWORD, is:DWORD
		vmovups ymm1, _gamma
		movsxd r11, is
		movsxd rax, ecx
		movsxd rcx, dword ptr [rdx]
		cmp rax, 5						; проверить на запакованные форматы
		jl _bc_x
		mov r10, offset cnvFuncs - 5 * 16
		shl rax, 4
		add r10, rax
		lea r10, [r10 + r11 * 8]		; адрес функции преобразования
		imul ecx, dword ptr [rdx + 4]
		shr rcx, 1
		jz _fin
@@:		call qword ptr [r10]
		loop @b
		emms
_fin:	ret
_bc_x:	call asm_ssh_bc_x
		ret
asm_ssh_cnv endp

; rcx(ww) rdx(pal) r8(dst) r9(src)
asm_ssh_unpack_tga proc public USES r10 r11 r12 r13 rbx wh:QWORD, pal:QWORD, dst:QWORD, src:QWORD, bpp:DWORD, flags:DWORD
		movsxd rax, dword ptr [rcx]
		movsxd rcx, dword ptr [rcx + 4]
		imul rcx, rax
		mov rax, 1
		movsxd r10, bpp
		movsxd r11, flags
		mov r13, r10
		test r11, idx		; если палитра - bpp = 1
		cmovnz r13, rax
		xor rbx, rbx
_loop:	mov r12, r13
		xor rax, rax
		test r11, rle
		jz @f
		mov bl, [r9]
		inc r9
		test bl, 128
		cmovnz r12, rax
		and bl, 127
@@:		inc bl				; длина последовательности
		sub rcx, rbx
_pack:	mov eax, [r9]		; из src в eax
		add r9, r12
		test r11, idx
		jz @f
		movzx rax, al
		imul rax, 3
		mov eax, [rax + rdx]; реальное значение из палитры
@@:		mov [r8], eax		; сохраняем значение в dst
		add r8, r10
		dec bl
		jnz _pack
		test r12, r12
		jnz @f
		add r9, r13
@@:		test rcx, rcx
		jg _loop
		ret
asm_ssh_unpack_tga endp

; rcx(w) rdx(h) r8(dst) r9(src)
asm_ssh_unpack_bmp proc public USES r10 r11 r12 rbx rdi w:QWORD, h:QWORD, dst:QWORD, src:QWORD, pal:QWORD
		mov r10, pal
_height:mov rdi, rcx
_width:	xor r11, r11
		movzx rbx, byte ptr [r9]			; длина последовательности
		movzx rax, byte ptr [r9 + 1]		; из src в eax
		inc r9
		test rbx, rbx
		jnz @f
		mov rbx, rax
		inc r11
		inc r9
@@:		sub rdi, rbx
		jge unpack
		or rdi, rdi
unpack:	movzx rax, byte ptr [r9]
		add r9, r11
@@:		mov r12d, dword ptr [rax * 4 + r10]	; реальное значение из палитры
		mov [r8], r12d						; сохраняем значение в dst
		add r8, 3
		dec rbx
		jnz unpack
		test r11, r11
		jz skip
		cmp byte ptr [r9], 0
		jnz skip
		inc r9
skip:	sub r9, r11
		inc r9
		test rdi, rdi
		jg _width
		dec r9
@@:		inc r9
		mov al, [r9]
		test al, al
		jz @b
		dec rdx
		jnz _height
		ret
asm_ssh_unpack_bmp endp

asm_ssh_bfs proc public
		xor rax, rax
		mov eax, [rcx]
		mov rcx, rax
		mov rdx, rax
		and rax, 00f0f0f0h
		jz _fin
		mov rax, rcx
		or eax, 0ff000000h
		jmp _fin
		and rdx, 00a0a0a0h
		jz _fin
		movd xmm7, ecx
		pmovzxbd xmm7, xmm7
		cvtdq2ps xmm7, xmm7
		dpps xmm7, xmm5, 01111000b
		cvtps2dq xmm7, xmm7
		packssdw xmm7, xmm7
		packuswb xmm7, xmm7
		movd eax, xmm7
		and rax, 0ff000000h
		and rcx, 00ffffffh
		or rax, rcx
_fin:	ret
asm_ssh_bfs endp

;rcx(iTrans) rdx(pal) r8(dst) r9(stk)
asm_ssh_unpack_gif proc public iTrans:DWORD, pal:QWORD, dst:QWORD, src:QWORD, stk:QWORD
LOCAL temp[1024]:BYTE	
LOCAL s_dict[4096]:DWORD
LOCAL v_dict[512]:QWORD
		push rsi
		push rdi
		push rbx
		push r11
		push r12
		push r13
		push r14
		push r15
		movq mm1, qword ptr shift1
		movq mm2, qword ptr shift2
		movq mm3, qword ptr shift3
		mov iTrans, ecx
		mov pal, rdx
		mov rdi, r8
		mov r10, stk
		lea r11, s_dict
		lea r12, v_dict
		movsxd rdx, [r10].gif_fmt.nMask					; nMask
		xor rcx, rcx									; shift
		movsxd rax, [r10].gif_fmt.nShift				; nShift
		mov ch, al
		movzx rbx, byte ptr [r9]
		inc r9
		lea r15, [r9 + rbx - 2]
_loop:	call get_val
		shr rax, cl											; value >>= shift
		and rax, rdx										; value &= nMask
		add cl, ch											; shift += nShift
		movzx rbx, cl
		shr rbx, 3
		add r9, rbx											; src += shift >> 3
		and cl, 7											; shift &= 7
		cmp eax, [r10].gif_fmt.EOI
		jz _fin
		cmp eax, [r10].gif_fmt.CC
		jnz _no_cc
		xor rax, rax
		movsxd r13, [r10].gif_fmt.EOI					; pos
@@:		mov [r12 + rax], al
		mov word ptr [r11 + rax * 2], -2
		inc rax
		cmp rax, r13
		jbe @b
		mov r13, rax									; pos
		movsxd r14, [r10].gif_fmt.rowDict				; max_pos
		movsxd rax, [r10].gif_fmt.nShift				; nShift
		mov ch, al
		movsxd rdx, [r10].gif_fmt.nMask					; nMask
		call get_val
		shr rax, cl
		and rax, rdx
		add cl, ch
		movzx rbx, cl
		shr rbx, 3
		add r9, rbx										; src += shift >> 3
		and cl, 7										; shift &= 7
		call _unpack
		jmp _next0
_no_cc:	mov [r11 + r13 * 2], r8w						; s_dict[pos] = v_old
		cmp word ptr [r11 + rax * 2], -1
		jnz @f
		mov bl, [r12 + r8]
		mov [r12 + r13], bl
		jmp _next
@@:		mov rsi, rax									; tmp = value
@@:		mov bl, [r12 + rsi]
		movzx rsi, word ptr [r11 + rsi * 2]
		cmp rsi, 0fffeh
		jb @b
		mov [r12 + r13], bl
_next:	call _unpack
		inc r13
_next0:	mov r8, rax										; v_old = value
		cmp r13, r14									; pos >= max_pos
		jb _loop
		cmp r13, 4096
		jae _loop
		shl r14, 1
		inc ch
		shl rdx, 1
		or rdx, 3
		jmp _loop
_fin:	pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop rbx
		pop rdi
		pop rsi
		emms
		ret
shift1 db 0, 1, 3, -1, -1, -1, -1, -1
shift2 db 0, 2, 3, -1, -1, -1, -1, -1
shift3 db 1, 2, 3, -1, -1, -1, -1, -1
OPTION EPILOGUE:NONE
get_val:mov eax, [r9]
		cmp r9, r15
		jb _exit
		mov rbx, r9
		sub rbx, r15
		movd mm0, rax
		movq mm4, mm1
		test rbx, rbx
		jz @f
		movq mm4, mm2
		cmp rbx, 1
		jz @f
		movq mm4, mm3
@@:		pshufb mm0, mm4
		movd rax, mm0
		cmp rbx, 1
		jbe _exit
		movzx rsi, byte ptr [r15 + 2]
		lea r15, [r15 + rsi + 1]
		inc r9
_exit:	ret
_unpack:push rax
		push rcx
		push rdx
		lea rsi, temp
		mov rdx, pal
		xor rbx, rbx
		xor rcx, rcx									; sz
@@:		mov bl, [r12 + rax]
		mov [rsi + rcx], bl
		movzx rax, word ptr [r11 + rax * 2]
		inc rcx
		cmp ax, -2
		jb @b
@@:		movzx rax, byte ptr [rsi + rcx - 1]				; индекс в палитре
		cmp eax, iTrans
		setz bl
		lea rax, [rax * 2 + rax]
		movd mm0, dword ptr [rdx + rax]
		pshufb mm0, qword ptr bgr8_unpack
		movd eax, mm0
		dec bl
		shl ebx, 24
		or eax, ebx
		stosd
		loop @b
		pop rdx
		pop rcx
		pop rax
		ret
OPTION EPILOGUE:EpilogueDef
asm_ssh_unpack_gif endp

end
