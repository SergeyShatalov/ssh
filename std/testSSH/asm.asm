
.data
align 16
vm1		dd 1.0, 2.0, 3.0, 4.0
vm2		dd 5.0, 6.0, 7.0, 8.0
vm3		dd 10.0, 20.0, 30.0, 40.0
v0		dd -1, 255, 255, 255
v1		db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,16,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
v2		db 32 dup(20)
_pp		db 0, 3, 2, 1, 4, 7, 6, 5
_pos	dd 0402h
_ext_b1	dd 00000000000000000000000000011111b
_ext_b2	dd 00000000000000000100011111100000b
_ext_b3	dd 00000000000000001111100000000000b
_dep_b1 dd 00000000000000000000000011111111b
_dep_b2 dd 00000000000000001111111100000000b
_dep_b3 dd 00000000111111110000000000000000b
.code

asm_ssh_shufb proc public
		mov rax, 255
		mov rcx, 160
		shr rax, 4
		shr rcx, 4
		shl cl, 4
		or al, cl
		movaps xmm1, vm1
		movaps xmm2, vm2
		movaps xmm3, vm3
		VFMsubADD231Ps xmm1, xmm2, xmm3
		ret
		movq mm0, qword ptr v1
		pavgb mm0, mm0
		mov ecx, dword ptr [v0]
		pext eax, ecx, _ext_b1
		pext edx, ecx, _ext_b2
		pext ebx, ecx, _ext_b3
		pdep eax, edx, _dep_b2
		pdep eax, eax, _dep_b1
		pdep ebx, ebx, _dep_b3
		movzx rcx, word ptr _pos
		mov edx, [v0]
		bextr eax, edx, ecx
		vpmovzxbd ymm0, dword ptr [v1]
		vcvtdq2ps ymm0, ymm0
		vdpps ymm0, ymm0, ymm0, 01111111b
		vcvtps2dq ymm0, ymm0
		vpackusdw ymm0, ymm0, ymm0
		vpackuswb ymm0, ymm0, ymm0
		mov rdx, offset v1
		movd dword ptr [v1], xmm0
		vextracti128 xmm0, ymm0, 1
		movd ecx, xmm0
		vmovaps ymm0, ymmword ptr v1
		vmovaps ymm1, ymmword ptr v2
		vpshufb ymm0, ymm0, ymm1
		vdpps ymm0, ymm0, ymm1, 01110111b
		movd mm0, rcx
		movq mm1, qword ptr _pp
		pshufb mm0, mm1
		ret
asm_ssh_shufb endp

end
