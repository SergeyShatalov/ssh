
extern asm_clip_bar:near
.data
align 16
;_tmp	dd 0, 0, 0, 0
f_255x8	dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
grid	dd 31.0, 63.0, 31.0, 0.0
half	dd 0.5, 0.5, 0.5, 0.0
alpha	dd 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255
rgba	db 10, 15, 20, 16, 25, 30, 35, 32, 40, 45, 50, 48, 55, 60, 65, 64
		db 70, 75, 80, 80, 85, 90, 95, 96, 100, 105, 110, 112, 115, 120, 125, 128
		db 130, 135, 140, 144, 0, 0, 0, 160, 160, 165, 170, 176, 175, 180, 185, 192
		db 190, 195, 200, 208, 205, 210, 215, 224, 220, 225, 230, 240, 235, 240, 245, 255
colors	dd 64 dup(0)
result	dd 128 dup(0)
result1	dd 128 dup(0)
_mm0	dq -1
_mm1	dq 0102030405060708h
_m1000	db 255, 0, 0, 0, 255, 0, 0, 0
_xmm0	dd 2.0, 30.0, 0.5, 0.25

.code

asm_ssh_shufb proc public USES rbx r15 r10 r11
		movups xmm0, _xmm0
		pextrw eax, xmm0, 0
		movd mm0, dword ptr _mm1
		pxor mm5, mm5
		punpcklbw mm0, mm5
		packuswb mm0, mm0
		call asm_clip_bar
		movsxd r10, dword ptr [rcx]			;pitch
		shl r10, 2
		movsxd rax, dword ptr [rdx]
		movsxd rcx, dword ptr [rdx + 4]
		imul rcx, r10
		lea rcx, [rcx + rax * 4]
		add r8, rcx
		movsxd rcx, dword ptr [rdx + 8]
		movsxd rdx, dword ptr [rdx + 12]
_loop:	push r8
		push rcx
@@:		or [r8], r9d
		add r8, 4
		loop @b
		pop rcx
		pop r8
		add r8, r10
		dec rdx
		jnz _loop
		ret
		mov rax, offset _1
		mov byte ptr [rax + 4], 1
_1:		vpshufd xmm0, xmm0, 0
		vpshufd xmm0, xmm0, 0
		vmovaps xmm10, grid
		vmovaps xmm11, half
		vmovaps ymm12, f_255x8
		mov rbx, 4
		mov r15, 16
		mov r8, offset result
		mov r9, offset rgba
		mov r10, offset colors
		mov r11, offset alpha
		;call asm_set_colors
		mov r9, offset result
		mov r8, offset result1
		;call asm_compress_colors
		ret
asm_ssh_shufb endp

end
