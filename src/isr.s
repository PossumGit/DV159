 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align	2
 	.global	asm_sum
 	.thumb
 	.thumb_func
 asm_sum:

 	add	r0,r0,r1
 //	push	{r7}
 //	sub	sp, sp, #12
 //	add	r7, sp, #0
// 	str	r0, [r7, #4]
// 	str	r1, [r7, #0]
// 	ldr	r2, [r7, #4]
// 	ldr	r3, [r7, #0]
// 	add	r3, r2, r3
 //	mov	r0, r3
 //	add	r7, r7, #12
// 	mov	sp, r7
 //	pop	{r7}
 	bx	lr

 	/* example.
 	  .text  .global func   @ int func(int a, int b, int c, int d, int e) func:  stmfd sp!, {r4, r5, lr}  @ Saves 3 registers, taking                                12 bytes from the stack   sub sp, sp, 4            @ Maintain 8 byte stack                                alignment   mov r4, r0               @ Save "a" in r4   add r5,r2,r3             @r5 =c+d   str r1, [sp,0]           @ Save "b" on the stack   bl some_other_finc       @ Make a function call   ldr r0, [sp, 0]          @ Load "b" into r0   eorr0,r0,r4              @r0 =a^b   and r0,r0,r5             @r0=(a^b)&(c+d)   ldr r1, [sp, 16]         @ Load "e" into r1                            @ "e" was at [sp,0] upon entry                               to the function, after the                               stack                            @ adjustments it is at [sp, 16]   mulr0,r0,r1              @r0 =((a^b)&(c+d))*e   add sp, sp, 4            @ Prepare to pop registers   ldmfd sp!, {r4, r5, pc}  @ Restore r4 and r5 and return                                to caller with the return                                value in r0   .type func, function   .size func, .-func
 	  .text
 	  .global func
 	  @ int func(int a, int b, int c, int d, int e)
 	func:
 	   stmfd sp!, {r4, r5, lr} @ Saves 3 registers, taking12 bytes from the stack
 	   sub sp, sp, 4            @ Maintain 8 byte stack alignment
 	   mov r4, r0               @ Save "a" in r4
 	   add r5,r2,r3             @r5 =c+d
 	   str r1, [sp,0]           @ Save "b" on the stack
 	   bl some_other_finc       @ Make a function call
 	   ldr r0, [sp, 0]          @ Load "b" into r0
 	   eorr0,r0,r4              @r0 =a^b
 	   and r0,r0,r5             @r0=(a^b)&(c+d)
 	   ldr r1, [sp, 16]         @ Load "e" into r1
 	                            @ "e" was at [sp,0] upon entry to the function, after the stack
 	                            @ adjustments it is at [sp, 16]
 	   mulr0,r0,r1              @r0 =((a^b)&(c+d))*e
 	   add sp, sp, 4            @ Prepare to pop registers
 	   ldmfd sp!, {r4, r5, pc}  @ Restore r4 and r5 and return to caller with the returnvalue in r0
 	   .type func, function
 	   .size func, .-func


 	   */
