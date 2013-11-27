# Mark Bryk OS PS8 P3.S
.global _start
.text
_start:
	movq $1, %rax		# Write System Call # for x86_64
        movq $1, %rdi
        movq $string, %rsi
        movq $12, %rdx
	syscall
        mov %rax, %rdi 		#Exit Return Code = Return Val from Write
        movq $60, %rax
        syscall
string:
	.ascii "Hello World\n"
