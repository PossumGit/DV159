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
PRIVATE char b=0;											//used for input state memory.
PRIVATE int		ilength;
PRIVATE char    in[]= {'I',' '};
//External variables
EXTERNAL int Buffer[];  ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL int FlashAddress;	///<This address needs to be set before flash read and flash write.

//Local functions
PRIVATE void	powerupHEX(void);
PRIVATE void	LOOP(void);
PRIVATE void 	repeatInput(void);
PRIVATE void 	initInput(void);
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

	powerupHEX();			//HEX option like factory reset on power up.
	initBT();
	initUART();
	initInput();
	LOOP();					//main loop, everything controlled form LOOP.








	for(;;);


	initI2C();
	for(;;)
		{

		}

	initAudio();
	recordAudio();
	playAudio();

	for(;;);

	initSSP0Flash();
	eraseFlash();

	FlashAddress=0;		//address for reading from flash
//	writePage();

	LED3GREEN();
	readFlashLow();
	readFlashHigh();
	LED3YELLOW();


//	initIR();

	for(;;)
	{
//	captureIR();
//	playIR();
	}

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



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Main loop. Everything comes back here.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void	LOOP(void)
{


while(1)
	{
	repeatInput();				//check if change of input, send via BT to android if change.
	rxtxBT();					//receive/transmit any BT data//may be possible to DMA here.
	processBT();				//process received information.
	}
}







PRIVATE void repeatInput(void)
{
	char a;
	a=INPUT();
	if(a!=b){
	in[1]=a;
	sendBT(in,ilength);
	b=a;
	}
}


PRIVATE void initInput(void)
{
	b=INPUT();
	ilength=sizeof(in);
}





PRIVATE void	powerupHEX(void)
	{char a;
	a=HEX();

	switch (HEX())
	{
case 0x0E:
	factoryBT();	//HEX2=0, HEX1=E Factory reset BT.
	break;
case 0x0F:			//HEX2=0, HEX1=F  bluetooth discovery.
	discoverBT();
	break;
default:
	break;
	}
	}

///////////////////////////////////////////////////////////
//TEST section.
//extern int asm_sum(int x, int y);
// Assembler test routine:
//	int j;
//	j = asm_sum(5, 6); //assembler test
