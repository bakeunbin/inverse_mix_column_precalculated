;
; I, Eunbin, am not reliable for anything, but
; this code is free to use.
;

[bits 64]
%ifidn __OUTPUT_FORMAT__, elf64
	%define arg0 rdi
%elifidn __OUTPUT_FORMAT__, win64
	%define arg0 rcx
%else
	%error Unsupported format
%endif
section .text
global check_aes_ni_support
global ni_inv_mix_column
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
ni_inv_mix_column:
	aesimc xmm0,  [arg0]
	movdqu [arg0], xmm0
	ret