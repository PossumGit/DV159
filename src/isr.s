  	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align	8
 	.global	asm_vivo
 	.global asm_holtek
 	.global D
 	.global Delay
 	.thumb
 	.thumb_func


D:
	push	{lr}
 	ldr 	r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr		r6,=0x2009C03C				//@GPIO 1 FIOCLR
	ldr		r4,=1<<28					//IROUT


	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r6]			//clr IR bit IR OFF.
	bl		Delay
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r6]			//clr IR bit IR OFF.
		pop		{lr}
	bx		lr

///////////////////////////
//
//Delay(N) where N>=2 delay in us at 4MHz.
//from C delay = N+1.25us where N>=2.
////////////////////////////////////////////////////////////////
Delay:			//delay N us at 4MHz. Minimum value N=2;
	.align 4
	sub		r0,#1
Delay1:
	subs	r0,r0,1
	cmp	r0,#0
	bne	Delay1
	bx lr


asm_holtek:
	push	{lr}
	ldr		r10,=0x2009C018				//FIO0SET
 	ldr		r11,=0x2009C01C				//FIO0CLR
 	ldr 	r12,=1<<11					//OFF pin set for OFF.
 	ldr		r13,=0x40000008					//WDT FEED.

 	ldr 	r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr		r6,=0x2009C03C				//@GPIO 1 FIOCLR
 	ldr		r2,=0x2009C014				//FIO0PIN
 	ldr		r3,=0x2009C054				//FIO2PIN
	ldr		r4,=1<<28					//IROUT

holtek1:
	str		r12,[r11]					//enable power



	///////////////////////////////

	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H16off
	bl		H32
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		H32off
	bl		H16
	bl		Hsync



holtek2:
	ldr	r8,	[r2]	//FIO0PIN
	ldr	r9,	[r3]	//FIO2PIN
	and	r8,r8,#(1<<21)		//ext normally low
	and	r9,r9,#(1<<11)		//int normally high
	eor	r9,r9,#(1<<11)		//makes int normally low
	add r9,r9,r8
	cmp	r9,#0
	bne	holtek1

	str	r12,[r10]					//disable power


	b	holtek2						//wait for input to be pressed.
	pop		{lr}
 	bx		lr






H16:
	ldr		r8,=16
		.align 4
H16a:


	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.


	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.



	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.

	subs	r8,r8,#1		//1	//branch prediction should work, so no pipline flush delays. But it doesn't work??
	.align 4
	bne		H16a			//3, 1 if continue
	bx		lr

H32:
	ldr		r8,=32
		.align 4
H32a:
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r5]				//set IR bit IR ON.


	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.
	str		r4,[r6]			//clr IR bit IR OFF.

	subs	r8,r8,#1		//1	//branch prediction should work, so no pipline flush delays. But it doesn't work??
	.align 4
	bne		H32a			//3, 1 if continue
	bx		lr


H32off:
.align 4
	ldr	r7,=1111
	.align 4
H32offa:
	subs	r7,r7,1
	bne	H32offa
	bx lr

H16off:

	ldr	r7,=552
	.align 4
H16offa:
	subs	r7,r7,1
	bne	H16offa
	bx lr


Hsync:

	ldr	r7,=19596
	.align 4
Hsynca:
	subs	r7,r7,1
	bne	Hsynca
	bx lr




















 asm_vivo:
 	push	{lr}
//r0=0,1,2,3
 	ldr	r10,=0x2009C018				//FIO0SET
 	ldr	r11,=0x2009C01C				//FIO0CLR
 	ldr r12,=1<<11					//OFF pin set for OFF.

 	ldr r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr	r6,=0x2009C03C				//@GPIO 1 FIOCLR
 	ldr	r2,=0x2009C014				//FIO0PIN
 	ldr	r3,=0x2009C054				//FIO2PIN
	ldr	r4,=1<<28					//IROUT


vivo1:
	str	r12,[r11]					//enable power

	cmp	r0,#0
	bne	try1
 	bl	twelve455KHz		//1
	bl	vivoSync		//1
	bl	twelve455KHz		//1
	bl	vivoOne			//input A pressed
	bl	twelve455KHz
	bl	vivoZero		//input B not pressed.
	bl	twelve455KHz
	bl	vivoZero		//channel bit 0
	bl	twelve455KHz
	bl	vivoZero		//channel bit 1
	bl	twelve455KHz
	bl	vivoZero		//battery OK
	bl	twelve455KHz
	bl	vivoOne			//xor of first 5 bits
	bl	twelve455KHz
	bl	vivoZero		//xnor of first 5 bits.
	bl	twelve455KHz
	bl	vivoSync
	bl	vivoSync		//double sync.
	bl	vivo2
