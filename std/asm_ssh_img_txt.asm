
include asm_ssh.inc

.const

.data?

.data

.code

;void asm_ssh_copy_wchar(const Bar<int>& pos, ssh_u height, void* pix, void* ptex, ssh_u pitch);
asm_ssh_copy_wchar proc USES r11 r12 pos:QWORD, h:QWORD, dst:QWORD, src:QWORD, pitch_dst:QWORD
		movaps xmm1, _gamma
		mov r10, pitch_dst
		movsxd rax, dword ptr [rcx].stk_bar.x
		movsxd r11, dword ptr [rcx].stk_bar.y
		imul r11, r10
		lea r11, [r11 + rax * 4]
		add r8, r11
		movsxd rcx, dword ptr [rcx].stk_bar.h
		lea rax, [rcx * 4]
		sub r10, rax
		mov r11, 512							; 128 * 4
		sub r11, rax
		mov r12, rcx
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
		add r8, r10
		add r9, r11
		mov rcx, r12
		dec rdx
		jnz @b
		ret
asm_ssh_copy_wchar endp

asm_ssh_compute_width_wchar proc USES rdi ptex:QWORD
		xor rax, rax
@@:		cmp dword ptr [rcx + rax * 4], 0
		jz @f
		inc rax
		cmp rax, 128
		jl @b
@@:		ret
asm_ssh_compute_width_wchar endp

end
