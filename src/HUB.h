/*
 * HUB.h
 *
 *  Created on: 4 Apr 2012
 *      Author: Duncan Irvine
 */
///"../linkscripts/HUB.ld"
///
//operators
//bitwise	~	&	|
//logical	!	&&	||
//
//xor ^
//ternary  ? :

//
//NVIC_SystemReset() to reset system.
//
//








#define PCBissue 4		//pcb issue only 4 at the moment. options 3 and earlier all decommissioned.
#define release	 1		//1=released, 0 = debug (disable watchdogs mainly)
#define NEATRX 0		//1=enable NEAT RX, 0=disable NEAT RX
#define NEATREPEATS 8		//NEAT repeats, 3 or 8.
#define NEATBUTTONREPEATS 8	//NEAT repeats when button is pressed with no ACK.


#define baud 11	//92/46/23/11/  96 not implemented yet, copy of 115.2
///
///2_V includes NEAT 8 transmits
///2-V excludes NEAT RX 8 transmits.
///

//2-W changes name of BT to RN42.
//
//2-X corrects bug on IR causing wrap around on IR timer.

//////////////////////////////////
//VERSION NUMBER
#if NEATRX==1
#define Version 0x20303031	//	100, was "2-X " little endian.
#elif NEATRX==0
#define Version 0x20303031	//	100 was "2-X " little endian.
#endif


#define IRreturn	10000		//40us	exit interrupt if wait>40us lower than 25KHz.
#define IRinterrupt	5000		//20us	interrupt if <20us before pulse.

extern int	PCBiss;		//calculated PCB issue (main.c)
//0 means debug, 1 means release.
//note issue 2 is now obsolete and no boards are now in use.

typedef unsigned char byte;		//8 bit on LPC17**
typedef unsigned int word;		//32 bit on LPC17**


//for conditional compile based on PCB issue.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PrescaleIR 1
#define PRIVATE static					//within file only
#define PUBLIC							//accessible outside of file.
#define EXTERNAL extern								//defined outside of file
#define CaptureMax  8192				//8192*4 bytes = 32768 bytes of RAM. Size of Buffer.
#define	CaptureExt	6000				//size of Buffer2
#define Peripherals 1<<1|1<<2|1<<4|1<<8|1<<10|1<<15|1<<19|1<<21|1<<22|1<<23|1<<27|1<<31;        // Power Control for Peripherals      */




#if PCBissue==3	|| PCBissue==4	//issue 4 or issue 3 PCB ONLY issue 4+ is possible. Earlier boards scrapped.
//GPIO pin definitions Issue 4

#define OFF   1<<11						// P0.11
#define LPC_GPIO_OFF 	LPC_GPIO0->

#define IROUT	1<<28					// P1.28
#define LPC_GPIO_IROUT	LPC_GPIO1->

#define IRON	1<<29					// P1.29
#define LPC_GPIO_IRON	LPC_GPIO1->

#define MICCE	1<<8					// P1.8
#define LPC_GPIO_MICCE	LPC_GPIO1->

#define FLASHCS	1<<10					// P1.10
#define LPC_GPIO_FLASHCS LPC_GPIO1->

#define T2G		1<<9					// P2.9
#define LPC_GPIO_T2G LPC_GPIO2->

#define LNR		1<<9					// P1.9
#define LPC_GPIO_LNR LPC_GPIO1->

#define BTMASTER	1<<1				// P1.1
#define LPC_GPIO_BTMASTER LPC_GPIO1->

#define BTRESET		1<<4				// P1.4
#define LPC_GPIO_BTRESET LPC_GPIO1->

#define BTFACTORY	1<<23				// P0.23
#define LPC_GPIO_BTFACTORY LPC_GPIO0->

#define BTDISCOVERY	1<<25				// P0.25
#define LPC_GPIO_BTDISCOVERY LPC_GPIO0->

#define BTDATASTATUS	1<<6			// P2.6
#define LPC_GPIO_BTDATASTATUS LPC_GPIO2->

