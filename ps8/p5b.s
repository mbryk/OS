	.file	"p5b.c"
	.text
	.globl	empty
	.type	empty, @function
empty:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	empty, .-empty
	.section	.rodata
	.align 8
.LC0:
	.string	"Error Recording Begin Time of Loop"
	.align 8
.LC1:
	.string	"Error Recording End Time of Loop"
.LC2:
	.string	"Execution Time = \t%ld ns\n"
.LC3:
	.string	"Each Iteration = \t%f ns\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$96, %rsp
	movl	$100000000, -68(%rbp)
	leaq	-32(%rbp), %rax
	movq	%rax, %rsi
	movl	$0, %edi
	call	clock_gettime
	cmpl	$-1, %eax
	jne	.L3
	movl	$.LC0, %edi
	call	perror
	movl	$-1, %eax
	jmp	.L10
.L3:
	movl	$0, -72(%rbp)
	jmp	.L5
.L6:
	movl	$0, %eax
	call	empty
	addl	$1, -72(%rbp)
.L5:
	movl	-72(%rbp), %eax
	cmpl	-68(%rbp), %eax
	jl	.L6
	leaq	-16(%rbp), %rax
	movq	%rax, %rsi
	movl	$0, %edi
	call	clock_gettime
	cmpl	$-1, %eax
	jne	.L7
	movl	$.LC1, %edi
	call	perror
	movl	$-1, %eax
	jmp	.L10
.L7:
	movq	-16(%rbp), %rdx
	movq	-32(%rbp), %rax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	movq	%rcx, %rax
	movq	%rax, -64(%rbp)
	movq	-8(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	movq	%rcx, %rax
	movq	%rax, -56(%rbp)
	cmpq	$0, -64(%rbp)
	jle	.L8
	movq	-64(%rbp), %rax
	imulq	$1000000000, %rax, %rdx
	movq	-56(%rbp), %rax
	addq	%rdx, %rax
	jmp	.L9
.L8:
	movq	-56(%rbp), %rax
.L9:
	movq	%rax, -48(%rbp)
	cvtsi2sdq	-48(%rbp), %xmm0
	cvtsi2sd	-68(%rbp), %xmm1
	divsd	%xmm1, %xmm0
	movsd	%xmm0, -40(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, %rsi
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	movq	-40(%rbp), %rax
	movq	%rax, -88(%rbp)
	movsd	-88(%rbp), %xmm0
	movl	$.LC3, %edi
	movl	$1, %eax
	call	printf
	movl	$0, %eax
.L10:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.7.2-2ubuntu1) 4.7.2"
	.section	.note.GNU-stack,"",@progbits
