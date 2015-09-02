
grey	= 8
rgb		= 4
idx		= 2
rle		= 1

OPTION NOKEYWORD:<width struct type>
;OPTION PROLOGUE:NONE
;OPTION EPILOGUE:NONE
;OPTION PROLOGUE:PrologueDef
;OPTION EPILOGUE:EpilogueDef

sz_func macro addr
LOCAL @@label
@@label	dq addr - ($ + 8)
endm

.data?
ALIGN 16
pixGIF				dq ?
palGIF				dq ?
dxtColors			dd 64 dup(?)
dxtWeights			dd 16 dup(?)
dxtCodes			dd 12 dup(?)
dxtIndices5Alpha	db 16 dup(?)
dxtIndices7Alpha	db 16 dup(?)

.const
ALIGN 16
_or255x4	dd 0ff000000h,0ff000000h,0ff000000h,0ff000000h
_gamma		dd 0.3, 0.59, 0.11, 0.0
dxtGrid		dd 31.0, 63.0, 31.0, 0
dxtHalf		dd 0.5, 0.5, 0.5, 0
_fpDiv2_0	dd 2.0, 2.0, 2.0, 0
_fpDiv3_0	dd 3.0, 3.0, 3.0, 0
_255x4		dd 255.0, 255.0, 255.0, 256.0
_fp0001		dd 0.0, 0.0, 0.0, 1.0
dDxt		dq dxt1Alpha, 0, dxt3dAlpha, 8, dxt5dAlpha, 8
cDxt		dq dxt1Alpha, dxt3cAlpha, dxt5cAlpha
cnvFuncs	dq Null,	Null
			dq ARGB8to,	toARGB8
			dq ABGR8to,	toABGR8
			dq BGR8to,	toBGR8
			dq RGB8to,	toRGB8
			dq R5G6B5to,toR5G6B5
			dq A1RGB5to,toA1RGB5
			dq ARGB4to,	toARGB4
			dq A8to,	toA8
			dq L8to,	toL8
			dq FNTto,	toFNT
			dq BC1to,	toBC1
			dq BC2to,	toBC2
			dq BC3to,	toBC3
_maxFlt		dd 3.402823466e+38F

.data

.code

BC1to:	
		dq 8, -1
BC2to:
		dq 16, -1
BC3to:
		dq 16, -1
toBC1:
		dq 8, -1
toBC2:
		dq 16, -1
toBC3:
		dq 16, -1
Null:
		dq 0, 0
ARGB8to:
		dq 4
		sz_func toARGB8
		mov eax, [r9]
		mov [r8], eax
		add r8, 4
		add r9, 4
toARGB8:
		dq 4
		sz_func ABGR8to
		mov eax, [r9]
		mov [r8], eax
		add r8, 4
		add r9, 4
ABGR8to:
		dq 4
		sz_func toABGR8
		mov eax, [r9]
		bswap eax
		ror eax, 8
		mov [r8], eax
		add r8, 4
		add r9, 4
toABGR8:
		dq 4
		sz_func BGR8to
		mov eax, [r9]
		bswap eax
		ror eax, 8
		mov [r8], eax
		add r8, 4
		add r9, 4
BGR8to:
		dq 3
		sz_func toBGR8
		mov eax, [r9]
		or eax, 0ff000000h
		bswap eax
		ror eax, 8
		mov [r8], eax
		add r8, 4
		add r9, 3
toBGR8:
		dq 3
		sz_func RGB8to
		mov eax, [r9]
		bswap eax
		ror eax, 8
		mov [r8], ax
		shr eax, 16
		mov [r8 + 2], al
		add r8, 3
		add r9, 4
RGB8to:
		dq 3
		sz_func toRGB8
		mov eax, [r9]
		or eax, 0ff000000h
		mov [r8], eax
		add r8, 4
		add r9, 3
toRGB8:
		dq 3
		sz_func R5G6B5to
		mov eax, [r9]
		mov [r8], ax
		shr eax, 16
		mov [r8 + 2], al
		add r8, 3
		add r9, 4
R5G6B5to:
		dq 2
		sz_func toR5G6B5
		mov ax, [r9]
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000000011111b
		and ecx, 00000000000000000000011111100000b
		and edx, 00000000000000001111100000000000b
		shl eax, 3
		shl ecx, 5
		shl edx, 8
		or eax, ecx
		or eax, edx
		or eax, 0ff000000h
		mov [r8], eax
		add r8, 4
		add r9, 2
toR5G6B5:
		dq 2
		sz_func A1RGB5to
		mov eax, [r9]
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000011111000b
		and ecx, 00000000000000001111110000000000b
		and edx, 00000000111110000000000000000000b
		shr eax, 3
		shr ecx, 5
		shr edx, 8
		or eax, ecx
		or eax, edx
		mov [r8], ax
		add r8, 2
		add r9, 4
