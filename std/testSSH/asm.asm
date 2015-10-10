
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
tmp_mtx	dd 10.0, 1.0, 5.0, 6.0, 11.0, 25.0, 40.0, 60.0, 0.0, 4.0, 7.0, 8.0, 3.0, 0.0, 1.0, 9.0
flt_max dd 3.402823466e+38F
_1		dd 0.5,0.5,0.5,0.5
_2		dd 0.2,0.2,0.2,0.2
f_2_0	dd 2.0,2.0,2.0,2.0
f_1_0	dd 1.0,1.0,1.0,1.0
_255	dd 255.0, 255.0, 255.0, 255.0
.code
externdef powf:near
asm_ssh_shufb proc public
		ret
		vmovups xmm0, _1		;0.5
		vmovups xmm12, _2	;0.2
		vmovups xmm10, f_1_0
		vmovups xmm9, f_2_0

		vmovaps xmm1, xmm0
		vmovaps xmm2, xmm12
		;vshufps xmm2, xmm2, xmm2, 0
		vmovaps xmm0, xmm10
		mov rcx, 12
@@:		vsqrtps xmm1, xmm1
		vroundps xmm3, xmm2, 11b
		vsubps xmm2, xmm2, xmm3
		vmulps xmm2, xmm2, xmm9
		vcmpps xmm3, xmm2, xmm10, 5
		vandps xmm4, xmm1, xmm3
		vandnps xmm3, xmm3, xmm10
		vorps xmm3, xmm3, xmm4
		vmulps xmm0, xmm0, xmm3
		loop @b
		movups xmm1, _255
		mulps xmm1, xmm0
		cvtps2dq xmm1, xmm1

		;movups _1, xmm0

		vcvtps2pd ymm0, xmmword ptr _1
		vcvtps2pd ymm12, xmmword ptr _2
		vcvtps2pd ymm10, xmmword ptr f_1_0
		vcvtps2pd ymm9, xmmword ptr f_2_0

		vmovaps ymm1, ymm0
		vmovaps ymm2, ymm12
		;vshufps ymm2, ymm2, ymm2, 0
		vmovaps ymm0, ymm10
		mov rcx, 20
@@:		vsqrtpd ymm1, ymm1
		vroundpd ymm3, ymm2, 11b
		vsubpd ymm2, ymm2, ymm3
		vmulpd ymm2, ymm2, ymm9
		vcmppd ymm3, ymm2, ymm10, 5
		vandpd ymm4, ymm1, ymm3
		vandnpd ymm3, ymm3, ymm10
		vorpd ymm3, ymm3, ymm4
		vmulpd ymm0, ymm0, ymm3
		loop @b
		vcvtpd2ps xmm0, ymm0
		vmovups _1, xmm0
		xorps xmm1, xmm1


		vzeroall

		sqrtss xmm1, xmm1
		roundss xmm3, xmm2, 11b
		subss xmm2, xmm3
		mulss xmm2, f_2_0
		cmpss xmm2, xmm10, 5

		vmovaps xmm1, xmm0
		vmovaps xmm2, xmm11				; 1/gamma|1/gamma|1/gamma|1/gamma
		vmovaps xmm0, xmm10
		vsqrtps xmm1, xmm1
		vroundps xmm3, xmm2, 11b
		vsubps xmm2, xmm2, xmm3
		vmulps xmm2, xmm2, xmm9
		vcmpps xmm3, xmm2, xmm10, 5
		vandps xmm4, xmm1, xmm3
		vandnps xmm3, xmm3, xmm10
		vorps xmm0, xmm3, xmm4
		vmulps xmm0, xmm0, xmm3		
		movss xmm0, _1 + 0
		roundss xmm0, xmm0, 11b
		ret
		movss xmm1, _1 + 4
		sub rsp, 8
;		call powf
		add rsp, 8
		mov rdi, offset tmp_mtx
		mov rsi, 4
		imul rsi, rsi
		lea r12, [rdi + rsi * 4]
		mov rbx, rsi
		shr rbx, 1
		lea rbx, [rdi + rbx * 4]
median0:vmovss xmm0, flt_max
		mov rax, rdi
		mov r11, rdi
@@:		cmp r11, r12
		jae @f
		vmovss xmm1, dword ptr [r11]
		add r11, 4
		vucomiss xmm0, xmm1
		jbe @b
		vmovss xmm0, xmm0, xmm1
		lea rax, [r11 - 4]
		jmp @b
@@:		vmovss xmm0, dword ptr [rax]
		vmovss xmm1, dword ptr [rdi]
		vmovss dword ptr [rax], xmm1
		vmovss dword ptr [rdi], xmm0
		add rdi, 4
		cmp rdi, r12
		jb median0
		mov eax, dword ptr [rbx]
		ret
		ret
asm_ssh_shufb endp

end
