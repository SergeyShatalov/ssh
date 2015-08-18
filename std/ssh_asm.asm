
extern malloc:near

.data?

result		dw 128 dup(?)
temp_print	dw 512 dup(?)

.data

dbl_znak	dq 1.0, -1.0
			dq 1.0
			dq 10.0
			dq 100.0
			dq 1000.0
			dq 10000.0
			dq 100000.0
			dq 1000000.0
			dq 10000000.0
			dq 100000000.0
			dq 1000000000.0
			dq 10000000000.0
			dq 100000000000.0
hex_sym		dw 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70
is_hex		dw 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
cpu_caps_1	db 8, 0, 4, 0, 8, 1, 8, 0, 8, 9, 5, 0, 8, 12, 9, 0, 8, 13, 10, 0, 8, 19, 6, 0
			db 8, 20, 7, 0, 8, 22, 11, 0, 8, 23, 12, 0, 8, 25, 13, 0, 8, 28, 14, 0, 8, 29, 25, 0, 8, 30, 15, 0
			db 12, 8, 31, 0, 12, 11, 31, 0, 12, 15, 16, 0, 12, 19, 31, 0, 12, 23, 1, 0, 12, 24, 31, 0, 12, 25, 2, 0, 12, 26, 3, 0, 0, 0, 0, 0
cpu_caps_7	db 4, 3, 17, 0, 4, 5, 18, 0, 4, 8, 19, 0, 4, 16, 20, 0, 4, 18, 21, 0, 4, 26, 22, 0, 4, 27, 23, 0, 4, 28, 24, 0, 4, 29, 31, 0, 0, 0, 0, 0
base64_chars	dw 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90
				dw 97, 98, 99, 100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122
				dw 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 43, 47, 0

.code

; преобразовать число в строку, в зависимости от системы счисления 0-decimal, (1-bin, 2-oct, 3-hex, 4-float,5-double)
; rcx - число
; rdx - система счисления
; ret - result of whar_t*
asm_ssh_ntow proc public
		push r10
		push r11
		mov r9, offset result + 126
		add r9, 2
		mov word ptr [r9], 0
		jrcxz @f
		mov rax, [rcx]
		imul rdx, 24
		mov r10, offset tbl_radix
		add rdx, r10
		mov r8, offset hex_sym
		mov r10, [rdx + 8]
		mov rcx, [rdx + 16]
		call qword ptr [rdx]
@@:		mov rax, r9
		pop r11
		pop r10
		ret
tbl_radix dq nto_dec, 0, 10, nto_ohb, 1, 1, nto_ohb, 7, 3, nto_ohb, 15, 4, nto_dbl, 0, 10, nto_flt, 0, 10
nto_ohb:sub r9, 2
		mov rdx, rax
		and rdx, r10
		movzx rdx, word ptr [rdx * 2 + r8]
		mov [r9], dx
		shr rax, cl
		jnz nto_ohb
		ret
nto_dec:; определяем знак
		sub r9, 2
		test rax, rax
		jns @f
		mov r10w, '-'
		neg rax
@@:		cmp rax, rcx
		jb @f
		xor rdx, rdx
		div rcx
		or dl, 48
		mov word ptr [r9], dx
		sub r9, 2
		jmp @b
@@:		or al, 48
		mov word ptr [r9], ax
		test r10, r10
		jz @f
		sub r9, 2
		mov [r9], r10w
@@:		ret
nto_flt:movd xmm0, eax
		mov r11, 4
		cvtss2sd xmm0, xmm0
		jmp @f
nto_dbl:; read double from rax
		mov r11, 8
		movd xmm0, rax
		; отбросим дробную часть
@@:		cvttsd2si rax, xmm0
		cvtsi2sd xmm1, rax
		; преобразуем целую часть в строку
		call nto_dec
		mov rdx, offset result + 128
		mov r8, offset dbl_znak + 24
		mov word ptr [rdx], '.'
		add rdx, 2
		; количество знаков после запятой
		mov rcx, r11
		mov r10, rdx
@@:		; дробная часть
		subsd xmm0, xmm1
		mulsd xmm0, qword ptr [r8]
		cvttsd2si rax, xmm0
		cvtsi2sd xmm1, rax
		test rax, rax
		cmovnz r10, rdx
		add rax, 48
		mov [rdx], ax
		add rdx, 2
		loop @b
		mov word ptr [r10 + 2], 0
		ret