A1RGB5to:
		dq 2
		sz_func toA1RGB5
		mov ax, [r9]
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000000011111b
		and ecx, 00000000000000000000001111100000b
		and edx, 00000000000000000111110000000000b
		shl eax, 3
		shl ecx, 6
		shl edx, 9
		or eax, ecx
		or eax, edx
		test ebx, 00000000000000001000000000000000b
		jz @f
		or eax, 0ff000000h
@@:		mov [r8], eax
		add r8, 4
		add r9, 2
toA1RGB5:
		dq 2
		sz_func ARGB4to
		mov eax, [r9]
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000011111000b
		shr eax, 3
		test ecx, 0ff000000h
		jz @f
		or eax, 00000000000000010000000000000000b
@@:		and ecx, 00000000000000001111100000000000b
		and edx, 00000000111110000000000000000000b
		shr ecx, 6
		shr edx, 9
		or eax, ecx
		or eax, edx
		mov [r8], ax
		add r8, 2
		add r9, 4
ARGB4to:
		dq 2
		sz_func toARGB4
		mov ax, [r9]
		mov ecx, eax
		mov edx, eax
		mov ebx, eax
		and eax, 00000000000000000000000000001111b
		and ecx, 00000000000000000000000011110000b
		and edx, 00000000000000000000111100000000b
		and ebx, 00000000000000001111000000000000b
		shl eax, 4
		shl ecx, 8
		shl edx, 12
		shl ebx, 16
		or eax, ecx
		or eax, edx
		or eax, ebx
		mov [r8], eax
		add r8, 4
		add r9, 2
toARGB4:
		dq 2
		sz_func A8to
		mov eax, [r9]
		mov ecx, eax
		mov edx, eax
		mov ebx, eax
		and eax, 00000000000000000000000011110000b
		and ecx, 00000000000000001111000000000000b
		and edx, 00000000111100000000000000000000b
		and ebx, 11110000000000000000000000000000b
		shr eax, 4
		shr ecx, 8
		shr edx, 12
		shl ebx, 16
		or eax, ecx
		or eax, edx
		or eax, ebx
		mov [r8], ax
		add r8, 2
		add r9, 4
A8to:
		dq 1
		sz_func toA8
		movzx eax, byte ptr [r9]
		movd xmm15, eax
		shufps xmm15, xmm15, 0
		packssdw xmm15, xmm15
		packuswb xmm15, xmm15
		movd eax, xmm15
		mov [r8], eax
		add r8, 4
		add r9, 1
toA8:
		dq 1
		sz_func L8to
		mov eax, [r9]
		shr eax, 24
		mov [r8], al
		add r8, 1
		add r9, 4
L8to:
		dq 1
		sz_func toL8
		movzx eax, byte ptr [r9]
		movd xmm7, eax
		shufps xmm7, xmm7, 0
		packssdw xmm7, xmm7
		packuswb xmm7, xmm7
		movd eax, xmm7
		or eax, 0ff000000h
		mov [r8], eax
		add r8, 4
		add r9, 1
toL8:
		dq 1
		sz_func FNTto
		mov eax, [r9]
		and eax, 00ffffffh
		movzx ecx, al
		shr eax, 8
		movzx edx, al
		shr eax, 8
		add eax, ecx
		add eax, edx
		shr eax, 2
		mov [r8], al
		add r8, 1
		add r9, 4
FNTto:
		dq 4
		sz_func toFNT
		xorps xmm6, xmm6
		movd xmm7, dword ptr [r9]
		punpcklbw xmm7, xmm6
		punpcklwd xmm7, xmm6
		cvtdq2ps xmm7, xmm7
		dpps xmm7, xmm5, 01111111b
		cvtps2dq xmm7, xmm7
		packssdw xmm7, xmm7
		packuswb xmm7, xmm7
		movd eax, xmm7
		mov [r8], eax
		add r8, 4
		add r9, 4
toFNT:
		dq 4
		sz_func cnvFin
		mov eax, [r9]
		mov [r8], eax
		add r8, 4
		add r9, 4
cnvFin:	ret

;rcx(width), rdx(fmt)
asmPitch proc public
		cmp rdx, 11
		jb @f
		shr rcx, 2
@@:		mov rax, rcx
		mov rcx, offset cnvFuncs
		shl rdx, 4
		mov rcx, [rdx + rcx]
		imul rax, [rcx]
		ret
asmPitch endp

;rcx(width), rdx(height), r8(fmt) r9(flag)
asmTargetSize proc public
		; выравнять размеры на 4, если это BC1-BC3
		cmp r8, 11		;BC1
		jb @f
		shr rdx, 2
		shr rcx, 2
@@:		imul rcx, rdx					; количество пикселей по размеру изображения
		mov rdx, offset cnvFuncs
		shl r8, 4
		mov r8, [r8 + rdx]				; адрес функции
		mov rax, [r8]					; байт на пиксель
		imul rax, rcx					; количество байт на изображение
		xor rcx, rcx
		cmp rax, [r8]
		cmovb rax, [r8]
		cmovb rcx, rax
		test r9, r9
		jz _fin
		mov [r9], rcx
_fin:	ret
asmTargetSize endp

