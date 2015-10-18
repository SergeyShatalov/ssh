
include asm_ssh.inc

side_left	= 1
side_right	= 2
side_top	= 4
side_bottom	= 8

draw_side macro p, w, h, f
LOCAL _h, _w
		mov r15, p
		xor rsi, rsi
_h:		xor rax, rax
_w:		movq mm2, _mm_tmp
		movd mm3, dword ptr [r15 + rax * 4]
		call f
		movd dword ptr [r15 + rax * 4], mm2
		inc rax
		cmp rax, w
		jb _w
		add r15, r10
		inc rsi
		cmp rsi, h
		jb _h
endm

public _alpha_not, _mm_gamma

.const
align 16
_gamma		dd 0.3, 0.59, 0.11, 1.0, 0.3, 0.59, 0.11, 1.0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
f_256x8		dd 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0
f_inv255x8	dd 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862, 0.00392156862
f_inv256x8	dd 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625, 0.00390625
f_0_5x8		dd 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
f_3_0x8		dd 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0
f_1_0x8		dd 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
f_2_0x8		dd 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0
f_100x4		dd 100.0, 100.0, 100.0, 100.0
flt_max		dd 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F, 3.402823466e+38F
_fabs		dd 7fffffffh, 7fffffffh, 7fffffffh, 7fffffffh
_mm_not		dd -1, -1, -1, -1
_alpha_not	dw 255, 255, 255, 255
_mm_gamma	dw 77, 151, 28, 1, 77, 151, 28, 1
f_16_0		dd 16.0
f_8_0		dd 8.0
f_4_0		dd 4.0
f_1073741824_0 dd 1073741824.0

.data?
_clip		stk_bar<?>
_mm_tmp		dq ?
histogramm	dd 256 dup(?)

.code

; rcx(bar), rdx(clip)
asm_absolute_bar proc
local tmp:stk_bar
		lea rax, tmp
		vcvtdq2ps xmm0, xmmword ptr [rcx]
		vmovlps xmm1, xmm1, qword ptr [rdx]
		vmovlhps xmm1, xmm1, xmm1
		vcvtdq2ps xmm1, xmm1
		vdivps xmm0, xmm0, f_100x4
		vmulps xmm0, xmm0, xmm1
		vcvtps2dq xmm0, xmm0
		vmovups [rax], xmm0
		ret
asm_absolute_bar endp

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
asm_clip_bar proc USES rax rdi rsi rbx
		mov qword ptr [_clip + 00], 0		; стираем область выхода за пределы клипа
		mov qword ptr [_clip + 08], 0
		movsxd rsi, dword ptr [rdx + 00]	; wc
		movsxd rdi, dword ptr [rdx + 04]	; hc
		movsxd rax, dword ptr [rcx + 00]	; xb
		movsxd rbx, dword ptr [rcx + 04]	; yb
		movsxd rdx, dword ptr [rcx + 12]	; hb
		movsxd rcx, dword ptr [rcx + 08]	; wb
		test rdx, rdx						; для неопределенной области
		cmovle rdx, rdi
		test rcx, rcx
		cmovle rcx, rsi
		test rax, rax						; xb >= 0 -> skip
		jge @f
		add rcx, rax						; wb += xb <= 0 -> error
		jle _err
		neg rax
		mov _clip.x, eax					; -xb -> clip.x
		xor rax, rax						; xb = 0
@@:		lea r10, [rax + rcx]				; ww = xb + xw
		sub r10, rsi						; ww -= wc <= 0 -> skip
		jle @f
		sub rcx, r10						; wb -= ww <= 0 -> error
		jle _err
		mov _clip.w, r10d					; ww -> clip.w
@@:		test rbx, rbx						; yb >= 0 -> skip
		jge @f
		add rdx, rbx						; hb += yb <= 0 -> error
		jle _err
		neg rbx
		mov _clip.y, ebx					; -yb -> clip.y
		xor rbx, rbx						; yb = 0
@@:		lea r10, [rbx + rdx]				; hh = yb + hb
		sub r10, rdi						; hh -= wh <= 0 -> skip
		jle @f
		sub rdx, r10						; wh -= hh <= 0 -> error
		jle _err
		mov _clip.h, r10d					; hh -> clip.h
@@:		shl rsi, 2							; wc *= 4
		push rsi
		imul rsi, rbx						; wc *= yb
		lea rsi, [rsi + rax * 4]			; wc += xb * 4
		add r8, rsi							; r8 += wc -> начальный адрес области
		pop r10								; r10 = pitch
		clc
		ret
_err:	stc
		ret
asm_clip_bar endp

asm_ssh_v_flip proc public USES r11 r12 bar:QWORD, clip:QWORD, dst:QWORD
		call asm_clip_bar
		jc _fin
		dec rdx
		jle _fin
		mov r11, r10
		imul r11, rdx
_loop:	mov r9, r8
		mov r12, rcx
@@:		mov rax, [r9]
		xchg rax, [r9 + r11]
		mov [r9], rax
		add r9, 8
		sub r12, 2
		ja @b
		jz @f
		mov eax, [r9]			; если ширина не четная
		xchg rax, [r9 + r11]
		mov [r9], eax
@@:		add r8, r10
		sub r11, r10
		sub r11, r10
		sub rdx, 2
		jg _loop
_fin:	ret
asm_ssh_v_flip endp

asm_ssh_h_flip proc public USES r11 bar:QWORD, clip:QWORD, dst:QWORD
		call asm_clip_bar
		jc _fin
		shr rcx, 1
		jle _fin
		movq mm2, qword ptr swap_h_flip
_loop:	mov r9, r8
		lea r11, [rcx * 8 - 8]
		push rcx
		shr rcx, 1
@@:		movq mm0, [r9]						; a2 b2 g2 r2 a1 b1 g1 r1 -> a1 b1 g1 r1 a2 b2 g2 r2
		movq mm1, [r9 + r11]
		pshufb mm0, mm2
		pshufb mm1, mm2
		movq [r9 + r11], mm0
		movq [r9], mm1
		sub r11, 16
		add r9, 8
		loop @b
		pop rcx
		add r8, r10
		dec rdx
		jnz _loop
		emms
_fin:	ret
swap_h_flip db 4, 5, 6, 7, 0, 1, 2, 3
asm_ssh_h_flip endp

asm_ssh_flip_90 proc USES rbx wh:QWORD, dst:QWORD, src:QWORD
		mov r9, rdx
		movsxd rdx, [rcx].stk_range.h		; w->h
		movsxd rcx, [rcx].stk_range.w		; h->w
		jrcxz _fin
		lea rbx, [rdx * 4]					; pitch
		lea r9, [r9 + rbx - 4]
