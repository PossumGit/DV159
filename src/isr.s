  	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align	8
 	.global	asm_vivo
 	.global asm_holtek
 	.global	asm_freq
 	.global D
 	.global Delay
 	.global carrier
 	.global raw
 	.thumb
 	.thumb_func


D:
	push	{R1-R12,lr}					//save all registers
//	push	{lr}
 	ldr 	r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr		r6,=0x2009C03C				//@GPIO 1 FIOCLR
	ldr		r4,=1<<28					//IROUT


	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r6]			//clr IR bit IR OFF.
	bl		Delay
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r6]			//clr IR bit IR OFF.
//		pop		{lr}
	pop	{R1-R12,lr}					//save all registers
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

asm_freq:

	ldr		r10,=0x2009C018				//FIO0SET
 	ldr		r11,=0x2009C01C				//FIO0CLR
 	ldr 	r12,=1<<11					//OFF pin set for OFF.
 	ldr		r13,=0x40000008					//WDT FEED.

 	ldr 	r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr		r6,=0x2009C03C				//@GPIO 1 FIOCLR
 	ldr		r2,=0x2009C014				//FIO0PIN
 	ldr		r3,=0x2009C054				//FIO2PIN
	ldr		r4,=1<<28					//IROUT
asmrep:
	str		r4,[r5]				//set IR bit IR ON.
	str		r4,[r6]				//clr IR bit IR OFF.
	str		r4,[r6]				//clr IR bit IR OFF.
	str		r4,[r6]				//clr IR bit IR OFF.
	str		r4,[r6]				//clr IR bit IR OFF.
	str		r4,[r6]				//clr IR bit IR OFF.
	b	asmrep

asm_holtek:
//	push	{lr}
	push	{R1-R12,lr}					//save all registers
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
 	ldr		r2,=0x2009C014				//FIO0PIN
 	ldr		r3,=0x2009C054				//FIO2PIN

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
	pop	{R1-R12,lr}					//save all registers
//	pop		{lr}
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
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.

	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.

	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.

	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.
	str		r4,[r5]			//clr IR bit IR OFF.

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
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.

	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.

	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.

	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.
	str		r4,[r5]			//set IR bit IR ON.

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


H32offx:
.align 4
	ldr	r7,=1111
	.align 4
H32offax:
	subs	r7,r7,1
	bne	H32offax


	bx lr

H16offx:

	ldr	r7,=552
	.align 4
H16offax:
	subs	r7,r7,1
	bne	H16offax
	bx lr


Hsyncx:

	ldr	r7,=19596
	.align 4
Hsyncax:
	subs	r7,r7,1
	bne	Hsyncax
	bx lr

H16off:
//R0=time
		.align 4
		ldr	r7,=1645	//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
HT0:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	HT0					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
//		cmp	r2,r7				//each cpmp=0.25us.
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr

H32off:
//R0=time
		.align 4
		ldr	r7,=3335	//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
HT1:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	HT1					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
//		cmp	r2,r7				//each cpmp=0.25us.
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr

Hsync:
//R0=time
		.align 4
		//6720 gives 1688us
		//6730 gives 1692us
		ldr	r7,=59390		//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
HTS:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	HTS					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
//		cmp	r2,r7				//each cpmp=0.25us.
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr





















 asm_vivo:
 	.align 4
	push	{R1-R12,lr}					//save all registers
// 	push	{lr}
//r0=0,1,2,3
 	ldr	r10,=0x2009C018				//FIO0SET
 	ldr	r11,=0x2009C01C				//FIO0CLR
 	ldr r12,=1<<11					//OFF pin set for OFF.

 	ldr r5,=0x2009C038				//@GPIO 1 FIOSET
 	ldr	r6,=0x2009C03C				//@GPIO 1 FIOCLR
 	ldr	r2,=0x2009C014				//FIO0PIN !(1<<21))>>17; 	//bit 5	EXTERNAL
 	ldr	r3,=0x2009C054				//FIO2PIN (1<<11))>>6;		//bit 6	INTERNAL
 	//!(1<<12))>>12;	//bit 4	EXTERNAL MID also means mono jack inserted


	ldr	r4,=1<<28					//IROUT