asm_ssh_ntow endp

asm_ssh_wton proc public
		push rbx
		push r10
		push r11
		push r12
		push r13
		xor rax, rax
		jrcxz @f
		mov r10, rcx
		xor r13, r13
		mov r9, offset radix
		imul rdx,24
		add rdx, r9
		mov r9, [rdx + 8]; множитель порядка
		mov r8, [rdx + 16]; маска
		call qword ptr [rdx]
@@:		mov qword ptr [result], rax
		pop r13
		pop r12
		pop r11
		pop r10
		pop rbx
		mov rax, offset result
		ret
_BIN	= 1
_OCT	= 2
_HEX	= 4
_DEC	= 8
tbl_hex dw 0, _HEX, _HEX, _HEX, _HEX, _HEX, _HEX, 0, 0, 0, 0, 0, 0, 0, 0, 0, _DEC + _BIN + _OCT + _HEX, _DEC + _BIN + _OCT + _HEX, _DEC + _OCT + _HEX, _DEC + _OCT + _HEX, _DEC + _OCT + _HEX
		dw _DEC + _OCT + _HEX, _DEC + _OCT + _HEX, _DEC + _OCT + _HEX, _DEC + _HEX, _DEC + _HEX
radix	dq wto_dec, 10, _DEC, wto_obh, 2, _BIN, wto_obh, 8, _OCT, wto_obh, 16, _HEX, wto_dbl, 10, _DEC, wto_flt, 10, _DEC
wto_obh:sub rcx, 2
		mov rdx, offset tbl_hex
@@:		add rcx, 2
		movzx rax, word ptr [rcx]
		and rax, -97
		cmp rax, 25
		ja @f
		movzx rax, word ptr [rax * 2 + rdx]
		and rax, r8
		jnz @b
@@:		mov r11, rcx
		mov r8, 1
		xor rax, rax
		mov rdx, offset is_hex
@@:		sub rcx, 2
		cmp rcx, r10
		jb @f
		movzx rbx, word ptr [rcx]
		and rbx, -97
		movzx rbx, word ptr [rbx * 2 + rdx]
		imul rbx, r8
		add rax, rbx
		imul r8, r9
		jmp @b
@@:		ret
wto_dec:; определяем признак отрицательного значения
		xor r12, r12
		cmp word ptr [rcx], '-'
		setz r12b
		lea rcx, [rcx + r12 * 2]
		call wto_obh
		test r12, r12
		jz @f
		neg rax
@@:		ret
wto_flt:inc r13
wto_dbl:call wto_dec
		mov r10, r11
		cvtsi2sd xmm0, rax
		xorps xmm1, xmm1
		cmp word ptr [r11], '.'
		jnz @f
		lea rcx, [r11 + 2]
		mov r10, rcx
		call wto_obh
		cvtsi2sd xmm1, rax
@@:		sub r11, r10
		mov rcx, 22
		cmp r11, rcx
		cmova r11, rcx
		mov r8, offset dbl_znak
		shl r12, 1
		divsd xmm1, qword ptr [r8 + r11 * 4 + 16]
		addsd xmm0, xmm1
		mulsd xmm0, qword ptr [r8 + r12 * 8]
		test r13, r13
		jz @f
		cvtsd2ss xmm0, xmm0
@@:		movd rax, xmm0
		ret
asm_ssh_wton endp

; 0 - register(offs), 1 - bit check, 2 - bit set 
asm_ssh_capability proc
		push rbx
		push r10
		xor r10, r10
		mov r9, offset result
		mov r8, offset cpu_caps_1
		mov rax, 1
		xor rcx, rcx
		call _cset
		mov rax, 7
		xor rcx, rcx
		mov r8, offset cpu_caps_7
		call _cset
		mov rax, r10
		pop r10
		pop rbx
		ret
_cset:	cpuid
		mov [r9 + 00], rax
		mov [r9 + 08], rbx
		mov [r9 + 16], rcx
		mov [r9 + 24], rdx
@@:		cmp dword ptr [r8], 0
		jz @f
		movzx rax, byte ptr [r8 + 00]		; offset
		movzx rcx, byte ptr [r8 + 01]		; bit check
		movzx rdx, byte ptr [r8 + 02]		; bit set
		add r8, 4
		mov rax, [r9 + rax * 2]
		bt rax, rcx
		jnc @b
		bts r10, rdx
		jmp @b