dxtUnpack565 proc USES rcx rdx
		mov ecx, eax
		mov edx, eax
		and eax, 00000000000000000000000000011111b
		and ecx, 00000000000000000000011111100000b
		and edx, 00000000000000001111100000000000b
		shl eax, 3
		shl ecx, 5
		shl edx, 8
		or eax, ecx
		or eax, edx
		ret
dxtUnpack565 endp

dxtPack565 proc USES rcx
		mulps xmm7, xmm10
		cvtps2dq xmm7, xmm7
		movd edx, xmm7
		psrldq xmm7, 4
		movd ecx, xmm7
		psrldq xmm7, 4
		movd eax, xmm7
		shl ecx, 5
		shl edx, 11
		or eax, ecx
		or eax, edx
		ret
dxtPack565 endp

asmMakeColorSet proc
		mov rdx, r9
		xor rax, rax
		mov rcx, 4
_loop:	mov rsi, rdx
_loop0:	movss xmm0, dword ptr [rdx]
		punpcklbw xmm0, xmm15
		punpcklwd xmm0, xmm15
		cvtdq2ps xmm0, xmm0
		pshufd xmm0, xmm0, 11000110b
		pshufd xmm1, xmm0, 11111111b
		addps xmm0, xmm13
		divps xmm0, xmm14
		movaps [r11 + rax * 4], xmm0				; colors
		cvtps2dq xmm1, xmm1
		movss dword ptr [r12 + rax], xmm1
		add rax, 4
		add rdx, 4
		test rax, 15
		jnz _loop0
		mov rdx, rsi
		add rdx, rbx
		loop _loop
		ret
asmMakeColorSet endp

asmComputePrinciple proc
		movaps xmm0, xmm6							; 6.0|6.1|6.2|6.3 - row0(matrix[0], matrix[1], matrix[2], matrix[3])
		pshufd xmm1, xmm7, 11010000b				; 7.0|7.0|7.1|7.3
		pshufd xmm2, xmm7, 11100100b				; 7.0|7.1|7.2|7.3
		psrldq xmm6, 4
		movss xmm1, xmm6							; 6.1|7.0|7.1|7.3 - row1(matrix[1], matrix[4], matrix[5], matrix[7]);
		psrldq xmm6, 4
		movss xmm2, xmm6							; 6.2|7.1|7.2|7.3 - row2(matrix[2], matrix[5], matrix[6], matrix[7]);
		movaps xmm3, xmm0							; row0
		movaps xmm4, xmm1							; row1
		movaps xmm5, xmm2							; row2
		dpps xmm3, xmm3, 01110001b					; r0
		dpps xmm4, xmm4, 01110001b					; r1
		dpps xmm5, xmm5, 01110001b					; r2
		comiss xmm3, xmm4							; r0 > r1
		jbe @f
		movaps xmm7, xmm0
		comiss xmm3, xmm5							; r0 > r2
		ja fin0
@@:		movaps xmm7, xmm1
		comiss xmm4, xmm5
		ja fin0
		movaps xmm7, xmm2
fin0:	mov rcx, 8
@@:		movaps xmm3, xmm7
		movaps xmm4, xmm7
		movaps xmm5, xmm7
		dpps xmm3, xmm0, 01110001b					; x
		dpps xmm4, xmm1, 01110001b					; y
		dpps xmm5, xmm2, 01110001b					; z
		movss xmm6, xmm3
		maxss xmm6, xmm4
		maxss xmm6, xmm5							; norm
		rcpss xmm6, xmm6							; iv = 1/norm
		pshufd xmm6, xmm6, 11000000b
		mulps xmm7, xmm6							; v *= iv
		loop @b
		ret
asmComputePrinciple endp

asmMakeCovariance proc
		xor rax, rax
		xorps xmm0, xmm0							; total
		xorps xmm2, xmm2							; centroid
@@:		movaps xmm1, [rax + r11]					; colors[i]
		pshufd xmm3, xmm1, 11111111b
		addps xmm0, xmm3							; total += weights[i]
		mulps xmm3, xmm1							; tmp = weights[i] * colors[i]
		addps xmm2, xmm3							; centroid += tmp
		add rax, 16
		cmp rax, 256
		jb @b
		rcpps xmm0, xmm0
		mulps xmm2, xmm0							; centroid /= total
		xor rax, rax
		xorps xmm7, xmm7							; covariance matrix
		xorps xmm6, xmm6
@@:		movaps xmm0, [rax + r11]					; colors[i]
		pshufd xmm1, xmm0, 11111111b
		subps xmm0, xmm2							; a = colors[i] - centroid
		mulps xmm1, xmm0							; b = weights[i] * a
		pshufd xmm3, xmm0, 11000000b				; a.x|a.x|a.x|a.w
		pshufd xmm4, xmm1, 11100100b				; b.x|b.y|b.z|b.w
		mulps xmm3, xmm4
		addps xmm6, xmm3
		pshufd xmm3, xmm0, 11100101b				; a.y|a.y|a.z|a.w
		pshufd xmm4, xmm1, 11101001b				; b.y|b.z|b.z|b.w
		mulps xmm3, xmm4
		addps xmm7, xmm3
		add rax, 16
		cmp rax, 256
		jb @b
		ret											;covariance = xmm6|xmm7
