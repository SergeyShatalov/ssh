
.data

.code

asm_ssh_to_bgra8 proc wh:QWORD, crop_w:QWORD, dst:QWORD, src:QWORD, fmt:QWORD
		ret
asm_ssh_to_bgra8 endp

asm_ssh_from_bgra8 proc wh:QWORD, dst:QWORD, src:QWORD, fmt:QWORD
		ret
asm_ssh_from_bgra8 endp

asm_ssh_bmp proc buf:QWORD, wh:QWORD, crop_w:DWORD, bmp:QWORD, pal:QWORD, flags:DWORD, bpp:DWORD, fmt:DWORD
		ret
asm_ssh_bmp endp

asm_ssh_tga proc buf:QWORD, wh:QWORD, crop_w:DWORD, tga:QWORD, pal:QWORD, flags:DWORD, bpp:DWORD, fmt:DWORD
		ret
asm_ssh_tga endp

asm_ssh_gif proc crop_w:DWORD, dst:QWORD, pal:QWORD, stk:QWORD, plzw:QWORD, iTrans:DWORD
		ret
asm_ssh_gif endp

asm_ssh_bfs proc
		ret
asm_ssh_bfs endp

;rcx - buf, rdx - bar, r8 - wh
asm_ssh_h_flip proc
		ret
asm_ssh_h_flip endp
		
;rcx - buf, rdx - bar, r8 - wh
asm_ssh_v_flip proc
		ret
asm_ssh_v_flip endp

end
