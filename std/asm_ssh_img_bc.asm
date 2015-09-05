
OPTION NOKEYWORD:<width type>

.const

_or255x4	dd 0ff000000h,0ff000000h,0ff000000h,0ff000000h
dxtGrid		dd 31.0, 63.0, 31.0, 0
dxtHalf		dd 0.5, 0.5, 0.5, 0
_fpDiv2_0	dd 2.0, 2.0, 2.0, 0
_fpDiv3_0	dd 3.0, 3.0, 3.0, 0
_255x4		dd 255.0, 255.0, 255.0, 256.0
_fp0001		dd 0.0, 0.0, 0.0, 1.0
_maxFlt		dd 3.402823466e+38F
dDxt		dq dxt1Alpha, 0, dxt3dAlpha, 8, dxt5dAlpha, 8
cDxt		dq dxt1Alpha, dxt3cAlpha, dxt5cAlpha
tmp			dq 8, -1, 16, -1, 16, -1, 0, 0, 0, 0

.data?

dxtColors			dd 64 dup(?)
dxtWeights			dd 16 dup(?)
dxtCodes			dd 12 dup(?)
dxtIndices5Alpha	db 16 dup(?)
dxtIndices7Alpha	db 16 dup(?)

.data

.code

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

dxtPack565 proc
		mulps xmm7, xmm10
		cvtps2dq xmm7, xmm7
		pextrd edx, xmm7, 0
		pextrd r15d, xmm7, 1
		pextrd eax, xmm7, 2
		shl ecx, 5
		shl edx, 11
		or eax, r15d
		or eax, edx
		ret
dxtPack565 endp

asmMakeColorSet proc
		mov rdx, r9
		xor rax, rax
		mov rcx, 4
_loop:	mov rsi, rdx
_loop0:	pmovzxbd xmm0, dword ptr [rdx]
		cvtdq2ps xmm0, xmm0
		pshufd xmm0, xmm0, 11000110b
		pshufd xmm1, xmm0, 11111111b
		addps xmm0, xmm13
		divps xmm0, xmm14
		movaps [r11 + rax * 4], xmm0				; colors
		cvtps2dq xmm1, xmm1
		movss dword ptr [r14 + rax], xmm1
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

dxtColorCompress proc
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
@@:		mov eax, [r14 + rcx]
		mov edx, [r14 + rcx + 4]
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
_loop:	mov eax, [r14 + rcx * 4]
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
@@:		mov eax, dword ptr [rcx * 4 + r14]	; val
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
_loop0:	mov eax, dword ptr [rcx * 4 + r14]	; val
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

dxt3dAlpha proc USES rcx
		mov rsi, r8
		xor rdi, rdi
		mov rcx, 4
@@:		mov ax, [r9]
		mov dx, ax
		and ax, 0000111100001111b
		and dx, 1111000011110000b
		mov r14w, ax
		mov r15w, dx
		shl ax, 4
		shr dx, 4
		or ax, r14w
		or dx, r15w
		mov [rsi + rdi + 3], al
		mov [rsi + rdi + 7], dl
		mov [rsi + rdi + 11], ah
		mov [rsi + rdi + 15], dh
		add r9, 2
		add rdi, r12
		loop @b
		ret
dxt3dAlpha endp

dxt5dAlpha proc USES rcx
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
		mov r15, 5
		cmp rdi, rbx						; alpha1 <= alpha2
		mov r14, 7
		cmovbe r14, r15
@@:		mov rax, rdi
		mov r15, r14
		sub r15, rcx						; tmp1 = 5(7) - i
		imul r15, rax						; tmp1 *= alpha0
		mov rax, rcx
		imul rax, rbx						; tmp2 = i * alpha1
		add rax, r15						; tmp2 += tmp1
		xor rdx, rdx
		div r14								; tmp2 /= 5(7)
		mov [rsi + rcx + 1], al				; codes[i + 1] = tmp2
		inc rcx
		cmp rcx, r14
		jb @b
		lea rdi, @@indices
		xor rcx, rcx						; i = 0
		mov r14, rdi
_loop:	mov eax, [r9]
		and eax, 0ffffffh					; value
		add r9, 3
		xor rdx, rdx						; j = 0
@@:		mov bl, al
		and bl, 7
		shr rax, 3
		mov [r14 + rdx], bl
		inc rdx
		cmp rdx, 8
		jb @b
		add r14, 8
		inc rcx
		cmp rcx, 2
		jb _loop
		xor rcx, rcx
		xor rdx, rdx
@@:		movzx rax, byte ptr [rdi + rcx]		; index
		mov al, [rsi + rax]					; alpha
		mov byte ptr [r8 + rdx + 3], al
		add rdx, 4
		inc rcx
		test rcx, 3
		jnz @b
		sub rdx, 12
		add rdx, r12
		cmp rcx, 16
		jb @b
		ret