asmMakeCovariance endp

dxtColorCompress proc USES rbx
LOCAL @@closest[16]:BYTE
LOCAL @@remmaped[16]:BYTE
		lea rsi, @@closest
		mov rdi, offset dxtCodes
		movaps [rdi], xmm5							; codes[0] = m_start;
		movaps [rdi + 16], xmm6						; codes[1] = m_end;
		movaps xmm0, xmm5
		movaps xmm1, xmm6
		mulps xmm0, xmm11
		mulps xmm1, xmm11
		addps xmm0, xmm1
		movaps [rdi + 32], xmm0						; codes[2] = 0.5f*m_start + 0.5f*m_end;
		xor rax, rax								; i = 0
_l0:	movss xmm0, _maxFlt							; dist = FLT_MAX
		xor rdx, rdx								; idx = 0
		xor rcx, rcx								; j = 0
		movaps xmm1, [r11 + rax]					; colors[i]
_l1:	movaps xmm2, xmm1
		subps xmm2, [rdi + rcx]						; tmp = colors[i] - codes[j]
		dpps xmm2, xmm2, 01110001b
		ucomiss xmm2, xmm0							; d < dist
		jae @f
		movss xmm0, xmm2							; dist = d
		mov rdx, rcx
@@:		add rcx, 16
		cmp rcx, 48
		jb _l1
		shr rdx, 4
		mov [rsi], dl
		inc rsi
		add rax, 16
		cmp rax, 256
		jb _l0 
		movaps xmm7, xmm5
		call dxtPack565
		mov cx, ax
		movaps xmm7, xmm6
		call dxtPack565
		lea rsi, @@closest
		lea rdi, @@remmaped
		movsq
		movsq
		sub rdi, 16
		mov [r8], cx
		mov [r8 + 2], ax
		add r8, 4
		mov rcx, 4
@@:		mov ax, [rdi + 0]
		mov dx, [rdi + 2]
		add rdi, 4
		shl ah, 2
		shl dl, 4
		shl dh, 6
		or al, ah
		or dl, dh
		or al, dl
		mov [r8], al
		inc r8
		loop @b
		ret
dxtColorCompress endp

dxt3cAlpha proc
		xor rcx, rcx
@@:		mov eax, [r12 + rcx]
		mov edx, [r12 + rcx + 4]
		and eax, 15
		and edx, 15
		shl edx, 4
		or eax, edx
		mov [r8], al
		inc r8
		add rcx, 8
		cmp rcx, 64
		jb @b
		ret
dxt3cAlpha endp

; edx = max ebx = min ecx = it
asmFixRange proc
		mov eax, edx
		sub eax, ebx			; tmp1 = max - min
		cmp eax, ecx			; tmp1 < it
		jge @f
		lea eax, [ebx + ecx]	; min + it
		mov edx, 255
		cmp eax, edx			; 
		cmovl edx, eax
@@:		mov eax, edx
		sub eax, ebx
		cmp eax, ecx
		jge @f
		mov eax, edx
		sub eax, ecx
		xor ebx, ebx
		cmp eax, ebx
		cmovg ebx, eax
@@:		ret
asmFixRange endp

;rsi = codes rdi = indices
asmFitCodes proc USES rbx rdx
		xor r11, r11				; err = 0
		xor rcx, rcx				; i = 0
_loop:	mov eax, [r12 + rcx * 4]
		push rcx
		push rdi
		xor rbx, rbx				; index = 0
		mov rdi, 2147483647			; least = INT_MAX
		xor rdx, rdx				; j = 0
@@:		mov r10, rax				; value
		movzx rcx, byte ptr [rsi + rdx]; codes[j]
		sub rax, rcx				; dist = value - codes[j]
		imul rax, rax				; dist *= dist
		cmp rax, rdi				; dist < least
		cmovl rdi, rax
		cmovl rbx, rdx
		mov rax, r10
		inc rdx
		cmp rdx, 8
		jb @b
		add r11, rdi				; err += least
		pop rdi
		pop rcx
		mov [rdi + rcx], bl			; indices[i] = index
		inc rcx
		cmp rcx, 16
		jb _loop
		mov rax, r11				; err
		ret
asmFitCodes endp

dxt5cAlpha7 proc USES rcx r8 r9 r10 r11
LOCAL @@codes7[8]:BYTE
		xor rdx, rdx				; max7
		mov rbx, 255				; min7
		xor rcx, rcx				; i = 0
@@:		mov eax, dword ptr [rcx * 4 + r12]	; val
		cmp eax, ebx				; val < min7
		cmovb ebx, eax
		cmp eax, edx				; val > max7
		cmova edx, eax
		inc rcx
		cmp rcx, 16
		jb @b
		cmp ebx, edx				; min7 > max7
		cmova ebx, edx
		mov rcx, 7
		call asmFixRange
		lea rsi, @@codes7
		mov [rsi + 0], bl			; codes7[0] = min7
		mov [rsi + 1], dl			; codes7[1] = max7
		mov rcx, 1
		mov r9, 7
		mov r10, rdx