try1:
	cmp	r0,#1
	bne	try2
 	bl	twelve455KHz		//1
	bl	vivoSync		//1
	bl	twelve455KHz		//1
	bl	vivoOne			//input A pressed
	bl	twelve455KHz
	bl	vivoZero		//input B not pressed.
	bl	twelve455KHz
	bl	vivoOne		//channel bit 0
	bl	twelve455KHz
	bl	vivoZero		//channel bit 1
	bl	twelve455KHz
	bl	vivoZero		//battery OK
	bl	twelve455KHz
	bl	vivoZero			//xor of first 5 bits
	bl	twelve455KHz
	bl	vivoOne		//xnor of first 5 bits.
	bl	twelve455KHz
	bl	vivoSync
	bl	vivoSync		//double sync.
	bl	vivo2
try2:
	cmp	r0,#2
	bne	try3
 	bl	twelve455KHz		//1
	bl	vivoSync		//1
	bl	twelve455KHz		//1
	bl	vivoOne			//input A pressed
	bl	twelve455KHz
	bl	vivoZero		//input B not pressed.
	bl	twelve455KHz
	bl	vivoZero		//channel bit 0
	bl	twelve455KHz
	bl	vivoOne		//channel bit 1
	bl	twelve455KHz
	bl	vivoZero		//battery OK
	bl	twelve455KHz
	bl	vivoZero			//xor of first 5 bits
	bl	twelve455KHz
	bl	vivoOne		//xnor of first 5 bits.
	bl	twelve455KHz
	bl	vivoSync
	bl	vivoSync		//double sync.
	bl	vivo2
try3:
 	bl	twelve455KHz		//1
	bl	vivoSync		//1
	bl	twelve455KHz		//1
	bl	vivoOne			//input A pressed
	bl	twelve455KHz
	bl	vivoZero		//input B not pressed.
	bl	twelve455KHz
	bl	vivoOne		//channel bit 0
	bl	twelve455KHz
	bl	vivoOne		//channel bit 1
	bl	twelve455KHz
	bl	vivoZero		//battery OK
	bl	twelve455KHz
	bl	vivoOne			//xor of first 5 bits
	bl	twelve455KHz
	bl	vivoZero		//xnor of first 5 bits.
	bl	twelve455KHz
	bl	vivoSync
	bl	vivoSync		//double sync.





	//check input if pressed, repeat, else OFF

vivo2:
	ldr	r8,	[r2]	//FIO0PIN
	ldr	r9,	[r3]	//FIO2PIN
	and	r8,r8,#(1<<21)		//ext normally low

	and	r9,r9,#(1<<11)		//int normally high
	eor	r9,r9,#(1<<11)		//makes int normally low
	add r9,r9,r8
	cmp	r9,#0
	bne	vivo1
	str	r12,[r10]					//disable power
	b	vivo2		//wait for input to be pressed.
	pop	{lr}
 	bx	lr


vivoZero:		//==0
//delay of 256 full cycles of 455KHz. (includes call+return)
	ldr	r7,=446		//was 585			//2
d256l:
	sub	r7,r7,#1		//1	//branch prediction should work, so no pipline flush delays. But it doesn't work??
	cmp	r7,#0			//1
	bne	d256l			//3, 1 if continue
	bx	lr				//3

vivoOne:		//==1
//delay of 512 full cycles of 455KHz. (includes call+return)
	ldr	r7,=895			//was 1112		//2
d512l:
	sub	r7,r7,#1		//1	//branch prediction should work, so no pipline flush delays. But it doesn't work??
	cmp	r7,#0			//1
	bne	d512l			//3, 1 if continue
	bx	lr				//3


vivoSync:		//==sync
//delay of 768 full cycles of 455KHz. (includes call+return)
	ldr	r7,=1343			//was 1674			//2
	d768l:
	sub	r7,r7,#1		//1	//branch prediction should work, so no pipline flush delays. But it doesn't work??
	cmp	r7,#0			//1
	bne	d768l			//3, 1 if continue
	bx	lr				//3







twelve455KHz:

 //enters with r0, r1 as paramters, returns with r0 as paramter.
//should last for 12 full cycles of 455KHz(includes call+return)

	str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
//	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
//	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit		9
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	10
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	11
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r5]						//set IR bit	12
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
 	bx	lr

