
extern asm_compress_colors:near
extern asm_set_colors:near

.data
align 16
grid		dd 31.0, 63.0, 31.0, 0.0
half		dd 0.5, 0.5, 0.5, 0.0
f_255x3_256 dd 255.0, 255.0, 255.0, 256.0
alpha	dd 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255
rgba	db 10, 15, 20, 16, 25, 30, 35, 32, 40, 45, 50, 48, 55, 60, 65, 64
		db 70, 75, 80, 80, 85, 90, 95, 96, 100, 105, 110, 112, 115, 120, 125, 128
		db 130, 135, 140, 144, 0, 0, 0, 160, 160, 165, 170, 176, 175, 180, 185, 192
		db 190, 195, 200, 208, 205, 210, 215, 224, 220, 225, 230, 240, 235, 240, 245, 255
colors	dd 64 dup(0)
result	dd 128 dup(0)
.code

asm_ssh_shufb proc public USES rbx r15 r10 r11
		vmovaps xmm10, grid
		vmovaps xmm11, half
		vmovaps xmm12,f_255x3_256
		;vrcpps xmm12, f_255x3_256
		mov rbx, 3
		mov r15, 16
		mov r8, offset result
		mov r9, offset rgba
		mov r10, offset colors
		mov r11, offset alpha
		call asm_set_colors
		call asm_compress_colors
		ret
asm_ssh_shufb endp

end