@@:		mov rax, rcx
		imul rax, r10				; tmp1 = i * max7
		mov r8, r9
		sub r8, rcx
		imul r8, rbx				; tmp2 = (7 - i) * min7
		add rax, r8					; tmp1 += tmp2
		xor rdx, rdx				
		div r9						; tmp1 /= 7
		mov [rsi + rcx + 1], al		; codes7[1 + i] = tmp1
		inc rcx
		cmp rcx, 7
		jb @b
		mov rdx, r10
		mov rdi, offset dxtIndices7Alpha
		call asmFitCodes
		ret
dxt5cAlpha7 endp

dxt5cAlpha5 proc USES rcx r8 r9 r10 r11
LOCAL @@codes5[8]:BYTE
		xor rdx, rdx				; max5
		mov rbx, 255				; min5
		xor rcx, rcx				; i = 0
_loop0:	mov eax, dword ptr [rcx * 4 + r12]	; val
		test eax, eax
		jz @f
		cmp eax, ebx				; val < min5
		cmovb ebx, eax
@@:		cmp rax, 255
		jz @f
		cmp eax, edx				; val > max5
		cmova edx, eax
@@:		inc rcx
		cmp rcx, 16
		jb _loop0
		cmp ebx, edx				; min5 > max5
		cmova ebx, edx
		mov rcx, 5
		call asmFixRange
		lea rsi, @@codes5
		mov [rsi + 0], bl			; codes5[0] = min5
		mov [rsi + 1], dl			; codes5[1] = max5
		mov byte ptr [rsi + 6], 0	; codes5[6] = 0
		mov byte ptr [rsi + 7], 255	; codes5[7] = 255
		mov rcx, 1
		mov r9, 5
		mov r10, rdx
@@:		mov rax, rcx
		imul rax, r10				; tmp1 = i * max5
		mov r8, r9
		sub r8, rcx
		imul r8, rbx				; tmp2 = (5 - i) * min5
		add rax, r8					; tmp1 += tmp2
		xor rdx, rdx				
		div r9						; tmp1 /= 5
		mov [rsi + rcx + 1], al		; codes5[1 + i] = tmp1
		inc rcx
		cmp rcx, 5
		jb @b
		mov rdx, r10
		mov rdi, offset dxtIndices5Alpha
		call asmFitCodes
		ret
dxt5cAlpha5 endp

dxt5cAlpha proc USES rbx r10 r11
LOCAL @@swapped[16]:BYTE
		call dxt5cAlpha5
		mov rcx, rax			; err5
		mov r10, rdx			; max5
		mov r11, rbx			; min5
		call dxt5cAlpha7
		cmp rcx, rax
		ja _7
		;WriteAlphaBlock5(min5, max5, indices5, block);
		mov rdi, offset dxtIndices5Alpha
		mov rdx, r10
		mov rbx, r11
		cmp rbx, rdx
		jbe _next
		xchg rbx, rdx
		lea rsi, @@swapped
		xor rcx, rcx
_loop0:	mov al, [rdi + rcx]
		xor ah, al
		cmp al, 2
		jb @f
		mov ah, al
		cmp al, 5
		ja @f
		mov ah, 7
		sub ah, al
@@:		mov [rsi + rcx], ah
		inc rcx
		cmp rcx, 16
		jb _loop0
		mov rdi, rsi
		jmp _next
_7:		;WriteAlphaBlock7(min7, max7, indices7, block);
		mov rdi, offset dxtIndices7Alpha
		cmp rbx, rdx
		jae _next
		xchg rbx, rdx
		lea rsi, @@swapped
		xor rcx, rcx
_loop1:	mov al, [rdi + rcx]
		xor ah, al
		cmp al, 2
		jb @f
		mov ah, 9
		sub ah, al
@@:		mov [rsi + rcx], ah
		inc rcx
		cmp rcx, 16
		jb _loop1
		mov rdi, rsi
_next:	mov [r8], bl
		mov [r8 + 1], dl
		add r8, 2
		xor rdx, rdx			; i = 0
_loop:	xor rax, rax			; value = 0
		mov rcx, 8
@@:		shl rax, 3
		mov bl, [rdi + rcx - 1]
		or al, bl
		loop @b
		add rdi, 8
		mov [r8], ax
		shr eax, 16
		mov [r8 + 2], al
		add r8, 3
		inc rdx
		cmp rdx, 2
		jb _loop
		ret
dxt5cAlpha endp

dxt1Alpha proc
		ret
dxt1Alpha endp

dxt3dAlpha proc USES rcx r12
		mov rsi, r8
		xor rdi, rdi
		mov r8, 4