#define BTCSTATUS		1<<24			// P0.24
#define LPC_GPIO_BTCSTATUS LPC_GPIO0->

#define BTCONNECTED		1<<26			// P0.26
#define LPC_GPIO_BTCONNECTED LPC_GPIO0->

#define NEATCS			1<<21			// P1.21
#define LPC_GPIO_NEATCS LPC_GPIO1->

#define NEATINT			1<<4			// P1.25 MOD to P2.4 pin 68 on PCB.
#define LPC_GPIO_NEATINT LPC_GPIO2->

#define LED1G			1<<16			// P1.16
#define LPC_GPIO_LED1G LPC_GPIO1->

#define LED1Y			1<<17			// P1.17
#define LPC_GPIO_LED1Y LPC_GPIO1->


#define LED2G			1<<14			// P1.14
#define LPC_GPIO_LED1G LPC_GPIO1->

#define LED2Y			1<<15			// P1.15
#define LPC_GPIO_LED1Y LPC_GPIO1->


#define BTCTS			1<<22			// P0.22
#define LPC_GPIO_BTCTS LPC_GPIO0->

#define BTRTS			1<<17			// P0.17
#define LPC_GPIO_BTRTS LPC_GPIO0->

#define BTSPICS			1<<31			//P1.31
#define LPC_GPIO_BTSPICS LPC_GPIO1->

#define ACOK			1<<8			//P2.8
#define LPC_GPIO_ACOK LPC_GPIO2->



#elif PCBissue==2						//issue 2 PCB Issue 1 PCBs were different processor, BC boards, scrapped.
#define OFF  		1<<21				// P0.21
#define LPC_GPIO_OFF 	LPC_GPIO0->

#define IROUT		1<<28				// P1.28
#define LPC_GPIO_IROUT	LPC_GPIO1->

#define MICCE	1<<28					// P4.28
#define LPC_GPIO_MICCE	LPC_GPIO4->

#define FLASHCS	1<<23					// P0.23
#define LPC_GPIO_FLASHCS LPC_GPIO0->

#define T2G		1<<9					// P2.9
#define LPC_GPIO_T2G LPC_GPIO2->

#define LNR		1<<17					// P1.17
#define LPC_GPIO_LNR LPC_GPIO1->

#define BTMASTER	1<<1				// P1.1
#define LPC_GPIO_BTMASTER LPC_GPIO1->

#define BTRESET		1<<4				// P1.4
#define LPC_GPIO_BTRESET LPC_GPIO1->

#define BTFACTORY	1<<0				// P1.0
#define LPC_GPIO_BTFACTORY LPC_GPIO1->

#define BTDISCOVERY	1<<9				// P1.9
#define LPC_GPIO_BTDISCOVERY LPC_GPIO1->

#define BTDATASTATUS	1<<14			// P1.14
#define LPC_GPIO_BTDATASTATUS LPC_GPIO1->

#define BTCONNECTED		1<<10			// P1.10
#define LPC_GPIO_BTCONNECTED LPC_GPIO1->

#define BTCSTATUS		1<<8			// P1.8
#define LPC_GPIO_BTCSTATUS LPC_GPIO1->

#define NEATCS			1<<21			// P1.21
#define LPC_GPIO_NEATCS LPC_GPIO1->

#define NEATINT			1<<0			// P2.0
#define LPC_GPIO_NEATINT LPC_GPIO2->

#define LED1G			1<<19			// P1.19
#define LPC_GPIO_LED1G LPC_GPIO1->

#define LED1Y			1<<22			// P1.22
#define LPC_GPIO_LED1Y LPC_GPIO1->


#define LED2G			1<<0			// P0.0
#define LPC_GPIO_LED1G LPC_GPIO0->

#define LED2Y			1<<11			// P0.11
#define LPC_GPIO_LED1Y LPC_GPIO0->


#define BTCTS			1<<22			// P0.22
#define LPC_GPIO_BTCTS LPC_GPIO0->

#define BTRTS			1<<17			//P0.17
#define LPC_GPIO_BTCTS LPC_GPIO0->

#endif



