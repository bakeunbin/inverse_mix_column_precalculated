;
; I, Eunbin, am not reliable for anything, but
; this code is free to use.
;

[bits 64]
%ifidn __OUTPUT_FORMAT__, elf64
	%define arg0 rdi
	%define arg1 rsi
	%define arg2 rdx
	%define arg3 rcx
%elifidn __OUTPUT_FORMAT__, win64
	%define arg0 rcx
	%define arg1 rdx
	%define arg2 r8
	%define arg3 r9
%endif
section .text
global check_aes_ni_support
global aesni_keygen
global full_aesni
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
aesni_keygen:
	push rbp
	mov rbp, rsp
	sub rsp, 176
	lea rcx, [rsp]
	movdqu xmm0, [arg0]
	movdqu [rcx], xmm0
	add rcx, 10h
	aeskeygenassist xmm1, xmm0, 01h
	call helper128
	aeskeygenassist xmm1, xmm0, 02h
	call helper128
	aeskeygenassist xmm1, xmm0, 04h
	call helper128
	aeskeygenassist xmm1, xmm0, 08h
	call helper128
	aeskeygenassist xmm1, xmm0, 10h
	call helper128
	aeskeygenassist xmm1, xmm0, 20h
	call helper128
	aeskeygenassist xmm1, xmm0, 40h
	call helper128
	aeskeygenassist xmm1, xmm0, 80h
	call helper128
	aeskeygenassist xmm1, xmm0, 1bh
	call helper128
	aeskeygenassist xmm1, xmm0, 36h
	call helper128
	sub rcx, 176
	movdqu xmm0,  [rcx + 0x00]
	aesimc xmm1,  [rcx + 0x10]
	aesimc xmm2,  [rcx + 0x20]
	aesimc xmm3,  [rcx + 0x30]
	aesimc xmm4,  [rcx + 0x40]
	aesimc xmm5,  [rcx + 0x50]
	aesimc xmm6,  [rcx + 0x60]
	aesimc xmm7,  [rcx + 0x70]
	aesimc xmm8,  [rcx + 0x80]
	aesimc xmm9,  [rcx + 0x90]
	movdqu xmm10, [rcx + 0xa0]
	movdqu [arg1 + 0xa0], xmm0
	movdqu [arg1 + 0x90], xmm1
	movdqu [arg1 + 0x80], xmm2
	movdqu [arg1 + 0x70], xmm3
	movdqu [arg1 + 0x60], xmm4
	movdqu [arg1 + 0x50], xmm5
	movdqu [arg1 + 0x40], xmm6
	movdqu [arg1 + 0x30], xmm7
	movdqu [arg1 + 0x20], xmm8
	movdqu [arg1 + 0x10], xmm9
	movdqu [arg1 + 0x00], xmm10
	mov rsp, rbp
	pop rbp
	ret
helper128:
	pshufd xmm1, xmm1, 0xff
	vpslldq xmm2, xmm0, 04h
	pxor xmm0, xmm2
	vpslldq xmm2, xmm0, 04h
	pxor xmm0, xmm2
	vpslldq xmm2, xmm0, 04h
	pxor xmm0, xmm2
	pxor xmm0, xmm1
	movdqu [rcx], xmm0
	add rcx, 10h
	ret
full_aesni:
	push rbp
	mov rbp, rsp
	movdqu xmm15, [arg0]
	movdqu xmm0,  [arg1 + 0x00]
	movdqu xmm1,  [arg1 + 0x10]
	movdqu xmm2,  [arg1 + 0x20]
	movdqu xmm3,  [arg1 + 0x30]
	movdqu xmm4,  [arg1 + 0x40]
	movdqu xmm5,  [arg1 + 0x50]
	movdqu xmm6,  [arg1 + 0x60]
	movdqu xmm7,  [arg1 + 0x70]
	movdqu xmm8,  [arg1 + 0x80]
	movdqu xmm9,  [arg1 + 0x90]
	movdqu xmm10, [arg1 + 0xa0]
	pxor xmm15, xmm0
	aesdec xmm15, xmm1
	aesdec xmm15, xmm2
	aesdec xmm15, xmm3
	aesdec xmm15, xmm4
	aesdec xmm15, xmm5
	aesdec xmm15, xmm6
	aesdec xmm15, xmm7
	aesdec xmm15, xmm8
	aesdec xmm15, xmm9
	aesdeclast xmm15, xmm10
	movdqu [arg0], xmm15
	mov rsp, rbp
	pop rbp
	ret