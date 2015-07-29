
.data?

len_val		dq ?			; длина полученного значения
znak_val	dq ?			; признак знака
result		dw 256 dup(?)

.data

tbl_por		dq 1.0
			dq 10.0
			dq 100.0
			dq 1000.0
			dq 10000.0
			dq 100000.0
			dq 1000000.0
			dq 10000000.0
tbl_por2	dq -1.0
			dq -10.0
			dq -100.0
			dq -1000.0
			dq -10000.0
			dq -100000.0
			dq -1000000.0
			dq -10000000.0
hex_sym		dw 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70
is_hex		dw 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
.code

; преобразовать число в строку, в зависимости от системы счисления (0-bin, 1-decimal, 2-oct, 3-hex, 4-double)
; rcx - число
; rdx - система счисления
; ret - result of whar_t*
asm_ssh_ntow proc public
		push r10
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
		pop r10
		ret
tbl_radix dq nto_ohb, 1, 1, nto_dec, 0, 10, nto_ohb, 7, 3, nto_ohb, 15, 4, nto_dbl, 0, 10, nto_flt, 0, 10
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
		cvtss2sd xmm0, xmm0
		jmp @f
nto_dbl:; read double from rax
		movd xmm0, rax
		; отбросим дробную часть
@@:		cvttsd2si rax, xmm0
		cvtsi2sd xmm1, rax
		; преобразуем целую часть в строку
		call nto_dec
		mov rdx, offset result + 128
		mov r8, offset tbl_por + 8
		mov word ptr [rdx], '.'
		add rdx, 2
		; количество знаков после запятой
		mov rcx, 4
@@:		; дробная часть
		subsd xmm0, xmm1
		mulsd xmm0, qword ptr [r8]
		cvttsd2si rax, xmm0
		cvtsi2sd xmm1, rax
		add rax, 48
		mov [rdx], ax
		add rdx, 2
		loop @b
		mov word ptr [rdx], 0
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
radix	dq wto_obh, 2, _BIN, wto_dec, 10, _DEC, wto_obh, 8, _OCT, wto_obh, 16, _HEX, wto_dbl, 10, _DEC, wto_flt, 10, _DEC
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
		ret
wto_flt:inc r13
wto_dbl:call wto_dec
		mov r10, r11
		mov rcx, r11
		cvtsi2sd xmm0, rax
		xorps xmm1, xmm1
		cmp word ptr [rcx], '.'
		jnz @f
		mov r8, 1
		add rcx, 2
		call wto_obh
		cvtsi2sd xmm1, rax
@@:		sub r11, r10
		shr r11, 1
		and r12, 7
		mov r8, offset tbl_por
		shl r12, 6
		add r8, r12
		divsd xmm1, qword ptr [r8]
		addsd xmm0, xmm1
		test r13, r13
		jz @f
		cvtsd2ss xmm0, xmm0
@@:		movsd qword ptr [result], xmm0
		mov rax, qword ptr [result]
		ret
asm_ssh_wton endp

end

asmAtoF proc public USES rdi rsi rbx
; целая часть
		mov r9, offset tbl_por
		xorps xmm1, xmm1
		mov r8, rcx
		push rcx
		push 0
		call atol
		cvtsi2sd xmm0, rax
; найти разделяющую точку (если есть)
		mov rsi, r8
_snova:	lodsb
		test al, al
		jz _fin
		cmp al, '.'
		jnz _snova
		mov rdi, rsi
		dec rsi
		sub rsi, r8
		cmp rsi, len_val
		jnz _fin
; дробная часть
		push rdi
		push 1
		call atol
		cvtsi2sd xmm1, rax
		mov rdx, znak_val
		shl rdx, 6
		add r9, rdx
		mov rcx, len_val
		and rcx, 7
		divsd xmm1, qword ptr [rcx * 8 + r9]
_fin:	addsd xmm0, xmm1
		ret
asmAtoF endp

