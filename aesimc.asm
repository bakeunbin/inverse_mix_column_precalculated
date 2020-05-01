; In NASM, compatible with both win64 and elf64
[bits 64]
%ifidn __OUTPUT_FORMAT__, elf64
	%define arg0 rdi
	%define arg1 rsi
	%define arg2 rdx
	%define arg3 rcx
;	%define arg4 r8
;	%define arg5 r9
%elifidn __OUTPUT_FORMAT__, win64
	%define arg0 rcx
	%define arg1 rdx
	%define arg2 r8
	%define arg3 r9
%else
	%error Unsupported bit
%endif
section .text
global check_aes_ni_support
global asm_inv_mix_column
; Check out Intel's AES-NI document.
check_aes_ni_support:
	mov eax, 0x01
	xor ecx, ecx
	cpuid
	xor rax, rax
	test ecx, 0x2000000
	jz .end
	mov rax, 1
.end:
	ret
asm_inv_mix_column:
	aesimc xmm0,  [arg0]
	movdqu [arg0], xmm0
	ret