dxt5dAlpha endp

asm_ssh_bc_x proc public
		movsxd rdx, dword ptr [rdx + 4]
		shr rdx, 2
		jz _fin
		lea r12, [rcx * 4]
		shr rcx, 2
		jle _fin
		test r11, r11					; проверка на компрессию
		jnz _dxtC
		; декомпрессор
		movaps xmm14, _fpDiv2_0
		movaps xmm13, _fpDiv3_0
		movaps xmm12, _or255x4
		mov r10, offset dDxt
		shl rax, 4
		mov r11, [rax + r10 + 8]		; смещение к индексам
		mov r10, [rax + r10]			; адрес функции распаковки альфа канала
		mov rax, r12
		lea r13, [r12 * 2 + rax]
_height:push rcx
		push rdx
_width:	mov eax, [r9 + r11 + 4]			; взять индексы цветов
		mov byte ptr [_shufd1 + 4], al
		mov byte ptr [_shufd2 + 4], ah
		shr eax, 16
		mov byte ptr [_shufd3 + 4], al
		mov byte ptr [_shufd4 + 4], ah
		mov ax, [r9 + r11]				; первый цвет
		mov bx, ax
		call dxtUnpack565				; распаковываем
		movd xmm0, eax
		pmovzxbd xmm2, xmm0
		cvtdq2ps xmm2, xmm2
		mov ax, [r9 + r11 + 2]			; второй цвет
		call dxtUnpack565				; распаковываем
		movd xmm1, eax
		pmovzxbd xmm3, xmm1
		cvtdq2ps xmm3, xmm3
		vaddps xmm4, xmm2, xmm3
		divps xmm4, xmm14
		xorps xmm5, xmm5
		test r11, r11				; проверка на отсутствие альфа канала
		jnz @f
		cmp bx, [r9 + r11 + 2]
		jb @f
		vmulps xmm4, xmm2, xmm14
		addps xmm4, xmm3
		vmulps xmm5, xmm3, xmm14
		addps xmm5, xmm2
		divps xmm4, xmm13
		divps xmm5, xmm13
		cvtps2dq xmm5, xmm5
		packssdw xmm5, xmm5
		packuswb xmm5, xmm5
@@:		cvtps2dq xmm4, xmm4
		packssdw xmm4, xmm4
		packuswb xmm4, xmm4
		unpcklps xmm0, xmm1
		unpcklps xmm4, xmm5
		movlhps xmm0, xmm4
		orps xmm0, xmm12
_shufd1:pshufd xmm1, xmm0, 0
		movaps [r8 + 00], xmm1
_shufd2:pshufd xmm2, xmm0, 0
		movaps [r8 + r12], xmm2
_shufd3:pshufd xmm3, xmm0, 0
		movaps [r8 + r12 * 2], xmm3
_shufd4:pshufd xmm4, xmm0, 0
		movaps [r8 + r13], xmm4
		; распаковываем альфа канал
		call r10
		; следующий блок
		add r9, 8
		add r8, 16
		dec rcx
		jnz _width
		pop rdx
		pop rcx
		add r8, r13
		dec rdx
		jnz _height
_fin:	ret
		; компрессор
_dxtC:	movaps xmm10, dxtGrid
		movaps xmm11, dxtHalf
		movaps xmm13, _fp0001
		movaps xmm14, _255x4
		mov r10, offset cDxt
		mov r10, [rax * 8 + r10]					; адрес функции запаковки альфа канала
		mov r11, offset dxtColors
		mov r14, offset dxtWeights
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
		lea r9, [r9 + r12 * 4]
		dec rdx
		jnz height
		ret
asm_ssh_bc_x endp


;static void EncodeBC1(_Out_ D3DX_BC1 *pBC, _In_reads_(NUM_PIXELS_PER_BLOCK) const HDRColorA *pColor, _In_ bool bColorKey, _In_ float alphaRef, _In_ DWORD flags)
; r8(pBC) r9(pColor)
fError		dd 16 dup(0)
bc_colors	dd 64 dup(0)

asm_ssh_encode_bc1 proc public
		vxorps ymm0, ymm0, ymm0
		vmovaps fError, ymm0						; memset(fError, 0x00, NUM_PIXELS_PER_BLOCK * sizeof(float));
		mov r10, offset bc_colors
		mov rcx, 16
_loop:	vmovaps xmm0, [r8]
		add r8, 4

		ret
asm_ssh_encode_bc1 endp

end		

