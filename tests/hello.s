	.text
	# Op=91
	# STACK 2
	# Op=100
	# DATALAB 2
	# Op=101
	# ITEML 1
	# Op=85
	# JUMP 3
	jmp L3
	# Op=94
	# ENTRY 6 1
//	SOMEFN
L1:
	pop (%ecx)
	mov %ebp,4(%ecx)
	mov %ecx,%ebp
	# Op=95
	# SAVE 5
	# Op=42
	# LN 2
	# Op=40
	# LP 2
	# Op=11
	movl $2,%eax
	# MULT/DIV/REM (op=11)
	imull 8(%ebp)
	# Op=92
	movl %eax,20(%ebp)
	# STORE
	# Op=42
	# LN 3
	# Op=40
	# LP 5
	# Op=14
	movl $3,%eax
	# PLUS
	addl 20(%ebp),%eax
	# Op=42
	# LN 42
	# Op=11
	movl $42,%ecx
	# MULT/DIV/REM (op=11)
	imull %ecx
	# Op=98
	# RES
	jmp L4
	# Op=91
	# STACK 5
	# Op=90
	# LAB 4
L4:
	# Op=93
	# RSTACK 5
	# Op=96
	# FNRN
	mov %ebp,%ecx
	mov 4(%ecx),%ebp
	jmp *(%ecx)
	# Op=103
	# ENDPROC 0
	# Op=91
	# STACK 2
	# Op=90
	# LAB 3
L3:
	# Op=92
	# STORE
	# Op=85
	# JUMP 6
	jmp L6
	# Op=94
	# ENTRY 5 5
//	START
L5:
	pop (%ecx)
	mov %ebp,4(%ecx)
	mov %ecx,%ebp
	# Op=95
	# SAVE 2
	# Op=91
	# STACK 4
	# Op=42
	# LN 23
	# Op=42
	# LN 5
	# Op=42
	# LN 62
	movl $23,16(%ebp)
	# Op=44
	# LL 2
	movl $5,20(%ebp)
	# Op=51
	movl $62,24(%ebp)
	# RTAP 2
	leal 8(%ebp),%ecx
	calll *L2
	# Op=91
	# STACK 4
	# Op=43
	# LSTR 12
	# Op=41
	# LG 60
	# Op=51
	movl $L999,%eax
	shr $2,%eax
	movl %eax,16(%ebp)
	# RTAP 2
	leal 8(%ebp),%ecx
	calll *240(%edi)
	# Op=97
	# RTRN
	mov %ebp,%ecx
	mov 4(%ecx),%ebp
	jmp *(%ecx)
	# Op=103
	# ENDPROC 0
	# Op=91
	# STACK 2
	# Op=90
	# LAB 6
L6:
	# Op=92
	# STORE
	# Op=76
	# GLOBAL
	ret
	.data
	.align 4
L2:
	.long L1
	.align 4
L999:
	.byte 12
	.byte 72
	.byte 101
	.byte 108
	.byte 108
	.byte 111
	.byte 44
	.byte 32
	.byte 119
	.byte 111
	.byte 114
	.byte 108
	.byte 100
	.global G1
	.equ G1,L5
	.text
	# Op=0
