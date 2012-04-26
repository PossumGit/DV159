 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align	2
 	.global	asm_sum
 	.thumb
 	.thumb_func
 asm_sum:
 	add	r0,r0,r1
 	bx	lr

