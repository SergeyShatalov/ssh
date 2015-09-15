

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

stk_range struct
	w dd 0
	h dd 0
stk_range ends

;rcx(bar), rdx(clip) r8(dst) -> out rcx(width) rdx(height) r10(pitch) r8(dst)
externdef asm_clip_bar:near
externdef asm_ssh_bc_x:near
;asm_clip_bar proto bar:QWORD, clip:QWORD, dst:QWORD, stk:QWORD

.const
align 16
_gamma		dd 0.3, 0.59, 0.11, 0.0, 0.3, 0.59, 0.11, 0.0
f_255x8		dd 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0

.data?
_clip		stk_clip<>