@@:		mov edi, [r12 + 9]
		mov ax, [r9]
		mov cx, ax
		and ax, 0000111100001111b
		and cx, 1111000011110000b
		mov dx, ax
		mov bx, cx
		shl ax, 4
		shr cx, 4
		or ax, dx
		or cx, bx
		mov [rsi + rdi + 3], al
		mov [rsi + rdi + 7], cl
		mov [rsi + rdi + 11], ah
		mov [rsi + rdi + 15], ch
		add r9, 2
		add r12, 13
		dec r8
		test r8, r8
		jnz @b
		mov r8, rsi
		ret
dxt3dAlpha endp

dxt5dAlpha proc USES rcx r10 r11 r12
LOCAL @@codes[8]:BYTE
LOCAL @@indices[16]:BYTE
		lea rsi, @@codes
		mov ax, [r9]
		add r9, 2
		mov [rsi], ax
		mov word ptr [rsi + 6], 0ff00h
		movzx rdi, al
		mov al, ah
		movzx rbx, al
		mov rcx, 1
		cmp rdi, rbx						; alpha1 <= alpha2
		mov r10, 7
		ja @f
		mov r10, 5
@@:		mov rax, rdi
		mov r11, r10
		sub r11, rcx						; tmp1 = 5(7) - i
		imul r11, rax						; tmp1 *= alpha0
		mov rax, rcx
		imul rax, rbx						; tmp2 = i * alpha1
		add rax, r11						; tmp2 += tmp1
		xor rdx, rdx
		div r10								; tmp2 /= 5(7)
		mov [rsi + rcx + 1], al				; codes[i + 1] = tmp2
		inc rcx
		cmp rcx, r10
		jb @b
		lea rdi, @@indices
		xor rcx, rcx						; i = 0
		mov r10, rdi
_loop:	mov eax, [r9]
		and eax, 0ffffffh					; value
		add r9, 3
		xor rdx, rdx						; j = 0
@@:		mov bl, al
		and bl, 7
		shr rax, 3
		mov [r10 + rdx], bl
		inc rdx
		cmp rdx, 8
		jb @b
		add r10, 8
		inc rcx
		cmp rcx, 2
		jb _loop
		xor rcx, rcx
_loop1:	mov edx, [r12 + 9]
		add rdx, 3
@@:		movzx rax, byte ptr [rdi + rcx]		; index
		mov al, [rsi + rax]					; alpha
		mov byte ptr [r8 + rdx], al
		add rdx, 4
		inc rcx
		test rcx, 3
		jnz @b
		add r12, 13
		cmp rcx, 16
		jb _loop1
		ret
dxt5dAlpha endp

;rcx(width), rdx(height), r8(dst), r9(src), fmt, is
asmDecode proc public USES rdi rsi rbx r12 w:QWORD, h:QWORD, dst:QWORD, src:QWORD, fmt:QWORD, is:QWORD
		movaps xmm5, _gamma
		mov w, rcx
		mov h, rdx
		mov rdi, offset _loop
		mov rsi, offset cnvFuncs
		mov rax, is
		shl rax, 3
		add rsi, rax
		mov rax, fmt
		shl rax, 4
		mov rsi, [rax + rsi]
		mov r10, [rsi + 8]
		test r10, r10
		js _dxt
		imul rdx, rcx
		mov rcx, r10
		add rsi, 16
		mov rax, rdi
		rep movsb
		mov r10, rdx
		mov rsi, offset _loop_
		movsd
		movsd
		movsd
		sub rdi, 7
		sub rax, rdi
		mov byte ptr [rdi - 1], al
_loop:
		db 80 dup(90h)

_loop_:	dec r10
		jnz _loop
_fin:	ret
_dxt:	shr rdx, 2
		jz _fin
		lea rbx, [rcx * 4]
		shr rcx, 2
		jle _fin
		xorps xmm15, xmm15
		mov rax, fmt
		cmp is, 1
		jz _dxtC
		; декомпрессор
		movaps xmm14, _fpDiv2_0
		movaps xmm13, _fpDiv3_0
		movaps xmm12, _or255x4
		mov r10, offset dDxt - 11 * 16
		shl rax, 4
		mov r11, [rax + r10 + 8]		; смещение к индексам
		mov r10, [rax + r10]			; адрес функции распаковки альфа канала
		mov r12, offset _shufd1
		mov dword ptr [_shufd1 + 9], 0
		mov rax, rbx
		mov dword ptr [_shufd2 + 9], ebx
		add rbx, rbx
		mov dword ptr [_shufd3 + 9], ebx
		add rbx, rax
		mov dword ptr [_shufd4 + 9], ebx
_height:push rcx
		push rbx
		push rdx