static void EncodeBC1(_Out_ D3DX_BC1 *pBC, _In_reads_(NUM_PIXELS_PER_BLOCK) const HDRColorA *pColor, _In_ bool bColorKey, _In_ float alphaRef, _In_ DWORD flags)
{
    size_t uSteps;
    
	if(bColorKey) uSteps = 3; else uSteps = 4;

    // Quantize block to R56B5, using Floyd Stienberg error diffusion.  This 
    // increases the chance that colors will map directly to the quantized 
    // axis endpoints.
    HDRColorA Color[NUM_PIXELS_PER_BLOCK];
    HDRColorA Error[NUM_PIXELS_PER_BLOCK];

    size_t i;
    for(i = 0; i < NUM_PIXELS_PER_BLOCK; ++i)
    {
        HDRColorA Clr;
        Clr.r = pColor[i].r;
        Clr.g = pColor[i].g;
        Clr.b = pColor[i].b;

        Color[i].r = (float) static_cast<int32_t>(Clr.r * 31.0f + 0.5f) * (1.0f / 31.0f);
        Color[i].g = (float) static_cast<int32_t>(Clr.g * 63.0f + 0.5f) * (1.0f / 63.0f);
        Color[i].b = (float) static_cast<int32_t>(Clr.b * 31.0f + 0.5f) * (1.0f / 31.0f);

        Color[i].a = 1.0f;

        if (flags & BC_FLAGS_DITHER_RGB)
        {
            HDRColorA Diff;
            Diff.r = Color[i].a * (Clr.r - Color[i].r);
            Diff.g = Color[i].a * (Clr.g - Color[i].g);
            Diff.b = Color[i].a * (Clr.b - Color[i].b);

            if(3 != (i & 3))
            {
                assert( i < 15 );
                _Analysis_assume_( i < 15 );
                Error[i + 1].r += Diff.r * (7.0f / 16.0f);
                Error[i + 1].g += Diff.g * (7.0f / 16.0f);
                Error[i + 1].b += Diff.b * (7.0f / 16.0f);
            }

            if(i < 12)
            {
                if(i & 3)
                {
                    Error[i + 3].r += Diff.r * (3.0f / 16.0f);
                    Error[i + 3].g += Diff.g * (3.0f / 16.0f);
                    Error[i + 3].b += Diff.b * (3.0f / 16.0f);
                }

                Error[i + 4].r += Diff.r * (5.0f / 16.0f);
                Error[i + 4].g += Diff.g * (5.0f / 16.0f);
                Error[i + 4].b += Diff.b * (5.0f / 16.0f);

                if(3 != (i & 3))
                {
                    assert( i < 11 );
                    _Analysis_assume_( i < 11 );
                    Error[i + 5].r += Diff.r * (1.0f / 16.0f);
                    Error[i + 5].g += Diff.g * (1.0f / 16.0f);
                    Error[i + 5].b += Diff.b * (1.0f / 16.0f);
                }
            }
        }

        Color[i].r *= g_Luminance.r;
        Color[i].g *= g_Luminance.g;
        Color[i].b *= g_Luminance.b;
    }

    // Perform 6D root finding function to find two endpoints of color axis.
    // Then quantize and sort the endpoints depending on mode.
    HDRColorA ColorA, ColorB, ColorC, ColorD;

    OptimizeRGB(&ColorA, &ColorB, Color, uSteps, flags);

    ColorC.r = ColorA.r * g_LuminanceInv.r;
    ColorC.g = ColorA.g * g_LuminanceInv.g;
    ColorC.b = ColorA.b * g_LuminanceInv.b;

    ColorD.r = ColorB.r * g_LuminanceInv.r;
    ColorD.g = ColorB.g * g_LuminanceInv.g;
    ColorD.b = ColorB.b * g_LuminanceInv.b;

    uint16_t wColorA = Encode565(&ColorC);
    uint16_t wColorB = Encode565(&ColorD);

    if((uSteps == 4) && (wColorA == wColorB))
    {
        pBC->rgb[0] = wColorA;
        pBC->rgb[1] = wColorB;
        pBC->bitmap = 0x00000000;
        return;
    }

    Decode565(&ColorC, wColorA);
    Decode565(&ColorD, wColorB);

    ColorA.r = ColorC.r * g_Luminance.r;
    ColorA.g = ColorC.g * g_Luminance.g;
    ColorA.b = ColorC.b * g_Luminance.b;

    ColorB.r = ColorD.r * g_Luminance.r;
    ColorB.g = ColorD.g * g_Luminance.g;
    ColorB.b = ColorD.b * g_Luminance.b;

    // Calculate color steps
    HDRColorA Step[4];

    if((3 == uSteps) == (wColorA <= wColorB))
    {
        pBC->rgb[0] = wColorA;
        pBC->rgb[1] = wColorB;

        Step[0] = ColorA;
        Step[1] = ColorB;
    }
    else
    {
        pBC->rgb[0] = wColorB;
        pBC->rgb[1] = wColorA;

        Step[0] = ColorB;
        Step[1] = ColorA;
    }

    static const size_t pSteps3[] = { 0, 2, 1 };
    static const size_t pSteps4[] = { 0, 2, 3, 1 };
    const size_t *pSteps;

    if(3 == uSteps)
    {
        pSteps = pSteps3;

        HDRColorALerp(&Step[2], &Step[0], &Step[1], 0.5f);
    }
    else
    {
        pSteps = pSteps4;

        HDRColorALerp(&Step[2], &Step[0], &Step[1], 1.0f / 3.0f);
        HDRColorALerp(&Step[3], &Step[0], &Step[1], 2.0f / 3.0f);
    }

    // Calculate color direction
    HDRColorA Dir;

    Dir.r = Step[1].r - Step[0].r;
    Dir.g = Step[1].g - Step[0].g;
    Dir.b = Step[1].b - Step[0].b;

    float fSteps = (float) (uSteps - 1);
    float fScale = (wColorA != wColorB) ? (fSteps / (Dir.r * Dir.r + Dir.g * Dir.g + Dir.b * Dir.b)) : 0.0f;

    Dir.r *= fScale;
    Dir.g *= fScale;
    Dir.b *= fScale;

    // Encode colors
    uint32_t dw = 0;
    if (flags & BC_FLAGS_DITHER_RGB) memset(Error, 0x00, NUM_PIXELS_PER_BLOCK * sizeof(HDRColorA));

    for(i = 0; i < NUM_PIXELS_PER_BLOCK; ++i)
    {
        if((3 == uSteps) && (pColor[i].a < alphaRef))
        {
            dw = (3 << 30) | (dw >> 2);
        }
        else
        {
            HDRColorA Clr;
            Clr.r = pColor[i].r * g_Luminance.r;
            Clr.g = pColor[i].g * g_Luminance.g;
            Clr.b = pColor[i].b * g_Luminance.b;

            if (flags & BC_FLAGS_DITHER_RGB)
            {
                Clr.r += Error[i].r;
                Clr.g += Error[i].g;
                Clr.b += Error[i].b;
            }

            float fDot = (Clr.r - Step[0].r) * Dir.r + (Clr.g - Step[0].g) * Dir.g + (Clr.b - Step[0].b) * Dir.b;
            uint32_t iStep;

            if(fDot <= 0.0f) iStep = 0;
            else if(fDot >= fSteps) iStep = 1;
            else iStep = static_cast<uint32_t>( pSteps[static_cast<size_t>(fDot + 0.5f)] );
            dw = (iStep << 30) | (dw >> 2);
            if (flags & BC_FLAGS_DITHER_RGB)
            {
                HDRColorA Diff;
                Diff.r = Color[i].a * (Clr.r - Step[iStep].r);
                Diff.g = Color[i].a * (Clr.g - Step[iStep].g);
                Diff.b = Color[i].a * (Clr.b - Step[iStep].b);

                if(3 != (i & 3))
                {
                    Error[i + 1].r += Diff.r * (7.0f / 16.0f);
                    Error[i + 1].g += Diff.g * (7.0f / 16.0f);
                    Error[i + 1].b += Diff.b * (7.0f / 16.0f);
                }

                if(i < 12)
                {
                    if(i & 3)
                    {
                        Error[i + 3].r += Diff.r * (3.0f / 16.0f);
                        Error[i + 3].g += Diff.g * (3.0f / 16.0f);
                        Error[i + 3].b += Diff.b * (3.0f / 16.0f);
                    }

                    Error[i + 4].r += Diff.r * (5.0f / 16.0f);
                    Error[i + 4].g += Diff.g * (5.0f / 16.0f);
                    Error[i + 4].b += Diff.b * (5.0f / 16.0f);

                    if(3 != (i & 3))
                    {
                        Error[i + 5].r += Diff.r * (1.0f / 16.0f);
                        Error[i + 5].g += Diff.g * (1.0f / 16.0f);
                        Error[i + 5].b += Diff.b * (1.0f / 16.0f);
                    }
                }
            }
        }
    }

    pBC->bitmap = dw;

asm_ssh_encode_bc2 proc public
		ret
asm_ssh_encode_bc2 endp

asm_ssh_encode_bc3 proc public
		ret
asm_ssh_encode_bc3 endp

asm_ssh_encode_bc4 proc public
		ret
asm_ssh_encode_bc4 endp

asm_ssh_encode_bc5 proc public
		ret
asm_ssh_encode_bc5 endp

asm_ssh_encode_bc6 proc public
		ret
asm_ssh_encode_bc6 endp

asm_ssh_encode_bc7 proc public
		ret
asm_ssh_encode_bc7 endp













end