_loop:	mov r10, r9
		mov r11, rcx
@@:		mov eax, [r8]
		mov [r10], eax
		add r10, rbx
		add r8, 4
		loop @b
		mov rcx, r11
		sub r9, 4
		dec rdx
		jg _loop
_fin:	ret
asm_ssh_flip_90 endp

asm_ssh_figure proc bar:QWORD, clip:QWORD, pix:QWORD, modify:QWORD
		ret
comment $
; r13(buf) rcx(x) rdx(y) rbx(pitch)
filling proc private USES rdi rsi r8 r9
		mov rdi, offset tmpBuf
		imul rdx, rbx
		add rdx, rcx
		lea rsi, [rdi + 32768]
		mov [rsi], edx				; начальное смещение
		xor rax, rax
		mov dl, 1
		movzx rcx, dl				; начальное количество смещений
loop0:	mov r8, rsi
		mov r9, rdi
loop1:	lodsd						; смещение от начала буфера пикселей
		inc rax
		cmp [rax + r15], dl
		jz @f
		mov [rax + r15], dl
		stosd
@@:		sub rax, 2
		cmp [rax + r15], dl
		jz @f
		mov [rax + r15], dl
		stosd
@@:		lea rax, [rax + rbx + 1]
		cmp [rax + r15], dl
		jz @f
		mov [rax + r15], dl
		stosd
@@:		sub rax, rbx
		sub rax, rbx
		cmp [rax + r15], dl
		jz @f
		mov [rax + r15], dl
		stosd
@@:		loop loop1
		mov rcx, rdi
		sub rcx, r9
		shr rcx, 2
		jz _fin
		cmp rcx, 8191
		jae _fin
		mov rsi, r9
		mov rdi, r8
		jmp loop0
_fin:	ret
filling endp

; rcx(bar) rdx(clip) r8(src) r9(array) count vals msks pixOps pixOpsEx figure radius shadow
asmFigure proc USES r12 r13 r14 r15 rsi rdi rbx bar:QWORD, clip:QWORD, src:QWORD, array:QWORD, count:QWORD, vals:QWORD, msks:QWORD, pixOps:BYTE, pixOpsEx:BYTE, figure:BYTE, radius:QWORD, shadow:DWORD
LOCAL @@texW:QWORD, @@texH:QWORD, @@msk1:DWORD, @@msk2:DWORD, @@pt[8]:QWORD
		or radius, 1
		shr radius, 1
		mov [dataFigures + 8], r9
		mov rax, msks
		mov r10, [rax + 00]
		mov r11, [rax + 08]
		mov @@msk1, r10d
		mov @@msk2, r11d
		not r10
		not r11
		movd mm6, r10d
		movd mm7, r11d
		call asmClipBar
		jnc _fin
		push r8
		push rbx
		movaps xmm13, _fp100x4
		movss xmm14, _fp1_0
		movss xmm15, _fp3_0
		mov @@texW, rcx
		mov @@texH, rdx
		push rdx
		mov rsi, rcx
		dec rdx
		dec rcx
		cvtsi2ss xmm7, rdx
		cvtsi2ss xmm6, rcx
		divss xmm7, xmm13					; чтобы не делить на 100 и потом умножать на габариты буфера
		divss xmm6, xmm13
		add rcx, 3
		add rdx, 3							; расширить для "рамки" вокруг буфера (чтобы исключить выход за пределы)
		mov rbx, rcx
		imul rcx, rdx						; размер буферов
		sub rsp, 32
		call malloc
		add rsp, 32
		mov r15, rax						; буфер для картинки
		mov rdi, rax
		pop rdx								; h
		; заполнить буфер картинки с учетом "рамки"
		mov al, 1
		lea rcx, [rsi + 2]
		rep stosb
@@:		mov al, 1
		stosb
		mov rcx, rsi
		xor al, al
		rep stosb
		mov al, 1
		stosb
		dec rdx
		jnz @b
		mov al, 1
		lea rcx, [rsi + 2]
		rep stosb
		; параметры фигуры
		movzx rax, figure
		mov rsi, offset dataFigures
		mov rsi, [rax* 8 + rsi]				; параметры фигуры
		test rax, rax
		mov r9, offset fRegion
		mov rax, offset fEllipse
		cmovnz rax, r9						; функция рисования
		; нарисовать фигуру во временном буфере
		lea rdi, [r15 + rbx + 1]
		movzx r9, byte ptr [rsi]			; количество точек
		push r15
		call rax							; рисуем фигуру
		pop r15
		movzx rcx, byte ptr [rsi + 1]		; координаты заполнения
		movzx rdx, byte ptr [rsi + 2]
		jrcxz @f
		cvtsi2ss xmm0, rcx
		cvtsi2ss xmm1, rdx
		mulss xmm0, xmm6
		mulss xmm1, xmm7
		cvtss2si rcx, xmm0
		cvtss2si rdx, xmm1
		call filling						; заполнить фигуру
@@:		cmp shadow, 0
		jz nShadow
		; формирование тени - метод Превита
		mov rdx, 1
_h:		mov r11, rdx
		imul r11, rbx
		add r11, r15
		mov rcx, 1
_w:		xor rsi, rsi				; sum mtx
		xor rdi, rdi				; sum inv_mtx
		mov r10, offset mtxPrewit
		mov r12, r11
		xor r9, r9
@@:		movzx eax, byte ptr [r12 + rcx]
		imul eax, [r10]
		add edi, eax
		movzx eax, byte ptr [r12 + rcx]
		imul eax, [r10 + 36]
		add esi, eax
		movzx eax, byte ptr [r12 + rcx + 1]
		imul eax, [r10 + 4]
		add edi, eax
		movzx eax, byte ptr [r12 + rcx + 1]
		imul eax, [r10 + 40]
		add esi, eax
		movzx eax, byte ptr [r12 + rcx + 2]
		imul eax, [r10 + 8]
		add edi, eax
		movzx eax, byte ptr [r12 + rcx + 2]
		imul eax, [r10 + 44]
		add esi, eax
		add r10, 12
		add r12, rbx
		inc r9
		cmp r9, 3
		jb @b
		cmp esi, edi
		cmovl edi, esi
		test edi, edi
		jge @f
		mov byte ptr [r11 + rcx], 2
@@:		inc rcx
		cmp rcx, @@texW
		jle _w
		inc rdx
		cmp rdx, @@texH
		jle _h