@@:		ret
asm_ssh_capability endp

;rcx = buf, rdx = count
asm_ssh_to_base64 proc
		push rsi
		push rdi
		push rbx
		push r12
		mov r8, 3
		mov rax, rdx
		mov r12, rax				; src_len
		mov rsi, rcx				; src_buf
		xor rdx, rdx
		div r8
		test rdx, rdx
		setnz dl
		lea rcx, [rax + rdx + 2]
		shl rcx, 3					; len_ret
		call malloc
		mov qword ptr [rax], 0
		lea rdi, [rax + 8]			; buf_ret
		push rdi
		mov r8, offset base64_chars
		mov r9, offset result		; char_array_3
_loop:	xor rdx, rdx				; i = 0
@@:		test r12, r12
		jz @f
		dec r12
		lodsb
		mov [r9 + rdx], al
		inc rdx
		cmp rdx, 3
		jnz @b
		call _sub
		jmp _loop
@@:		test rdx, rdx
		jz @f
		mov dword ptr [r9 + rdx], 0
		mov r12, 3
		sub r12, rdx
		call _sub
		sub rdi, r12
		sub rdi, r12
		mov ax, '='
		mov rcx, r12
		rep stosw
@@:		mov word ptr [rdi], 0
		pop rax
		pop r12
		pop rbx
		pop rdi
		pop rsi
		ret
_sub:	movzx rcx, byte ptr [r9 + 00]
		movzx rdx, byte ptr [r9 + 01]
		movzx rbx, byte ptr [r9 + 02]
		mov rax, rcx
		and rax, 0fch
		shr rax, 2
		mov ax, [r8 + rax * 2]
		stosw
		mov rax, rdx
		and rcx, 3
		and rax, 0f0h
		shl rcx, 4
		shr rax, 4
		add rax, rcx
		mov ax, [r8 + rax * 2]
		stosw
		mov rax, rbx
		and rdx, 15
		and rax, 0c0h
		shl rdx, 2
		shr rax, 6
		add rax, rdx
		mov ax, [r8 + rax * 2]
		stosw
		and rbx, 3fh
		mov ax, [r8 + rbx * 2]
		stosw
		ret
asm_ssh_to_base64 endp

; rcx - str, rdx - length, r8 - len_buf, r9 - is_null
asm_ssh_from_base64 proc
		push rbx
		push rsi
		push rdi
		push r10
		push r12
		mov rsi, rcx			; src
		lea r12, [rcx + rdx * 2]
		mov r10, rsi
		mov rcx, 3dh
		call _wcschr
		cmovc r12, rbx			; in_len
		mov rax, rdx
		shr rax, 2
		imul rax, 3
		sub rdx, r12
		sub rax, rdx			; out_len
		add rax, r9
		mov [r8], rax
		lea rcx, [rax + 14]		; 8 + 6 корекция на выход за пределы
		call malloc
		mov qword ptr [rax], 0
		lea rdi, [rax + 8]		; out_buf
		push rdi
		mov r10, offset base64_chars
		mov r9, offset result	; char_array_4
_loop:	xor rdx, rdx			; i = 0
@@:		test r12, r12
		jz @f
		dec r12
		lodsw
		mov cx, ax
		call _wcschr
		jnc @f
		mov [r9 + rdx * 2], bx
		inc rdx
		cmp rdx, 4
		jnz @b
		call _sub
		jmp _loop
@@:		test rdx, rdx
		jz @f
		mov qword ptr [r9 + rdx * 2], 0
		call _sub
@@:		mov word ptr [rdi], 0
		pop rax
		pop r12
		pop r10
		pop rdi
		pop rsi
		pop rbx
		ret
_sub:	mov rbx, rdx
		movzx rcx, byte ptr [r9 + 00]
		shl rcx, 2
		movzx rdx, byte ptr [r9 + 02]
		mov rax, rdx
		and rax, 30h
		shr rax, 4
		add rax, rcx
		stosb
		dec rbx
		jle @f
		movzx rcx, byte ptr [r9 + 04]
		and rdx, 15
		shl rdx, 4
		mov rax, rcx
		and rax, 3ch
		shr rax, 2
		add rax, rdx
		stosb
		dec rbx
		jle @f
		mov rax, rcx
		and rax, 3
		shl rax, 6
		add al, [r9 + 06]
		stosb