vivo1:
	str	r12,[r11]					//enable power

	cmp	r0,#0
	bne	try1
 	bl	twelve455KHz	//
	bl	vivoSync		//768*1000/455 =1690.11us
	bl	twelve455KHz	//
	bl	vivoOne			//input A pressed	//512*1000/455=1127.47us
	bl	twelve455KHz
	bl	vivoZero		//input B not pressed.	//256*1000/455=564.84us
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
//	bl	V0
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
 	ldr	r2,=0x2009C014				//FIO0PIN !(1<<21))>>17; 	//bit 5	EXTERNAL
 	ldr	r3,=0x2009C054				//FIO2PIN (1<<11))>>6;		//bit 6	INTERNAL
 vivo3:
	ldr	r8,	[r2]	//FIO0PIN
	and	r8,r8,#(1<<21)		//ext normally low

	ldr	r9,	[r3]	//FIO2PIN
//GPIO2 (1<<11):   1= INTERNAL
//GPIO2 !(1<<12):  0=external so use ! makes 1=EXTERNAL
//GPIO0 !(1<<21):  0==mid so use ! makes 1=MID
//IGNORE MID for VIVO remote function.
	and	r9,r9,#(1<<11)		//internal normally high
	eor	r9,r9,#(1<<11)		//makes int normally low
	add r9,r9,r8
	cmp	r9,#0
	bne	vivo1
	str	r12,[r10]					//disable power
	b	vivo3		//wait for input to be pressed.
	pop	{R1-R12,lr}					//save all registers
//	pop	{lr}
 	bx	lr



twelve455KHz:
	.align 4


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

vivo1MHz:
	.align 4
		str	r4,[r5]						//set IR bit	START pulse
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	10
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	20
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	30
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	40
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	50
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	60
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	70
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	80
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	90
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit

		str	r4,[r5]						//set IR bit	1
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	2
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	3
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	4
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	5
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	6
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	7
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	8
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	9
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit
		str	r4,[r5]						//set IR bit	100
	str	r4,[r5]						//set IR bit
	str	r4,[r6]						//clr IR bit
	str	r4,[r6]						//clr IR bit





	bx	lr





vivoZero:
//R0=time
		.align 4
		ldr	r7,=2210	//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
T0:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	T0					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
//		cmp	r2,r7				//each cpmp=0.25us.
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr

vivoOne:
//R0=time
		.align 4
		ldr	r7,=4470	//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
T1:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	T1					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
		cmp	r2,r7				//each cpmp=0.25us.
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr

