

OPTION NOKEYWORD:<width type>
;OPTION PROLOGUE:NONE
;OPTION EPILOGUE:NONE
;OPTION PROLOGUE:PrologueDef
;OPTION EPILOGUE:EpilogueDef

pw typedef ptr word
pd typedef ptr dword

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
		type			dd 0				; тип модификатора
		type_ops		dd 0				; тип операций
		type_address	dd 0				; адресаци€
		type_figure		dd 0				; фигура
		type_filter		dd 0				; фильтр
		type_terrain	dd 0				; тип ландшафта
		type_histogramm	dd 0				; тип гистограммы
		coord			dd 0				; тип координат
		src_ops			dd 0				; операции с пиксел€ми
		dst_ops			dd 0
		src_msk			dd 0				; маски
		dst_msk			dd 0
		src_val			dd 0				; значени€ маски
		dst_val			dd 0
		sides			dd 0				; маска границы
		w_border		dd 0				; ширина границы
		radius			dd 0				; радиус
		shadow			dd 0				; маска тени
		aspect			dd 0				; пропорции изображений в мозаике
		w_mtx			dd 0				; габариты матрицы
		scale			dd 0.0				; масштаб
		alpha			dd 0.0				; значение альфы
		rgba			dq 0, 0, 0			; массив значений(size, is_owner, ptr)
		flt_vec			stk_vec<>			; вектор дл€ фильтра
		bar				stk_bar<>			; область действи€ модификатора
		range			stk_range<>			; диапазон
		wh				stk_range<>			; габариты
		wh_cells		stk_range<>			; габариты €чейки
		wh_repeat		stk_range<>			; количество повторений
		array_count		stk_range<>			; диапазон элементов в массивах
		cols_histogramm stk_range<>			; цвета дл€ формировани€ гистограммы
stk_modify ends

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
externdef asm_clip_bar:near
externdef asm_ssh_bc_x:near
externdef asm_absolute_bar:near
externdef asm_ssh_pow:near

externdef _func_ops:QWORD
externdef _gamma:DWORD
externdef f_255x8:DWORD
externdef f_256x8:DWORD
externdef f_0_5x8:DWORD
externdef f_100x4:DWORD
externdef _clip:stk_bar
externdef flt_max:DWORD
externdef f_3_0x8:DWORD
externdef f_1_0x8:DWORD
externdef f_2_0x8:DWORD
externdef _mm_not:QWORD

externdef malloc:near
externdef free:near
;data segment common
;.data
;align 16

;data ends