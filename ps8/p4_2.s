.global _start
.text
_start:
        movq $1, %rax           # Write System Call # for x86_64
        movq $1, %rdi
        movq $string, %rsi
        movq $12, %rdx
        syscall
        mov %rax, %rdi          #Exit Return Code = Return Val from Write
        movq $41, %rax		# Bad System Call # (SYS_SOCKET)
        syscall
string:
        .ascii "Hello World\n"