vivoSync:
//R0=time
		.align 4
		//6720 gives 1688us
		//6730 gives 1692us
		ldr	r7,=6730	//last start cycle to next start cycle
						//count (18+3.5*(#-10)/14))us.
						//#%13 =number of compares to add.
						//approx #=10+ 4(us-18)
						//for 564.75 we get
		ldr R1,=0x40008000		//TIM1->TCR

		ldr	R2,=0x3				//r6,TIM1->TCR		reset TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer
		ldr	R2,=0x1				//r6,TIM1->TCR		restart TIM1
		str r2,[r1,#4]				//TIM1->TCR reset and start Timer


//test for count finished.
	.align 4
TS:
		ldr	r2,[r1,#8]				//T2 TIM1->TC read timer count.
		cmp	r2,r7				//T1 compare timer count match count
		bcc	TS					//T(1 or 1+pipeline) wait until TC>=IRMATCH branch if R4>R3
		cmp	r2,r7				//each cpmp=0.25us.
		cmp	r2,r7
		cmp	r2,r7
		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
//		cmp	r2,r7
///		cmp	r2,r7
//		cmp	r2,r7
		bx	lr








raw:



//r0=IRAddress
//r1=count, max number of pulses = Capturemax-IADress
//r2=return if match time exceeds r2
//return IRAddress of next pulse.

		push	{R1-R12,lr}					//R0 is return, R13=SP, R14=LR, R15=PC save all registers
	//TIM0->EMR 0x4000 403C		//R8
	// TIM0->TCR 0x4000 4004	//R9
	// TIM1->TC 0x4000 8008		//R10
		.align 4

		add	R0,R0,R0				//R0*2
		add	R0,R0,R0				//R0*2	convert to word pointer
		ldr	R5,=0x11			//r5, TIM0->EMR		external pulse until match
		ldr	R8,=0x4000403C		//TIM0->EMR
		ldr	R6,=0x1				//r6,TIM0->TCR		reset and restart TIM0

		ldr R9,=0x40004004		//TIM0->TCR
		ldr R10,=0x40008008		//TIM1->TC  IR counter.
		ldr R11,=Buffer			//Buffer
//		add R12,R12,R0
		add R12,R11,R0			//R12=Buffer+IRAddress

		ldr	R4,[R12],#4			//load IRdata=Buffer[IRAddress++]
		cbz	R4,rawexit			//exit if data=0


RawMatch:
		ldr	r3,[r10]		//read timer count.
		cmp	r4,r3			//compare timer count match count
		bcs	RawMatch		//wait until TC>=IRMATCH branch if R4>R3
//GeneratePulse
		str	r5,[r8]			//r5,[TIM0->EMR] Set up output pulse
		str r6,[r9]			//r6,[TIM0->TCR] Set up output pulse.
//count pulses
		subs	r1,r1,#1			// count pulses
		beq		rawexit	//repeat	//exit if used up count of pulses
//set up next pulse
		ldr		R4,[R12],#4				//load IRdata=Buffer[IRAddress++]
		cbz		R4,rawexit				//exit if IRData==0
//if R4-R3< R2 loop, else exit.
		sub		R3,R4,R3			//R11=IRData-Timer.
		cmp		R2,R3				//40us
		bcs		RawMatch			//branch if R3>=R2 time to next pulse < IRreturn

//exit if IRData is not raw or is 0 or count done or IRTimeMatch>IRreturn.
rawexit:
	sub	R12,R12,#4					//R12 is used as pointer, correct to IRAddress
	sub	R0,R12,R11					//remove initial R0
//	sub	R0,R12,R0
	lsr	R0,R0,#2					//correct from byte pointer to word pointer (/4)
	pop	{R1-R12,lr}					//save all registers
	bx lr							//return




carrier:

//r0=0,1,2,3 etc =count
//r1=100, 200 etc=period
//r2 is IRTIMEMATCH
	push	{R1-R12,lr}					//save all registers
//	ldr		r2,=0x2009C014				//FIO0PIN
//	ldr	r8,	[r2]	//FIO0PIN
///	and	r8,r8,#(1<<21)		//ext normally low
//	cmp	r8,#0
//	bne	carrier1

	//		while(IRTimeMatch>LPC_TIM1->TC);    //wait for next time match
	//						LPC_TIM0->EMR = 1 | 1 << 4; //Set P1.28, clear P1.28 MR0 on match generate IR PULSE
	//						LPC_TIM0->TCR = 1 | 0 << 1; //reset timer0 and start timer0// bit 1 has to be cleared for counting.
	//						IRTimeMatch +=Period;
	.align 4
	//TIM0->EMR 0x4000 403C		//R8
	// TIM0->TCR 0x4000 4004	//R9
	// TIM1->TC 0x4000 8008		//R10
		ldr	R5,=0x11
		ldr	R8,=0x4000403C
		ldr	R6,=0x1
		ldr R9,=0x40004004
		ldr R10,=0x40008008
		cmp	R0,#0
		beq	NoPulse

WaitMatch:
		ldr	r3,[r10]		//read timer count.
		cmp	r2,r3			//compare timer count match count
		bpl	WaitMatch		//wait until TC>=IRMATCH
		str	r5,[r8]			//r5,[TIM0->EMR] Set up output pulse
		str r6,[r9]			//r6,[TIM0->TCR] Set up output pulse.
		add	r2,r2,r1		//set up next timer match
		subs	r0,r0,#1			//count pulses
		bne		WaitMatch	//repeat
NoPulse:
	pop	{R1-R12,lr}					//save all registers

		bx	lr