nShadow:mov r11, offset pix_ops
		movzx rax, pixOps
		mov r10, [rax * 8 + r11]			; пиксельная операция внутри фигуры
		movzx eax, pixOpsEx
		mov r11, [rax * 8 + r11]			; пиксельная операция вне фигуры
		lea rsi, [r15 + rbx + 1]
		mov r14, offset cnvPixFigure
		mov rdx, @@texH
		mov r13, vals
		pop rbx
		pop r8
loop0:	push r8
		mov rcx, @@texW
@@:		lodsb
		movzx rax, al
		movd mm3, dword ptr [r8]
		call qword ptr [rax * 8 + r14]
		movd dword ptr [r8], mm3
		add r8, 4
		loop @b
		pop r8
		add rsi, 2
		add r8, rbx
		dec rdx
		jnz loop0
		sub rsp, 32
		mov rcx, r15
		call free
		add rsp, 32
_fin:	emms
		ret
cnvPixFigure	dq fig0, fig1, fig2
OPTION EPILOGUE:NONE
fig0:	movq mm0, [r13 + 00]
		movd mm1, @@msk1
		movq mm2, mm6
		jmp r10
fig1:	movq mm0, [r13 + 08]
		movd mm1, @@msk2
		movq mm2, mm7
		jmp r11
fig2:	movd mm2, dword ptr shadow
		psubusb mm3, mm2
		ret
dataFigures	dq dataEllipse, 0, dataRect, dataUTri, dataDTri, dataRTri, dataLTri, dataPyramid, dataSixangle
			dq dataEightangle, dataRomb, dataStar1, dataStar2, dataRarrow, dataLarrow, dataDarrow, dataUarrow
			dq dataCross45, dataChecked, dataVPlzSlider, dataHPlzSlider, dataPlus
getPoint:	mov rax, rcx							; для замыкания
			xor rdx, rdx
			div r9
			movzx rax, byte ptr [rsi + rdx * 2 + 3]
			movzx rdx, byte ptr [rsi + rdx * 2 + 4]
			cvtsi2ss xmm4, rax
			cvtsi2ss xmm5, rdx
			mulss xmm4, xmm6
			mulss xmm5, xmm7
			ret
radiusPt:	mov r8, -1
			mov r14, r11
			imul r14, rbx
			add r14, r10
			mov rdx, 1				; delta X
			sub rcx, r10			; приращатели
			jge @f
			neg rcx
			neg rdx
@@:			setnz r10b
			movzx r10, r10b
			imul r10, rdx
			mov rdx, rbx
			sub rax, r11
			jge @f
			neg rax
			neg rdx
@@:			setnz r11b
			movzx r11, r11b
			imul r11, rdx
			cmp rcx, rax			; направления
			jge @f
			xchg rcx, rax			; <
			xchg r10, r11
@@:			mov r12, rcx			; r12(ebp,r13) = delta X, rax(rsi) = delta Y, rcx - length, r10(rax) - offsX, r11(rbx) - offsY
			; считаем 2 точки радиуса
			sub rcx, radius
			jle _pt2
			xor rdx, rdx			; error
_loopr:		add rdx, rax			; главный цикл создания линии(в данном случае только расчет координат)
			lea r13, [rdx * 2]
			cmp r13, r12
			jl @f
			add r14, r11
			sub rdx, r12
@@:			add r14, r10
			cmp rcx, radius
			jnz @f
			; 1 точка
			mov r8, r14
@@:			loop _loopr
_pt2:		; 2 точка
			test r8, r8
			cmovs r8, r14
			mov rax, r14
			xor rdx, rdx
			div rbx
			; записываем точки(они временные)
			mov [r9 + 32], rdx
			mov [r9 + 40], rax
			xor rdx, rdx
			mov rax, r8
			div rbx
			mov [r9 + 00], rdx
			mov [r9 + 08], rax
			add r9, 16
			ret						;rax = pty2 rcx = ptx2
calcRound:	push rcx
			push r9
			lea r9, @@pt
			cvttss2si r10, xmm0		; x0
			cvttss2si r11, xmm1		; y0
			cvttss2si rcx, xmm2		; x1
			cvttss2si rax, xmm3		; y1
			call radiusPt
			cvttss2si r10, xmm4		; x0
			cvttss2si r11, xmm5		; y0
			cvttss2si rcx, xmm2		; x1
			cvttss2si rax, xmm3		; y1
			call radiusPt
			pop r9
			pop rcx
			ret
calcBesie:	cvtsi2ss xmm4, r14
			cvtsi2ss xmm13, r13
			divss xmm4, xmm13					; scalar
			movss xmm5, xmm14					; 1 - scalar
			subss xmm5, xmm4
			movss xmm8, xmm5
			mulss xmm8, xmm8
			mulss xmm8, xmm5
			movss xmm9, xmm8
			cvtsi2ss xmm10, @@pt + 00
			cvtsi2ss xmm11, @@pt + 08
			mulss xmm8, xmm10
			mulss xmm9, xmm11
			movss xmm12, xmm8
			movss xmm13, xmm9
			movss xmm8, xmm5
			mulss xmm8, xmm8
			mulss xmm8, xmm4
			mulss xmm8, xmm15
			movss xmm9, xmm8
			cvtsi2ss xmm10, @@pt + 32
			cvtsi2ss xmm11, @@pt + 40
			mulss xmm8, xmm10
			mulss xmm9, xmm11
			addss xmm12, xmm8
			addss xmm13, xmm9
			movss xmm8, xmm4
			mulss xmm8, xmm8
			mulss xmm8, xmm5
			mulss xmm8, xmm15
			movss xmm9, xmm8
			cvtsi2ss xmm10, @@pt + 48
			cvtsi2ss xmm11, @@pt + 56
			mulss xmm8, xmm10
			mulss xmm9, xmm11
			addss xmm12, xmm8
			addss xmm13, xmm9
			movss xmm8, xmm4
			mulss xmm8, xmm8
			mulss xmm8, xmm4
			movss xmm9, xmm8
			cvtsi2ss xmm10, @@pt + 16
			cvtsi2ss xmm11, @@pt + 24
			mulss xmm8, xmm10
			mulss xmm9, xmm11
			addss xmm12, xmm8
			addss xmm13, xmm9
			ret
funcRound:	mov r13, radius
			xor r14, r14
			call calcBesie
			movss xmm0, xmm12
			movss xmm1, xmm13
			inc r14
