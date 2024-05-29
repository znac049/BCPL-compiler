	.text
				; sn=1, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=0
				; STACK 2
				; sn=2, s1=0, s2=0, s3=9
				; DATALAB 3
				; sn=2, s1=0, s2=0, s3=9
				; ITEML 2
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; JUMP L4
	jmp L4
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; ENTRY 5 1 ...
//	START
L1:
	pop (%ecx)
	mov %ebp,4(%ecx)
	mov %ecx,%ebp
				; sn=2, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=2
				; SAVE 2
				; sn=2, s1=0, s2=1, s3=8
				; LN 3
				; load(4, 3)
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=1, sp=3
	movl $3,8(%ebp)
				; STORE (ignored)
				; sn=2, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=3
				; STACK 5
				; sn=5, s1=0, s2=1, s3=8
				; LSTR 12 '...'
				; load(7, 999)
				; sn=5, s1=0, s2=1, s3=8
				; LG 60
				; load(2, 60)
				; sn=5, s1=1, s2=2, s3=1
				; force(1)
				;  lp=2, sp=7
	movl $L999,%eax
	shr $2,%eax
	movl %eax,20(%ebp)
				; RTAP 3
	leal 12(%ebp),%ecx
	calll *240(%edi)
				; sn=3, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=3
				; STACK 5
				; sn=5, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=5
				; STACK 7
				; sn=7, s1=0, s2=1, s3=8
				; LP 2
				; load(1, 2)
				; sn=7, s1=0, s2=1, s3=8
				; LL 3
				; load(3, 3)
				; sn=7, s1=1, s2=3, s3=1
				; force(1)
				;  lp=2, sp=9
	movl 8(%ebp),%eax
	movl %eax,28(%ebp)
				; FNAP 5
	leal 20(%ebp),%ecx
	calll *L3
				; sn=5, s1=0, s2=1, s3=8
				; LG 62
				; load(2, 62)
				; sn=5, s1=1, s2=2, s3=1
				; force(1)
				;  lp=2, sp=7
	movl %eax,20(%ebp)
				; RTAP 3
	leal 12(%ebp),%ecx
	calll *248(%edi)
				; sn=3, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=3
				; STACK 2
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; FNRN/RTRN
	mov %ebp,%ecx
	mov 4(%ecx),%ebp
	jmp *(%ecx)
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; ENDPROC 0 (ignored)
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; ENTRY 6 2 ...
//	DOUBLE
L2:
	pop (%ecx)
	mov %ebp,4(%ecx)
	mov %ecx,%ebp
				; sn=2, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=2
				; SAVE 4
				; sn=4, s1=0, s2=1, s3=8
				; LP 2
				; load(1, 2)
				; sn=4, s1=0, s2=1, s3=8
				; LP 2
				; load(1, 2)
				; sn=4, s1=2, s2=1, s3=4
				; force(2)
				;  lp=2, sp=6
	movl 8(%ebp),%eax
				; PLUS
	addl 8(%ebp),%eax
				; sn=4, s1=0, s2=1, s3=8
				; LP 3
				; load(1, 3)
				; sn=4, s1=2, s2=1, s3=4
				; force(2)
				;  lp=2, sp=6
				; PLUS
	addl 12(%ebp),%eax
				; sn=4, s1=1, s2=0, s3=3
				; force(1)
				;  lp=1, sp=5
				; RES 5
	jmp L5
				; sn=4, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=4
				; LAB 5
L5:
				; sn=4, s1=0, s2=3, s3=0
				; force(0)
				;  lp=0, sp=4
				; STACK 4
				; sn=4, s1=1, s2=0, s3=3
				; force(1)
				;  lp=1, sp=5
				; FNRN/RTRN
	mov %ebp,%ecx
	mov 4(%ecx),%ebp
	jmp *(%ecx)
				; sn=4, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=4
				; ENDPROC 0 (ignored)
				; sn=4, s1=0, s2=2, s3=0
				; force(0)
				;  lp=0, sp=4
				; STACK 2
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; LAB 4
L4:
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; STORE (ignored)
				; sn=2, s1=0, s2=0, s3=0
				; force(0)
				;  lp=0, sp=2
				; GLOBAL
	ret
	.data
	.align 4
L3:
	.long L2
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
	.equ G1,L1
	.text
