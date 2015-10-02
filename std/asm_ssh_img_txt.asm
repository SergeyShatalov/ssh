
include asm_ssh.inc

.const

.data?

.data
num dq 0

.code

asm_ssh_copy_wchar proc USES r11 r12 pos:QWORD, pt:QWORD, pix:QWORD, ptex:QWORD, pitch_pix:QWORD, pitch_tex:QWORD, h:QWORD
		inc num
		mov r12, [num]
		xorps xmm6, xmm6
		movaps xmm1, _gamma
		mov r10, pitch_pix
		mov r11, pitch_tex
		movsxd rax, dword ptr [rcx].stk_bar.x
		movsxd rcx, dword ptr [rcx].stk_bar.y
		imul rcx, r10
		lea rcx, [rcx + rax * 4]
		add r8, rcx
		movsxd rax, dword ptr [rdx].stk_bar.x
		movsxd rcx, dword ptr [rdx].stk_bar.y
		imul rcx, r11
		lea rcx, [rcx + rax * 4]
		add r9, rcx
		movsxd rcx, dword ptr [rdx].stk_bar.h
		mov rdx, h
		lea rax, [rcx * 4]
		;sub r10, rax
		;sub r11, rax
		;mov r12, rcx
_loop:	push r9
		push r8
		push rcx
@@:		pmovzxbd xmm0, dword ptr [r9]
		cvtdq2ps xmm0, xmm0
		dpps xmm0, xmm1, 01111111b
		cvtps2dq xmm0, xmm0
		packssdw xmm0, xmm0
		packuswb xmm0, xmm0
		movd dword ptr [r8], xmm0
		add r8, 4
		add r9, 4
		loop @b
		pop rcx
		pop r8
		pop r9
		add r8, r10
		add r9, r11
		;mov rcx, r12
		dec rdx
		jnz _loop
		ret
asm_ssh_copy_wchar endp

asm_ssh_compute_width_wchar proc x:QWORD, y:QWORD, ptex:QWORD, pitch:QWORD
		imul rdx, r9
		lea rdx, [rdx + rcx * 4]
		add r8, rdx
		xor rax, rax
@@:		cmp dword ptr [r8 + rax * 4], 0
		jz @f
		inc rax
		cmp rax, 256
		jl @b
@@:		ret
asm_ssh_compute_width_wchar endp

end