_width:	mov eax, [r9 + r11 + 4]			; взять индексы цветов
		mov byte ptr [_shufd1 + 4], al
		mov byte ptr [_shufd2 + 4], ah
		shr eax, 16
		mov byte ptr [_shufd3 + 4], al
		mov byte ptr [_shufd4 + 4], ah
		mov ax, [r9 + r11]				; первый цвет
		push rax
		call dxtUnpack565				; распаковываем
		movd xmm0, eax
		movss xmm2, xmm0
		punpcklbw xmm2, xmm15
		punpcklwd xmm2, xmm15
		cvtdq2ps xmm2, xmm2
		mov ax, [r9 + r11 + 2]
		push rax
		call dxtUnpack565
		movd xmm1, eax
		movss xmm3, xmm1
		punpcklbw xmm3, xmm15
		punpcklwd xmm3, xmm15
		cvtdq2ps xmm3, xmm3
		pop rax
		pop rbx
		test r11, r11
		jz _ndxt1
		cmp bx, ax
		jb _ndxt1
		addps xmm2, xmm3
		divps xmm2, xmm14		; /= 2.0f
		xorps xmm3, xmm3
		jmp next
_ndxt1:	movaps xmm4, xmm2

		mulps xmm2, xmm14
		addps xmm2, xmm3

		mulps xmm3, xmm14
		addps xmm3, xmm4

		divps xmm2, xmm13
		divps xmm3, xmm13
next:	cvtps2dq xmm2, xmm2
		packssdw xmm2, xmm2
		packuswb xmm2, xmm2
		
		cvtps2dq xmm3, xmm3
		packssdw xmm3, xmm3
		packuswb xmm3, xmm3
		
		unpcklps xmm0, xmm1
		unpcklps xmm2, xmm3
		movlhps xmm0, xmm2
		orps xmm0, xmm12

_shufd1:pshufd xmm1, xmm0, 0
		movaps [r8 + 10203040h] ,xmm1
_shufd2:pshufd xmm2, xmm0, 0
		movaps [r8 + 10203040h] ,xmm2
_shufd3:pshufd xmm3, xmm0, 0
		movaps [r8 + 10203040h] ,xmm3
_shufd4:pshufd xmm4, xmm0, 0
		movaps [r8 + 10203040h] ,xmm4
		; распаковываем альфа канал
		call r10
		; следующий блок
		add r9, 8
		add r8, 16
		dec rcx
		jnz _width
		pop rdx
		pop rbx
		pop rcx
		add r8, rbx
		dec rdx
		jnz _height
		ret
		; компрессор
_dxtC:	movaps xmm10, dxtGrid
		movaps xmm11, dxtHalf
		movaps xmm13, _fp0001
		movaps xmm14, _255x4
		mov r10, offset cDxt - 11 * 8
		mov r10, [rax * 8 + r10]					; адрес функции запаковки альфа канала
		mov r11, offset dxtColors
		mov r12, offset dxtWeights
height:	push rcx
		push rdx
		push r9
width:	push rcx
		; создаем объект colorSet
		call asmMakeColorSet
		; пакуем альфа канал
		call r10
		; создаем объект rangeFit
		call asmMakeCovariance
		call asmComputePrinciple
		; найти первый и последний цвет из диапазона
		movaps xmm5, [r11]				; start
		movaps xmm6, xmm5				; end
		movaps xmm0, xmm5
		dpps xmm0, xmm7, 01110001b		; min
		movss xmm1, xmm0				; max
		mov rax, 16
_l0:	movaps xmm2, [rax + r11]		; colors[i]
		movaps xmm3, xmm2
		dpps xmm3, xmm7, 01110001b		; val
		ucomiss xmm3, xmm0
		jae @f
		movaps xmm5, xmm2
		movss xmm0, xmm3
		jmp _l1
@@:		ucomiss xmm3, xmm1
		jbe _l1
		movaps xmm6, xmm2
		movss xmm1, xmm3
_l1:	add rax, 16
		cmp rax, 256
		jb _l0
		; пакуем цвет
		call dxtColorCompress
		add r9, 16
		pop rcx
		loop width
		pop r9
		pop rdx
		pop rcx
		lea r9, [r9 + rbx * 4]
		dec rdx
		jnz height
		ret
asmDecode endp

;rcx(width) rdx(height) r8(dst) r9(src)
asmCnv proc public USES rbx rdi rsi r12 r13 r14 r15 w:QWORD, h:QWORD, dst:QWORD, src:QWORD, palette:QWORD, flags:QWORD, bpp:QWORD, bmp:QWORD, fmt:QWORD
		mov rbx, rcx
		mov rdi, offset inMem
		mov rsi, offset cnvFuncs
		mov rax, fmt
		shl rax, 4
		mov rsi, [rax + rsi]
		mov rcx, [rsi + 8]
		sub rcx, 11					; вычесть eax add r8, xxx add r9, xxx
		sub rax, rcx
		add rsi, 16
		rep movsb
		mov byte ptr [rdi], 0c3h
		mov r10, palette
		mov r11, flags
		mov r12, bpp
		mov r14, bmp
		xor r13, r13
		mov r15, 3
		test r14, r14
		jz _height
		inc r15
		push rdx
		mov r13, 4					; вычислить кратность
		xor rdx, rdx
		mov rax, rbx
		div r13
		mov r13, rdx
		pop rdx
_height:push rbx
		push rdx