@@:		ret
_wcschr:mov rbx, -1
@@:		inc rbx
		mov ax, [r10 + rbx * 2]
		test ax, ax
		jz @f
		cmp ax, cx
		jnz @b
		stc
@@:		ret
asm_ssh_from_base64 endp

;rcx - src, rdx - vec
;<[/]tag [attr = value ...][/]>
;vec - 0 - <[/], 1- tag, 2 - [/]>, 3.... - attrs
xml_chars	db 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2
			db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 0, 0, 1
			db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1
			db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
asm_ssh_parse_xml proc
		push rbx
		push r10
		push r11
		push r12
		mov r11, 002d002d0021003ch
		mov r12, 003e002d002d0020h
		xor r8, r8					; cur src
		xor rbx, rbx				; count attrs
		call _skip_c
		test rax, rax
		jz _fin
		cmp rax, '<'
		jnz _err1
		mov r10, offset xml_chars
		lea r9, [r8 + 1]
		cmp word ptr [rcx + r9 * 2], '/'
		jnz @f
		inc r9
@@:		mov rax, 1
		call _set_v
		mov r8, r9
		call _skip_s
		; tag
		call _word
		mov rax, 2
		call _set_v
		mov r8, r9
@@:		call _skip_s
		cmp rax, '/'
		jz @f
		cmp rax, '>'
		jz @f
		; attrs - name
		call _word
		lea rax, [rbx + 5]
		call _set_v
		mov r8, r9
		call _skip_s
		cmp rax, '='
		jnz _err
		inc r8
		call _skip_s
		; attr - value
		call _str
		jnc _err1
		inc r8
		lea rax, [rbx + 6]
		call _set_v
		lea r8, [r9 + 1]
		add rbx, 2
		jmp @b
@@:		lea r9, [r8 + 1]
		cmp rax, '/'
		jnz @f
		cmp word ptr [rcx + r9 * 2], '>'
		jnz _err1
		inc r9
@@:		mov rax, 3
		call _set_v
		; value tag
		mov r8, r9
		call _skip_c
		call _str
		jnc @f
		inc r8
		mov rax, 4
		call _set_v
		inc r9
		jmp _ex
@@:		mov dword ptr [rdx + 16], 0
_ex:	xor rax, rax
		movzx r8, word ptr [rdx + 4]
		call _set_v
		lea rax, [rbx + 3]
		pop r12
		pop r11
		pop r10
		pop rbx
		ret
_skip_c:call _skip_s
;<!-- sergey -->
		cmp qword ptr [rcx + r8 * 2], r11
		jnz _skip_cc
		add r8, 3
@@:		inc r8
		movzx rax, word ptr [rcx + r8 * 2]
		test rax, rax
		jz _err
		cmp rax, '>'
		jnz @b
		cmp qword ptr [rcx + r8 * 2 - 6], r12
		jnz @b
		inc r8
		call _skip_s
_skip_cc:ret
_skip_s:dec r8
@@:		inc r8
		movzx rax, word ptr [rcx + r8 * 2]
		cmp rax, 33
		jae @f
		test rax, rax
		jnz @b
@@:		ret
_set_v: mov [rdx + rax * 4], r8w
		mov [rdx + rax * 4 + 2], r9w
		ret
_word:	lea r9, [r8 - 1]
		cmp rax, 48
		jb _err
		cmp rax, 58
		jb _err
@@:		inc r9
		movzx rax, word ptr [rcx + r9 * 2]
		cmp rax, 128
		jae @b
		movzx rax, byte ptr [rax + r10]
		test rax, rax		; undef
		jz _err
		dec rax				; word
		jz @b
		ret
_err:	add rsp, 8
_err1:	mov rax, -1
		ret
_fin:	xor rax, rax
		ret
_str:	cmp rax, 34
		jnz _str1
		mov r9, r8
@@:		inc r9
		movzx rax, word ptr [rcx + r9 * 2]
		test rax, rax
		jz _str1
		cmp rax, 34
		jnz @b
		stc
		ret
_str1:	clc
		ret
asm_ssh_parse_xml endp

end
