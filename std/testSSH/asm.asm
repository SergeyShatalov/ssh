
.data
align 16
ind		db 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0
v1		dd 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255
v2		dd 32 dup(0)
v3		dd -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0
and0	dw 0, -1, 0, -1, 0, -1, 0, -1
and1	dw -1, 0, -1, 0, -1, 0, -1, 0
y_shift dd 1, 1, 1, 1, 1, 1, 1, 1
msk		db -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.code

asm_ssh_shufb proc public
		mov rsi, offset ind
		mov rdi, offset v2
		vpmovzxbd xmm0, dword ptr [rsi + 00]
		vpmovzxbd xmm1, dword ptr [rsi + 04]
		vpmovzxbd xmm2, dword ptr [rsi + 08]
		vpmovzxbd xmm3, dword ptr [rsi + 12]
		vpslld xmm1, xmm1, 2
		vpslld xmm2, xmm2, 4
		vpslld xmm3, xmm3, 6
		vorps xmm1, xmm1, xmm2
		vorps xmm0, xmm0, xmm1
		vorps xmm0, xmm0, xmm3
		vpackusdw xmm0, xmm0, xmm0
		vpackuswb xmm0, xmm0, xmm0
		movd dword ptr [rdi], xmm0
		ret
		mov rsi, offset v1
		mov rdi, offset v2
		movups xmm3, xmmword ptr msk
		mov rcx, 2
@@:		vpshufd ymm1, [rsi], 00001000b
		vpshufd ymm2, [rsi], 00001101b
		vpsrld ymm1, ymm1, 4
		vpsrld ymm2, ymm2, 4
		vpslld ymm2, ymm2, 4
		vorps ymm1, ymm1, ymm2
		vpackusdw ymm1, ymm1, ymm1
		vpackuswb ymm1, ymm1, ymm1
		vextractf128 xmm2, ymm1, 1
		vmaskmovdqu xmm1, xmm3
		add rdi, 2
		maskmovdqu xmm2, xmm3
		add rdi, 2
		add rsi, 32
		loop @b
		mov r8, rdi
		ret
		xor rax, rax
@@:		mov ecx, [r11 + rax * 8]
		mov edx, [r11 + rax * 8 + 4]
		shr rcx, 4
		shr edx, 4
		shl dl, 4
		or cl, dl
		mov [r8], cl
		inc r8
		inc rax
		cmp rax, 8
		jb @b
		ret
asm_ssh_shufb endp

end
