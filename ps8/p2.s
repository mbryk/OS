.section .rodata
string:
	.ascii "Hello World\n"
.text
.globl	main
main:
	movq $4, %rax
        movq $1, %rbx
        movq $string, %rcx
        movq $12, %rdx
        int $0x80            
        movq $1, %rbx 		#Exit Return Code
        movq $1, %rax
        int $0x80   