@@:			call calcBesie
			movss xmm2, xmm12
			movss xmm3, xmm13
			call funcLine
			movss xmm0, xmm2
			movss xmm1, xmm3
			inc r14
			cmp r14, r13
			jle @b
			ret
funcLine:	push rcx
			push rbx
			; координаты
			cvttss2si r10, xmm0		; x0
			cvttss2si r11, xmm1		; y0
			cvttss2si rcx, xmm2		; x1
			cvttss2si rax, xmm3		; y1
			mov rdx, 1				; delta X
			mov r8, r11				; адрес начальной точки
			imul r8, rbx
			add r8, r10
			add r8, rdi
			sub rcx, r10			; приращатели
			jge @f
			neg rcx
			neg rdx
@@:			setnz r10b
			movzx r10, r10b
			imul r10, rdx
			mov rdx, rbx
			sub rax, r11
			jge @f
			neg rax
			neg rdx
@@:			setnz r11b
			movzx r11, r11b
			imul r11, rdx
			cmp rcx, rax			; направления
			jge @f
			xchg rcx, rax			; <
			xchg r10, r11
@@:			jrcxz _finLn
			mov rbx, rcx			; delta X
			xor rdx, rdx			; error
_ln:		mov byte ptr [r8], 1
			add rdx, rax			; error += delta Y
			lea r12, [rdx * 2]
			cmp r12, rbx			; error * 2 >= deltaX
			jl @f
			add r8, r11				; buf += offsY
			sub rdx, rbx			; error -= deltaX
@@:			add r8, r10				; buf += offsX
			loop _ln
_finLn:		pop rbx
			pop rcx
			ret
ellipsePt:	push rax
 			mov rax, rdx
 			imul rax, rbx
			lea r14, [rax + rcx]
 			mov byte ptr [r11 + r14], 1
			sub rax, rcx
 			mov byte ptr [r11 + rax], 1
			mov rax, rdx
			imul rax, rbx
			mov rax, rdx
			neg rax
			imul rax, rbx
			lea r14, [rax + rcx]
 			mov byte ptr [r11 + r14], 1
			sub rax, rcx
 			mov byte ptr [r11 + rax], 1
			pop rax
			ret
fEllipse:	mov rcx, @@texW
			mov rdx, @@texH
			shr rdx, 1
			jle no2
			shr rcx, 1
			jle no2
			mov r10, rbx				; pitch
			imul r10, rdx
			add r10, rcx
			add r10, rdi
			mov r11, r10
			mov rax, rcx
			mov r10, rdx
			xor rcx, rcx				; x = 0
			mov rdx, r10				; y = b
			imul rax, rax				; a2 = a * a
			imul r10, r10				; b2 = b * b
			lea rdi, [rdx * 2]			; 2 * b2
			mov r12, 1
			sub r12, rdi
			imul r12, rax
			lea r12, [r12 + r10 * 2]	; S = a2 * (1 - 2 * b) + 2 * b2
			dec rdi
			imul rdi, rax
			shl rdi, 1
			sub rdi, r10
			neg rdi						; T = b2 - 2 * a2 * (2 * b - 1)
			call ellipsePt
 ellips:	push r11
			test r12, r12
			jge @f
			lea r11, [rcx * 2 + 3]
			imul r11, r10
			shl r11, 1
			add r12, r11
			inc rcx
			jmp next
@@:			test rdi, rdi
			jl @f
			push rdi
			lea r11, [rdx - 1]
			imul r11, rax
			shl r11, 2
			lea rdi, [rcx * 2 + 3]
			imul rdi, r10
			shl rdi, 1
			sub rdi, r11
			add r12, rdi
			pop rdi
			push r12
			lea r11, [rdx * 2 - 3]
			imul r11, rax
			shl r11, 1
			lea r12, [rcx + 1]
			imul r12, r10
			shl r12, 2
			sub r12, r11
			add rdi, r12
			pop r12
			inc rcx
			dec rdx
			jmp next
@@:			lea r11, [rdx -1]
			imul r11, rax 
			shl r11, 2
			sub r12, r11
			lea r11, [rdx * 2 - 3]
			imul r11, rax
			shl r11, 1
			sub rdi, r11
			dec rdx
next:		pop r11
			call ellipsePt
			test rdx, rdx
			jg ellips
no2:		ret
fRegion:	xor rcx, rcx
			cmp radius, rcx
			jnz round
			call getPoint
			movss xmm0, xmm4
			movss xmm1, xmm5
			inc rcx
@@:			call getPoint
			movss xmm2, xmm4
			movss xmm3, xmm5
			inc rcx
			call funcLine
			movss xmm0, xmm2
			movss xmm1, xmm3
			cmp rcx, r9
			jbe @b
			ret
round:		dec rcx
			call getPoint
			movss xmm0, xmm4
			movss xmm1, xmm5
			inc rcx
			call getPoint
			movss xmm2, xmm4
			movss xmm3, xmm5
			inc rcx
			call getPoint
			call calcRound
			mov rax, r9
@@:			push rax
			cvtsi2ss xmm0, @@pt + 16
			cvtsi2ss xmm1, @@pt + 24
			call getPoint
			movss xmm2, xmm4
			movss xmm3, xmm5
			inc rcx
			call getPoint
			call calcRound
			cvtsi2ss xmm2, @@pt + 00
			cvtsi2ss xmm3, @@pt + 08
			call funcLine
			call funcRound
			pop rax
			dec rax
			jg @b
			ret
asmFigure endp

OPTION EPILOGUE:EpilogueDef
$

asm_ssh_figure endp