_width:	xor rcx, rcx
		test r11, rle
		jz @f
		mov cl, [r9]
		test cl, 128
		setnz ch
		shl ch, 7
		or ch, 64
		and cl, 127
		inc r9
@@:		inc cl
		movzx rax, cl
		sub rbx, rax
looping:test r11, idx
		jz @f
		movzx rax, byte ptr [r9]	; индекс в палитре
		imul rax, r15
		mov eax, [rax + r10]		; взять цвет из палитры
		jmp _next
@@:		push rcx
		push rbx
		call inMem
		pop rbx
		pop rcx
		test r14, r14
		jz @f
_next:	or eax, 0ff000000h
@@:		mov [r8], eax
		add r8, 4
		test ch, 128
		jnz @f
		add r9, r12
@@:		dec cl
		jnz looping
		cmp ch, 192
		jnz @f
		add r9, r12
@@:		test rbx, rbx
		jg _width
		add r9, r13
		pop rdx
		pop rbx
		dec rdx
		jnz _height
		ret
inMem:	
		db 64 dup(90h)
asmCnv endp

unpackGIF proc private USES rax rcx rdi
		xor rcx, rcx
		xor rdx, rdx
@@:		mov dl, byte ptr [rax + r15]
		mov [r11 + rcx], dl
		inc rcx
		mov ax, word ptr [rax * 2 + r14]
		cmp rax, 0fffeh
		jb @b
		mov rdi, pixGIF
		mov rsi, palGIF
@@:		movzx rax, byte ptr [r11 + rcx - 1]
		cmp rax, r12
		lea rax, [rax * 2 + rax]
		mov eax, [rax + rsi]
		setz dl
		dec dl
		bswap eax
		shr eax, 8
		shl edx, 24
		or eax, edx
		stosd
		loop @b
		mov pixGIF, rdi
		ret
unpackGIF endp

;rcx(dst) rdx(pal) r8(stk) r9(lzw)
asmDecodeGIF proc public USES rbx rdi rsi r12 r13 r14 r15 dst:QWORD, palette:QWORD, stk:QWORD, lzw:QWORD, iTrans:QWORD
LOCAL @@temp[1024]:BYTE	
LOCAL @@szDictGIF[1024]:QWORD
LOCAL @@dictGIF[512]:QWORD
		lea r11, @@temp
		lea r14, @@szDictGIF
		lea r15, @@dictGIF
		mov r12, iTrans
		mov pixGIF, rcx
		mov palGIF, rdx
		mov r10, [r8]				; nMask
		mov rcx, [r8 + 15]			; nShift
begin:	mov eax, [r9]
		shr rax, cl
		and rax, r10
		add cl, ch
		movzx rdx, cl
		shr rdx, 3
		add r9, rdx
		and cl, 7
		cmp rax, [r8 + 32]
		jz finish
		cmp rax, [r8 + 24]
		jnz noCC
		xor rdx, rdx
@@:		mov byte ptr [rdx + r15], dl
		mov word ptr [rdx * 2 + r14], -2
		inc rdx
		cmp rdx, [r8 + 32]
		jle @b
		mov rdi, rdx				; pos
		mov r10, [r8 + 0]			; nMask
		mov r13, [r8 + 8]			; maxPos
		mov rax, [r8 + 16]			; nShift
		mov ch, al
		mov eax, [r9]
		shr eax, cl
		and rax, r10
		add cl, ch
		movzx rdx, cl
		shr rdx, 3
		add r9, rdx
		and cl, 7
		mov rbx, rax
		call unpackGIF
		jmp begin
noCC:	mov word ptr [rdi * 2 + r14], bx
		cmp word ptr [rax * 2 + r14], -1
		jz no0
		mov rbx, rax
@@:		mov dl, byte ptr [rbx + r15]
		movzx rbx, word ptr [rbx * 2 + r14]
		cmp rbx, 0fffeh
		jb @b
		mov byte ptr [rdi + r15], dl
		call unpackGIF
		jmp @f
no0:	mov dl, byte ptr [rbx + r15]
		mov byte ptr [rdi + r15], dl
		mov rbx, rax
		mov rax, rdi
		call unpackGIF
		mov rax, rbx
@@:		inc rdi
		mov rbx, rax
		cmp rdi, r13
		jb begin
		cmp rdi, 4096
		jae begin
		shl r13, 1
		inc ch
		shl r10, 1
		or r10, 3
		jmp begin
finish:	ret
asmDecodeGIF endp

asmDecodeBFS proc public
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
		xorps xmm6, xmm6
		movd xmm7, ecx
		punpcklbw xmm7, xmm6
		punpcklwd xmm7, xmm6
		cvtdq2ps xmm7, xmm7
		dpps xmm7, xmm5, 01111000b
		cvtps2dq xmm7, xmm7
		packssdw xmm7, xmm7
		packuswb xmm7, xmm7
		movd eax, xmm7
		and rax,0ff000000h
		and rcx, 00ffffffh
		or rax, rcx
_fin:	ret
asmDecodeBFS endp

end
