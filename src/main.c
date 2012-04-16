/*
 ===============================================================================
 Name        : main.c
 Author      : Duncan Irvine
 Version     : test
 Copyright   : Copyright (C) 
 Description : main definition
 ===============================================================================
*/
#include "HUB.h"
#include "lpc17xx_timer.h"
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;
//CODE PROTECTION from writing




int captureIR(void);
int playIR(void);
void c_entry(void);

extern int asm_sum(int x, int y);
/////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	int j;
	j = asm_sum(5, 6);

	int CaptureTime,PlaySize,i,b;
	float a=0,ClockMHz, CaptureSeconds,IRClockMHz;
	LPC_GPIO1->FIOCLR|=1<<28;		//clear p1.28 output.
	LPC_GPIO1->FIODIR|=1<<28;		// p1.28 is output.




	CaptureTime = captureIR();//a is number of clock cycles of IR captured.






	CaptureSeconds=(float)CaptureTime/100000000;

	PlaySize=playIR();

	for (;;) {


		ClockMHz = SystemCoreClock/1000000;
		IRClockMHz=ClockMHz/PrescaleIR;
		a=a;
	}
}