asm_ssh_gradient proc USES r11 r12 r13 r14 r15 rsi rbx bar:QWORD, clip:QWORD, pix:QWORD, modify:QWORD
		vzeroupper
		movsxd r14, [rcx].stk_bar.w
		movsxd r15, [rcx].stk_bar.h
		call asm_clip_bar
		jc _fin
		pxor mm7, mm7
		movq mm6, qword ptr _mm_alpha
		movd mm0, [r9].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, qword ptr _mm_not
		mov r11, offset _func_ops
		mov r12, offset _func_tex
		movsxd rax, [r9].stk_modify.src_ops
		mov r11, [r11 + rax * 8]						; функция пиксельной операции
		movsxd rax, [r9].stk_modify.type_address
		mov r12, [r12 + rax * 8]						; функция адресации координат
		; преобразовать начальный и конечный цвет в плавающий формат
		vmovaps xmm12, f_255x8
		vmovaps xmm13, f_1_0x8
		vpmovzxbd xmm14, dword ptr [r9].stk_modify.src_val
		vpmovzxbd xmm15, dword ptr [r9].stk_modify.dst_val
		vcvtdq2ps xmm14, xmm14
		vcvtdq2ps xmm15, xmm15
		vdivps xmm14, xmm14, xmm12						; color1
		vdivps xmm15, xmm15, xmm12						; color2
		; произвести предрасчет градиента
		vcvtsi2ss xmm0, xmm0, r14						; plane_x
		vcvtsi2ss xmm1, xmm1, r15						; plane_y
		vdivss xmm6, xmm0, [r9].stk_modify.wh_repeat.w
		vdivss xmm7, xmm1, [r9].stk_modify.wh_repeat.h
		vdivss xmm8, xmm0, xmm6							; dx
		vdivss xmm9, xmm1, xmm7							; dy
		vmulss xmm2, xmm0, xmm0							; xx = plane_x * plane_x
		vfmadd231ss xmm2, xmm1, xmm1					; xx += plane_y * plane_y
		vrsqrtss xmm4, xmm2, xmm2						; length = 1.0f / sqrtf(xx)
		vmulss xmm2, xmm4, xmm4							; length *= length
		vmulss xmm0, xmm0, xmm2
		vmulss xmm1, xmm1, xmm2
		; сдвинуть координаты на clip.x, clip.y
		vshufps xmm7, xmm9, xmm8, 00000000b				; dx|dx|dy|dy
		vcvtsi2ss xmm6, xmm6, _clip.y
		vmulss xmm6, xmm6, xmm9							; y
		vshufps xmm7, xmm6, xmm7, 00100000b				; dy|dx|y|y
		vcvtsi2ss xmm6, xmm6, _clip.x
		vmulss xmm6, xmm6, xmm8							; x
_h:		push rdx
		push rcx
		vmovss xmm7, xmm7, xmm6							; dy|dx|y|x
		vcvtps2dq xmm5, xmm7
		vpextrd r13, xmm5, 1
		mov r9, r8
@@:		vcvtss2si rbx, xmm7
		call r12										; вернуть координаты, в соответствии с адресацией координат
		vcvtsi2ss xmm2, xmm2, rbx
		vcvtsi2ss xmm3, xmm3, rdx
		vmulss xmm2, xmm2, xmm0							; x *= plane_x
		vfmadd231ss xmm2, xmm3, xmm1					; x += y * plane_y
		vsubss xmm2, xmm2, xmm4							; x -= plane_z
		vshufps xmm2, xmm2, xmm2, 0
		vsubps xmm3, xmm13, xmm2						; tmp2 = 1 - color1
		vmulps xmm2, xmm15, xmm2						; tmp1 = color2 * color1
		vfmadd231ps xmm2, xmm3, xmm14					; tmp1 += tmp2 * color1
		vmulps xmm2, xmm2, xmm12
		vcvtps2dq xmm2, xmm2							; пакуем цвет
		vpackssdw xmm2, xmm2, xmm2
		movdq2q mm2, xmm2
		packuswb mm2, mm2
		movd mm3, dword ptr [r9]
		call r11										; пиксельная операция
		movd dword ptr [r9], mm2
		add r9, 4
		vmovhlps xmm2, xmm2, xmm7						; 0|0|dy|dx
		vaddss xmm7, xmm7, xmm2							; dy|dx|y|x+dx
		loop @b
		vmovlhps xmm2, xmm2, xmm7						; y|x|0|0
		vaddps xmm2, xmm2, xmm7							; y+dy|x+dx|y|x
		vmovhlps xmm7, xmm7, xmm2						; dy|dx|y+dy|x+dx
		pop rcx
		pop rdx
		add r8, r10
		dec rdx
		jnz _h
_fin:	emms
		ret
_func_tex	dq _clamp,_mirror,_repeat,_clamp, _mirror, _repeat
epilog_none
_clamp:	mov rdx, r13
		ret
_repeat:xor rdx, rdx
		mov rax, rbx
		div r14
		mov rbx, rdx
		xor rdx, rdx
		mov rax, r13
		div r15
		ret
