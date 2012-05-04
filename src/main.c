///@name        	Main C entry code and main loop.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Includes
#include "HUB.h"
#include <NXP/crp.h>

//Public variables
PUBLIC __CRP const unsigned int CRP_WORD = CRP_NO_CRP;///< code protection word

//Private variables
PRIVATE char Copyright[] = "Copyright Possum 2012, HC8500"; ///< machine readable copyright message.

//External variables
EXTERNAL int Buffer[];  ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL int FlashAddress;	///<This address needs to be set before flash read and flash write.

//Local functions

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief  main is entry point for main code, called from cr_starup_lpc176x.c
/// Called by ResetISR in cr_startup_lpc176x.c (which is the first code to run from the reset of the cpu.)
/// ResetISR address comes from vector table.
///@param void
///@return never returns
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int main(void) {
//


	initSSP0Flash();
	eraseFlash();

	FlashAddress=0;		//address for readingh from flash
//	writePage();

	LED3GREEN();
	readFlashLow();
	readFlashHigh();
	LED3YELLOW();


	initIR();
	captureIR();
	eraseFlash();
	FlashAddress=0;		//address for writing to flash
	writeFlashLow();
	writeFlashHigh();

	FlashAddress=0;		//address for readingh from flash
	readFlashLow();
	readFlashHigh();

	LED3GREEN();
	for(;;);

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



///////////////////////////////////////////////////////////
//TEST section.
//extern int asm_sum(int x, int y);
// Assembler test routine:
//	int j;
//	j = asm_sum(5, 6); //assembler test
