	.file	"linklayer.c"
	.text
	.section	.rodata
.LC0:
	.string	"tcgetattr"
	.text
	.globl	llopen
	.type	llopen, @function
llopen:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$160, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	-64(%rbp), %rax
	leaq	16(%rbp), %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movl	68(%rbp), %eax
	movl	%eax, -148(%rbp)
	movl	72(%rbp), %eax
	movl	%eax, -144(%rbp)
	movl	76(%rbp), %eax
	movl	%eax, -140(%rbp)
	movl	80(%rbp), %eax
	movl	%eax, -136(%rbp)
	leaq	-64(%rbp), %rax
	movl	$258, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	open@PLT
	movl	%eax, -132(%rbp)
	cmpl	$0, -132(%rbp)
	jns	.L2
	leaq	-64(%rbp), %rax
	movq	%rax, %rdi
	call	perror@PLT
	movl	$-1, %edi
	call	exit@PLT
.L2:
	leaq	-128(%rbp), %rdx
	movl	-132(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	tcgetattr@PLT
	cmpl	$-1, %eax
	jne	.L3
	leaq	.LC0(%rip), %rdi
	call	perror@PLT
	movl	$-1, %edi
	call	exit@PLT
.L3:
	movl	$1, %eax
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L5
	call	__stack_chk_fail@PLT
.L5:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	llopen, .-llopen
	.globl	llwrite
	.type	llwrite, @function
llwrite:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	$1, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	llwrite, .-llwrite
	.globl	llread
	.type	llread, @function
llread:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movl	$1, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	llread, .-llread
	.globl	llclose
	.type	llclose, @function
llclose:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	$1, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	llclose, .-llclose
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
