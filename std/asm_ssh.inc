

OPTION NOKEYWORD:<width type>
;OPTION PROLOGUE:NONE
;OPTION EPILOGUE:NONE
;OPTION PROLOGUE:PrologueDef
;OPTION EPILOGUE:EpilogueDef

stk_clip struct
	x dq 0
	y dq 0
	w dq 0
	h dq 0
stk_clip ends

bc1 struct
	dw rgb0
	dw rgb1
	dd idx
bc1 ends

bc2 struct
	dd idx1
	dd idx2
	bc bc1<>
bc2 ends

bc3 struct
	db alpha0
	db alpha1
	db alpha_idx0
	db alpha_idx1
	db alpha_idx2
	db alpha_idx3
	db alpha_idx4
	db alpha_idx5
	bc bc1<>
bc3 ends

gif_fmt struct
	nMask	dd 0
	rowDict	dd 0
	nShift	dd 0
	CC		dd 0
	EOI		dd 0
gif_fmt ends

stk_bar struct
	x dd 0
	y dd 0
	w dd 0
	h dd 0
stk_bar ends

stk_vec struct
	x dd 0
	y dd 0
	z dd 0
	w dd 0
stk_vec ends

stk_range struct
	w dd 0
	h dd 0
stk_range ends

stk_modify struct
		type			dd 0				; ��� ������������
		type_ops		dd 0				; ��� ��������
		type_address	dd 0				; ���������
		type_figure		dd 0				; ������
		type_filter		dd 0				; ������
		type_terrain	dd 0				; ��� ���������
		type_histogramm	dd 0				; ��� �����������
		coord			dd 0				; ��� ���������
		src_ops			dd 0				; �������� � ���������
		dst_ops			dd 0
		src_msk			dd 0				; �����
		dst_msk			dd 0
		src_msk_val		dd 0				; �������� �����
		dst_msk_val		dd 0
		sides			dd 0				; ����� �������
		w_border		dd 0				; ������ �������
		radius			dd 0				; ������
		shadow			dd 0				; ����� ����
		aspect			dd 0				; ��������� ����������� � �������
		w_mtx			dd 0				; �������� �������
		scale			dd 0.0				; �������
		alpha			dd 0.0				; �������� �����
		rgba			dq 0				; ������ ��������
		flt_vec			stk_vec<>			; ������ ��� �������
		bar				stk_bar<>			; ������� �������� ������������
		range			stk_range<>			; ��������
		wh				stk_range<>			; ��������
		wh_cells		stk_range<>			; �������� ������
		wh_repeat		stk_range<>			; ���������� ����������
		array_count		stk_range<>			; �������� ��������� � ��������
		cols_histogramm stk_range<>			; ����� ��� ������������ �����������
stk_modify ends

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
externdef asm_clip_bar:near
externdef asm_ssh_bc_x:near
externdef asm_absolute_bar:near

.const
align 16
_gamma		dd 0.3, 0.59, 0.11, 0.0, 0.3, 0.59, 0.11, 0.0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
f_256x8		dd 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0, 256.0
f_100x4		dd 100.0, 100.0, 100.0, 100.0
.data
_clip		stk_clip<>