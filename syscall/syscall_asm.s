.section .rodata
str:
	.ascii "hello\n"
str_end:
	.equ len, str_end - str

.section .text
.global main
main:
	pushq %rbp
	movq %rsp,%rbp
	movq $1, %rax
	movq $1, %rdi
	leaq str(%rip), %rsi
	movq $len, %rdx
	syscall

	movq $60, %rax
	movq $0, %rdi
	syscall
