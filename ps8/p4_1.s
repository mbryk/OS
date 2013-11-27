# Mark Bryk OS PS8 P4A.S
.global _start
.text
_start:
        movq $1, %rax           # Write System Call # for x86_64
        movq $5, %rdi		# Invalid FD for this process
        movq $string, %rsi
        movq $12, %rdx
        syscall
        mov %rax, %rdi          #Exit Return Code = Return Val from Write
        movq $60, %rax
        syscall
string:
        .ascii "Hello World\n"