_mirror:mov rax, rbx
		xor rdx, rdx
		lea rsi, [r14 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, r14
		cmovge rdx, rsi
		mov rbx, rdx
		mov rax, r13
		xor rdx, rdx
		lea rsi, [r15 * 2]
		div rsi
		lea rax, [rdx + 1]
		sub rsi, rax
		cmp rdx, r15
		cmovge rdx, rsi
		ret
epilog_def
asm_ssh_gradient endp

asm_ssh_replace proc vals:QWORD, msks:QWORD, pix:QWORD, clip:QWORD
		mov r10, r8
		mov r8d, [rcx].stk_range.w				; src_val
		movd mm0, dword ptr [rcx].stk_range.h	; dst_val
		movsxd rax, [r9].stk_range.w
		movsxd rcx, [r9].stk_range.h
		imul rcx, rax							; длина изображения
		jrcxz _fin
		mov r9d, [rdx].stk_range.w				; src_msk
		movd mm1, dword ptr [rdx].stk_range.h	; dst_msk
		movq mm2, mm1
		pandn mm2, mm0							; dst_val
_loop:	movd mm0, dword ptr [r10]
		movd edx, mm0
		and edx, r9d
		cmp edx, r8d
		jnz @f
		pand mm0, mm1
		por mm0, mm2
		movd dword ptr [r10], mm0
@@:		add r10, 4
		loop _loop
		emms
_fin:	ret
asm_ssh_replace endp

histogramm_rgb			= 0
histogramm_red			= 1
histogramm_green		= 2
histogramm_blue			= 3
histogramm_alpha		= 4
histogramm_rgb_v		= 5
histogramm_red_v		= 6
histogramm_green_V		= 7
histogramm_blue_v		= 8
histogramm_alpha_v		= 9

asm_ssh_histogramm proc USES rsi rdi rbx wh:QWORD, modify:QWORD, buf:QWORD
		vmovaps xmm1, _gamma
		mov r11, offset _addr
		mov eax, [rdx].stk_modify.type_histogramm
		mov r11, [r11 + rax * 8]					; процедура извлечения канала
		mov rdi, offset histogramm					; буфер гистограммы
		mov rsi, rcx
		mov r9, [rdx].stk_modify.rgba.buf
		movsxd rcx, [rdx].stk_modify.wh_rgba.w
		imul ecx, [rdx].stk_modify.wh_rgba.h
		mov r10, rdi
		mov rbx, rcx
		mov rcx, 128
		xor rax, rax								; заполняем нулями буфер
		rep stosq
		mov rcx, rbx
@@:		call r11
		inc dword ptr [rax * 4 + r10]
		add r9, 4
		loop @b
		xor r11, r11								; максимальное значение
		mov rcx, r11
		vcvtsi2ss xmm1, xmm1, rbx
@@:		movsxd rax, dword ptr [r10 + rcx * 4]
		vcvtsi2ss xmm0, xmm0, rax
		movss dword ptr [r10 + rcx * 4], xmm0
		cmp r11, rax
		cmovl r11, rax
		inc rcx
		cmp rcx, 255
		jb @b
		cmp [rdx].stk_modify.type_histogramm, histogramm_rgb_v
		jb img
		mov [r8], ebx
		lea rdi, [r8 + 4]
		mov rsi, r10
		mov rcx, 128
		rep movsq
		ret
img:	movsxd rbx, [rsi].stk_range.w				; w
		movsxd rsi, [rsi].stk_range.h				; h
		mov rcx, rbx
		imul rcx, rsi
		vcvtsi2ss xmm0, xmm0, rbx
		vdivss xmm0, xmm0, f_256x8					; dx
		vcvtsi2ss xmm1, xmm1, rsi
		vcvtsi2ss xmm2, xmm2, r11					; max_h
		vdivss xmm1, xmm1, xmm2						; dh
		shl rbx, 2									; pitch
		; заполнить картинку цветом гистограммы
		mov eax, [rdx]. stk_modify.cols_histogramm.h; f_rgba
		mov rdi, r8
		rep stosd
		mov eax, [rdx]. stk_modify.cols_histogramm.w; b_rgba
		; находим x1, x2
		xor rdx, rdx
_loop:	mov rdi, rdx
@@:		cmp rdi, 255
		jae @f
		inc rdi
		movsxd rcx, dword ptr [r10 + rdi * 4]
		jrcxz @b
@@:		cmp rdx, rdi
		jnz @f
		cmp rdx, 255
		jz _fin
		mov rdi, 255
@@:		vmulss xmm2, xmm1, dword ptr [r10 + rdx * 4]; h1
		vmulss xmm3, xmm1, dword ptr [r10 + rdi * 4]; h2
		vcvtsi2ss xmm4, xmm4, rdx
		vcvtsi2ss xmm5, xmm5, rdi
		vmulss xmm4, xmm4, xmm0						; x1
		vmulss xmm5, xmm5, xmm0						; x2
		vsubss xmm6, xmm5, xmm4						; w_length
		vsubss xmm7, xmm3, xmm2						; h_length
		vdivss xmm7, xmm7, xmm6						; dy = h_length / w_length
		vcvtss2si r9, xmm4
		vcvtss2si rcx, xmm6
		cmp rcx, 1
		adc rcx, 0
		mov rdx, rdi
		lea r9, [r8 + r9 * 4]
_loop1:	vcvtss2si r11, xmm2							; h
		sub r11, rsi
		neg r11
		jle _no
		mov rdi, r9
@@:		mov [rdi], eax
		add rdi, rbx
		dec r11
		jnz @b
_no:	vaddss xmm2, xmm2, xmm7
		add r9, 4
		loop _loop1
		jmp _loop
_fin:	ret
_addr	dq _rgb, _red, _green, _blue, _alpha, _rgb, _red, _green, _blue, _alpha
epilog_none
_red:	movzx rax, byte ptr [r9 + 0]
		ret
_green:	movzx rax, byte ptr [r9 + 1]
		ret
_blue:	movzx rax, byte ptr [r9 + 2]
		ret
_alpha:	movzx rax, byte ptr [r9 + 3]
		ret
_rgb:	vpmovzxbd xmm0, dword ptr [r9]
		vcvtdq2ps xmm0, xmm0
		vdivps xmm0, xmm0, f_255x8
		vdpps xmm0, xmm0, xmm1, 01110001b
		vmulps xmm0, xmm0, f_255x8
		vcvtss2si rax, xmm0
		ret
epilog_def
asm_ssh_histogramm endp

asm_ssh_correct proc USES rdi clip:QWORD, wh:QWORD, type:DWORD, pix:QWORD
		movsxd r11, [rcx].stk_range.w
		imul r11d, [rcx].stk_range.h			; длина изображения
		; фиксируем диапазон
		movsxd rcx, [rdx].stk_range.w			; start
		movsxd rdx, [rdx].stk_range.h			; end
		xor rax, rax
		mov r10, 255
		cmp rcx, rax
		cmovl rcx, rax
		cmp rdx, rax
		cmovl rdx, rax
		cmp rcx, r10
		cmovg rcx, r10
		cmp rdx, r10
		cmovg rdx, r10							; clamp(start, 0, 255), clamp(end, 0, 255)
		mov rax, rcx
		cmp rcx, rdx
		cmovg rcx, rdx
		cmp rdx, rax
		cmovl rdx, rax							; start > end -> swap(start, end)
		mov r10, offset _addr1
		mov r10, [r10 + r8 * 8]
		mov r8, offset histogramm
		mov rdi, r8
		xor rax, rax
		sub rdx, rcx
		rep stosb								; стереть до начальной позиции диапазона
		mov rcx, rdx
		jrcxz _fin
		vcvtsi2ss xmm1, xmm1, rcx
		vmovaps xmm0, f_256x8
		vdivss xmm1, xmm0, xmm1					; dx
		vxorps xmm0, xmm0, xmm0					; x
@@:		vcvtss2si rax, xmm0
		vaddss xmm0, xmm0, xmm1
		stosb
		loop @b
		mov rcx, rdi
		sub rcx, r8
		sub rcx, 256
		neg rcx
		mov rax, 255
		rep stosb								; заполнить конец диапазона значениями 255
		mov rcx, r11							; корректировать изображение
@@:		call r10
		add r9, 4
		loop @b
_fin:	ret
_addr1	dq _rgb, _red, _green, _blue, _alpha, _rgb, _red, _green, _blue, _alpha
epilog_none
_rgb:	mov al, [r9 + 0]
		mov al, [rax + r8]
		mov [r9 + 0], al
		mov al, [r9 + 1]
		mov al, [rax + r8]
		mov [r9 + 1], al
		mov al, [r9 + 2]
		mov al, [rax + r8]
		mov [r9 + 2], al
		ret
_red:	mov al, [r9 + 0]
		mov al, [rax + r8]
		mov [r9 + 0], al
		ret
_green:	mov al, [r9 + 1]
		mov al, [rax + r8]
		mov [r9 + 1], al
		ret
_blue:	mov al, [r9 + 2]
		mov al, [rax + r8]
		mov [r9 + 2], al
		ret
_alpha:	mov al, [r9 + 3]
		mov al, [rax + r8]
		mov [r9 + 3], al
		ret
epilog_def
asm_ssh_correct endp

asm_noise proc
		xor rax, rax
		cmp r11, rax
		cmovl r11, rax
		cmp r12, rax
		cmovl r12, rax
		cmp r11, r14
		cmovg r11, r14
		cmp r12, r15
		cmovg r12, r15						; x = clamp(x, 0, r14), y = clamp(y, 0, r15)
		imul r12, 57
		add r11, r12						; n
		mov rax, r11
		shl rax, 13
		xor r11, rax						; n = (n << 13) ^ n
		mov r12, r11
		imul r12, r11						; nn = n * n
		imul r12, 15731						; nn *= 15731
		add r12, 789221						; nn += 789221
		imul r11, r12						; n *= nn
		add r11, 1376312589					; n += 1376312589
		and r11, 7fffffffh					; n &= 7fffffff
		vcvtsi2ss xmm10, xmm10, r11
		vsubss xmm10, xmm12, xmm10			; n = 1.0 - n
		vdivss xmm10, xmm10, f_1073741824_0 ; n /= 1073741824.0
		ret
asm_noise endp

asm_smooth_noise proc
		lea r11, [rcx - 1]
		lea r12, [rdx - 1]
		call asm_noise
		vmovss xmm8, xmm8, xmm10
		lea r11, [rcx + 1]
		lea r12, [rdx - 1]
		call asm_noise
		vaddss xmm8, xmm8, xmm10
		lea r11, [rcx + 1]
		lea r12, [rdx + 1]
		call asm_noise
		vaddss xmm8, xmm8, xmm10
		lea r11, [rcx + 1]
		lea r12, [rdx + 1]
		call asm_noise
		vaddss xmm8, xmm8, xmm10		; corners
		lea r11, [rcx - 1]
		mov r12, rdx
		call asm_noise
		vmovss xmm9, xmm9, xmm10
		lea r11, [rcx + 1]
		mov r12, rdx
		call asm_noise
		vaddss xmm9, xmm9, xmm10
		mov r11, rcx
		lea r12, [rdx - 1]
		call asm_noise
		vaddss xmm9, xmm9, xmm10
		mov r11, rcx
		lea r12, [rdx + 1]
		call asm_noise
		vaddss xmm9, xmm9, xmm10		; sides
		mov r11, rcx
		mov r12, rdx
		call asm_noise
		vdivss xmm8, xmm8, f_16_0
		vdivss xmm9, xmm9, f_8_0
		vdivss xmm10, xmm10, f_4_0
		vaddss xmm10, xmm10, xmm9
		vaddss xmm10, xmm10, xmm8
		ret
asm_smooth_noise endp

asm_ssh_noise_perlin proc USES rbx rsi rdi r12 r13 r14 r15 clip:QWORD, vals:DWORD, dst:QWORD, modify:QWORD
		movsxd rax, [r9].stk_modify.src_ops
		mov r10, offset _func_ops
		mov r10, [r10 + rax * 8]				; пиксельная операция
		movsxd r14, [rcx].stk_range.w
		movsxd r15, [rcx].stk_range.h
		lea rbx, [r14 * 4]						; pitch
		dec r14
		dec r15
		movss xmm12, f_1_0x8
		movss xmm14, [r9].stk_modify.flt_vec.x	; frequency
		movss xmm15, [r9].stk_modify.flt_vec.y	; persistence
		pxor mm7, mm7
		movq mm6, qword ptr _mm_alpha
		movd mm0, [r9].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, qword ptr _mm_not
		mov r9, rdx								; число октав
		xor rdx, rdx							; y
_h:		push rdx
		push r8
		xor rcx, rcx							; x
_w:		push rcx
		vxorps xmm11, xmm11, xmm11				; total
		xor r13, r13
@@:		; интерполяция
		vcvtsi2ss xmm0, xmm0, rcx				; x
		vcvtsi2ss xmm1, xmm1, rdx				; y
		lea rax, [r13 + 1]
		vcvtsi2ss xmm13, xmm13, rax				; ii = i + 1
		vmulss xmm10, xmm13, f_2_0x8			; tmp = ii * 2
		vmulss xmm0, xmm0, xmm13				; x *= tmp
		vmulss xmm1, xmm1, xmm13				; y *= tmp
		vroundss xmm2, xmm0, xmm0, 11b			; int_x
		vroundss xmm3, xmm1, xmm1, 11b			; int_y
		vcvtss2si rsi, xmm2
		vcvtss2si rdi, xmm3
		vsubss xmm2, xmm0, xmm2					; frac_x
		vsubss xmm3, xmm1, xmm3					; frac_y
		mov rcx, rsi
		mov rdx, rdi
		call asm_smooth_noise
		vmovss xmm4, xmm4, xmm10				; v1
		lea rcx, [rsi + 1]
		mov rdx, rdi
		call asm_smooth_noise
		vmovss xmm5, xmm5, xmm10				; v2
		mov rcx, rsi
		lea rdx, [rdi + 1]
		call asm_smooth_noise
		vmovss xmm6, xmm6, xmm10				; v3
		lea rcx, [rsi + 1]
		lea rdx, [rdi + 1]
		call asm_smooth_noise
		vmovss xmm7, xmm7, xmm10				; v4
		vsubss xmm8, xmm12, xmm2				; sx = 1.0 - frac_x
		vsubss xmm9, xmm12, xmm3				; sy = 1.0 - frac_y
		vmulss xmm4, xmm4, xmm2					; v1 *= frac_x
		vfmadd231ss xmm4, xmm5, xmm8			; i1 = v1 + v2 * sx
		vmulss xmm6, xmm6, xmm2					; v3 *= frac_x
		vfmadd231ss xmm6, xmm7, xmm8			; i2 = v3 + v4 * sx
		vmulss xmm4, xmm4, xmm3					; i1 *= frac_y
		vfmadd231ss xmm4, xmm6, xmm9			; i1 = i1 + i2 * sy
		vmulss xmm10, xmm13, xmm15				; tmp = ii * amplitude
		vfmadd231ss xmm11, xmm4, xmm10			; total += i1 * tmp
		inc r13
		cmp r13, r9
		jb @b
		vpshufd xmm11, xmm11, 0b
		vpackssdw xmm11, xmm11, xmm11
		vpackuswb xmm11, xmm11, xmm11
		movd dword ptr [r8], xmm11
		pop rcx
		add r8, 4
		inc rcx
		cmp rcx, r14
		jbe _w
		pop r8
		pop rdx
		add r8, rbx
		inc rdx
		cmp rdx, r15
		jbe _h
		ret
asm_ssh_noise_perlin endp

asm_ssh_noise_terrain proc bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
		ret
asm_ssh_noise_terrain endp

asm_draw_border proc USES rsi r13 rbx r14 r15
		pxor mm7, mm7
		movq mm6, qword ptr _mm_alpha
		movd mm0, [r9].stk_modify.src_msk
		movq mm1, mm0
		pandn mm1, qword ptr _mm_not
		movd mm2, [r9].stk_modify.src_val
		movq _mm_tmp, mm2
		movsxd r13, [r9].stk_modify.sides
		movsxd r9, [r9].stk_modify.w_border
		test r9, r9
		jle _fin
		call asm_clip_bar
		jc _fin
		test r13, side_top
		jz @f
		mov rbx, r9
		sub ebx, _clip.y
		jle @f
		draw_side r8, rcx, rbx, r11
		sub rdx, rbx
		imul rbx, r10
		add r8, rbx
@@:		test r13, side_bottom
		jz @f
		mov rbx, r9
		sub ebx, _clip.h
		jle @f
		sub rdx, rbx
		mov r14, rdx
		imul r14, r10
		add r14, r8
		draw_side r14, rcx, rbx, r12
@@:		test r13, side_left
		jz @f
		mov rbx, r9
		sub ebx, _clip.x
		jle @f
		draw_side r8, rbx, rdx, r11
		lea r8, [r8 + rbx * 4]
		sub rcx, rbx
@@:		test r13, side_right
		jz _fin
		mov rbx, r9
		sub ebx, _clip.w
		jle _fin
		sub rcx, rbx
		lea r14, [r8 + rcx * 4]
		draw_side r14, rbx, rdx, r12
_fin:	emms
		ret
asm_draw_border endp

asm_ssh_border2d proc USES r12 bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
		mov r11, offset _func_ops
		movsxd rax, [r9].stk_modify.src_ops
		mov r11, [r11 + rax * 8]					; процедура отрисовка левой и верхней стороны
		mov r12, r11								; процедура отрисовка правой и нижней стороны
		call asm_draw_border
		ret
asm_ssh_border2d endp

asm_ssh_border3d proc USES r12 bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
		mov r11, offset _add						; процедура отрисовка левой и верхней стороны
		mov r12, offset _sub						; процедура отрисовка правой и нижней стороны
		mov rax, r11
		cmp [r9].stk_modify.type_ops, 3				; brd_o3d
		cmovnz r11, r12
		cmovnz r12, rax
		call asm_draw_border
		ret
asm_ssh_border3d endp

asm_ssh_group proc bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
LOCAL _bar:stk_bar
		mov bar, rcx
		mov clip, rdx
		mov dst, r8
		mov modify, r9
		mov [r9].stk_modify.type_ops, 4		; brd_i3d
		call asm_ssh_border3d
		mov r8, dst
		mov r9, modify
		mov [r9].stk_modify.type_ops, 3		; brd_o3d
		mov rdx, bar
		lea rcx, _bar
		; скорректировать область на ширину границы
		movsxd r10, [r9].stk_modify.w_border
		lea rax, [r10 * 2]
		movsxd r10, [rdx].stk_bar.x
		movsxd r11, [rdx].stk_bar.y
		add r10, rax
		add r11, rax
		mov [rcx].stk_bar.x, r10d
		mov [rcx].stk_bar.y, r11d
		shl rax, 1
		movsxd r10, [rdx].stk_bar.w
		movsxd r11, [rdx].stk_bar.h
		sub r10, rax
		sub r11, rax
		mov [rcx].stk_bar.w, r10d
		mov [rcx].stk_bar.h, r11d
		mov rdx, clip
		call asm_ssh_border3d
		ret
asm_ssh_group endp

asm_ssh_table proc bar:QWORD, clip:QWORD, dst:QWORD, modify:QWORD
		ret
asm_ssh_table endp

; (const Range<int>& _wh, ImgMod* modify, void* cells, void* ptr)
asm_ssh_mosaik proc
		ret
asm_ssh_mosaik endp

;rdx(pos) r8(clip)
asm_clip_quad proc private USES r10
		xorps xmm7, xmm7
		mov r10, [r8 + 00]		; xc
		mov r11, [r8 + 08]		; yc
		mov r12, [r8 + 16]		; wc
		mov r13, [r8 + 24]		; hc
		mov rax, [rdx + 00]		; xp
		mov rbx, [rdx + 16]		; wp
		mov rcx, [rdx + 24]		; hp
		mov rdx, [rdx + 08]		; yp
		mov r14, rax
		sub r14, r10
		jge @f
		add rbx, r14
		jle _err
		mov rax, r10
		pinsrd xmm7, r14d, 0
@@:		mov r14, rdx
		sub r14, r11
		jge @f
		add rcx, r14
		jle _err
		mov rdx, r11
		pinsrd xmm7, r14d, 1
@@:		lea r14, [rax + rbx]
		sub r14, r12
		jle @f
		sub rbx, r14
		jle _err
		pinsrd xmm7, r14d, 2
@@:		lea r14, [rdx + rcx]
		sub r14, r13
		jle @f
		sub rcx, r14
		jle _err
		pinsrd xmm7, r14d, 3
@@:		cvtdq2ps xmm7, xmm7
		stc
		ret
_err:	clc
		ret
asm_clip_quad endp

;rcx(bar) rdx(out_bar2) r8(clip)
asm_ssh_get_clipbar proc USES rbx
local tmp:stk_bar
		mov r9, rdx
		call asm_clip_quad
		lea r8, tmp
		test r9, r9
		cmovz r9, r8
		mov [r9].stk_bar.x, eax
		mov [r9].stk_bar.y, edx
		mov [r9].stk_bar.w, ebx
		mov [r9].stk_bar.h, ecx
		mov rax, r9
		ret
asm_ssh_get_clipbar endp
		
;QUAD* asm_ssh_make_quad(const Bar<float>& tex, const Bar<int>& pos, const Bar<int>& clip, const Range<int>& screen, int bgra);
asm_ssh_make_quad proc
		
		ret
asm_ssh_make_quad endp

end
