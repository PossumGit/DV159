/*
 ===============================================================================
 Name        : main.c
 Author      : Duncan Irvine
 Version     : test
 Copyright   : Copyright (C) 
 Description : main definition
 ===============================================================================
 */
//MAIN
/////////////////////////////////////////////////////////////////////////////////////////////////
////FUNCTIONS
//main()
//
////PUBLIC FUNCTIONS
//LED3GREEN()	LED3 = GREEN
//LED3YELLOW()	LED3 = YELLOW
//LED3OFF()		LED3 = OFF
//HEX1()	read HEX1
//HEX2()	read HEX2
//
////PUBLIC VARIABLES
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////


#include "HUB.h"
#include "lpc17xx_timer.h"
#include <NXP/crp.h>
#include "lpc17xx.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;
//CODE PROTECTION from writing



//local variables
char Copyright[] = "Copyright Possum 2012, HC8500";


extern int Buffer[];



/////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN.
//Called by ResetISR in cr_startup_lpc176x.c (which is the first code to run from the reset of the cpu.)
//ResetISR address comes from vector table.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
//


	initFlash();
	writeFlash();
	initAudio();
	recordAudio();
	playAudio();



	initIR();

//////////////////////////////////////////
//MAIN LOOP								//
	for (;;) {							//
		captureIR();					//
		playIR();						//
	}									//
//MAIN LOOP								//
//////////////////////////////////////////
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//LEDON
//Turns LED off
//
//DECLARED in HUB.c
////////////////////////////////////////////////////////////////////////////////////////////////
void	LED3GREEN()
	{
	LPC_GPIO1->FIODIR |= 1 << 19; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= 1 << 22; 		//IR defined as an output.
	LPC_GPIO1->FIOCLR = 1 << 22; 		//
	LPC_GPIO1->FIOSET = 1 << 19; 		//LED3	GREEN
	}


/////////////////////////////////////////////////////////////////////////////////////////////////
//YELLOW
//Turns LED to YELLOW
//
//DECLARED in HUB.c
////////////////////////////////////////////////////////////////////////////////////////////////
void	LED3YELLOW()
	{
	LPC_GPIO1->FIODIR |= 1 << 19; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= 1 << 22; 		//IR defined as an output.
	LPC_GPIO1->FIOSET = 1 << 22; 		//
	LPC_GPIO1->FIOCLR = 1 << 19; 		//LED3 YELLOW
	}



/////////////////////////////////////////////////////////////////////////////////////////////////
//LED3OFF
//Turns LED off
//
//DECLARED in HUB.c
////////////////////////////////////////////////////////////////////////////////////////////////
void	LED3OFF()
	{
	LPC_GPIO1->FIOCLR = 1 << 22; 		//
	LPC_GPIO1->FIOCLR = 1 << 19; 		//LED3 OFF
	}



int HEX1()
{
	int a,b,c,d;
	a=(LPC_GPIO0->FIOPIN &1<<18)>>18;
	b=(LPC_GPIO1->FIOPIN &1<<25)>>24;
	c=(LPC_GPIO4->FIOPIN &1<<29)>>27;
	d=(LPC_GPIO1->FIOPIN &1<<27)>>24;
	return 15-(a+b+c+d);

}
int HEX2()
{
	int a,b,c,d;
	a=(LPC_GPIO0->FIOPIN &1<<28)>>28;
	b=(LPC_GPIO0->FIOPIN &1<<27)>>26;
	c=(LPC_GPIO0->FIOPIN &1<<25)>>23;
	d=(LPC_GPIO0->FIOPIN &1<<26)>>23;
	return 15-(a+b+c+d);
}



///////////////////////////////////////////////////////////
//TEST section.
//extern int asm_sum(int x, int y);
// Assembler test routine:
//	int j;
//	j = asm_sum(5, 6); //assembler test
