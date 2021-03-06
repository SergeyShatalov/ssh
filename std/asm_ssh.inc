

OPTION NOKEYWORD:<width type>
;OPTION PROLOGUE:NONE
;OPTION EPILOGUE:NONE
;OPTION PROLOGUE:PrologueDef
;OPTION EPILOGUE:EpilogueDef

epilog_none	equ OPTION EPILOGUE:NONE
epilog_def	equ OPTION EPILOGUE:EPILOGUEDEF

floor macro p1
	vroundss p1, p1, 01b
endm

ceil macro p1
	vroundss p1, p1, 10b
endm

trunc macro p1
	vroundss p1, p1, 11b
endm

frac macro p1, p2
	vroundss p2, p1, 11b
	vsubss p1, p1, p2
endm

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
	x dd ?
	y dd ?
	w dd ?
	h dd ?
stk_bar ends

stk_vec struct
	x dd ?
	y dd ?
	z dd ?
	w dd ?
stk_vec ends

stk_range struct
	w dd ?
	h dd ?
stk_range ends

stk_buf struct
	sz  dq ?
	own dq ?
	buf dq ?
stk_buf ends

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
		src_val			dd 0				; �������� �����
		dst_val			dd 0
		sides			dd 0				; ����� �������
		w_border		dd 0				; ������ �������
		radius			dd 0				; ������
		shadow			dd 0				; ����� ����
		aspect			dd 0				; ��������� ����������� � �������
		w_mtx			dd 0				; �������� �������
		scale			dd 0.0				; �������
		alpha			dd 0.0				; �������� �����
		rgba			stk_buf<>			; ������ ��������(size, is_owner, ptr)
		flt_vec			stk_vec<>			; ������ ��� �������
		bar				stk_bar<>			; ������� �������� ������������
		wh				stk_range<>			; ��������
		wh_cells		stk_range<>			; �������� ������
		wh_repeat		stk_range<>			; ���������� ����������
		wh_rgba			stk_range<>			; �������� ��������� � ��������
		cols_histogramm stk_range<>			; ����� ��� ������������ �����������
stk_modify ends

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
externdef asm_clip_bar:near
externdef asm_ssh_bc_x:near
externdef asm_absolute_bar:near
externdef asm_ssh_pow:near
externdef _add:near
externdef _sub:near

externdef _func_ops:QWORD
externdef _gamma:DWORD
externdef f_255x8:DWORD
externdef f_256x8:DWORD
externdef f_inv255x8:DWORD
externdef f_inv256x8:DWORD
externdef f_0_5x8:DWORD
externdef f_100x4:DWORD
externdef _clip:stk_bar
externdef flt_max:DWORD
externdef f_3_0x8:DWORD
externdef f_1_0x8:DWORD
externdef f_2_0x8:DWORD
externdef _mm_not:DWORD
externdef _mm_gamma:WORD
externdef _alpha_not:WORD
externdef _mm_alpha:QWORD

externdef malloc:near
externdef free:near
;data segment common
;.data
;align 16

;data ends