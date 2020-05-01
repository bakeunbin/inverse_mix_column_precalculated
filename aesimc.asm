;
; MIT License
;
; Copyright (c) 2020 Eunbin Bak
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;

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
