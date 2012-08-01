///@name        	Main C entry code and main loop.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Includes
#include "HUB.h"
//#include <NXP/crp.h>
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx.h"

//Public variables
//PUBLIC__CRP const unsigned int CRP_WORD = CRP_NO_CRP;///< code protection word

//Private variables
PRIVATE char Copyright[] = "Copyright Possum 2012, HC8500"; ///< machine readable copyright message.
PRIVATE char b = 0; //used for input state memory.

PRIVATE char in[] = { 'I', ' ' };
PRIVATE int ilength = sizeof(in);
//External variables
EXTERNAL int Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL int FlashAddress; ///<This address needs to be set before flash read and flash write.
EXTERNAL uint32_t SW1;
EXTERNAL uint32_t SW2;
EXTERNAL uint32_t SW3;
EXTERNAL uint32_t SWF1;
EXTERNAL uint32_t SWF2;
EXTERNAL uint32_t SWF3;
EXTERNAL uint32_t SWBT;
//Local functions
PRIVATE void powerupHEX(void);
PRIVATE void LOOP(void);
PRIVATE int repeatInput(void);

//External functions
//EXTERNAL void fullSpeed(void);
EXTERNAL void powerDown(void);
EXTERNAL void readNEAT(void);

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
	while (1) {
		LPC_GPIO0->FIOCLR = 1 << 21; //OFF button set low to keep on, set high to turn off.
		LPC_GPIO0->FIODIR |= 1 << 21; //OFF. //1K pull-down prevents turning on during power up. (3.3mA is OK)
									//set GPIO0_21 to turn off device.

		fullSpeed();		//set clock to 100MHz.

		//
		//set various inputs/outputs to default state.
		LPC_GPIO1->FIOCLR |= 1 << 28; //clear IR output (IR off).
		LPC_GPIO1->FIODIR |= 1 << 28; //IR defined as an output. Small reduction Iq -190uA

		LPC_GPIO4->FIOCLR = 1 << 28; //CHIPEN=0=disabled.
		LPC_GPIO4->FIODIR |= 1 << 28; //CHIPEN on mic =output. Small increase Iq +50uA

		LPC_GPIO0->FIOSET = 1 << 23; //CHIPEN=1=disabled.
		LPC_GPIO0->FIODIR |= 1 << 23; //CHIPEN 1 disabled on FLASH.	Small reduction Iq -30uA

		LPC_GPIO2->FIOSET = 1 << 9; //T2g=1=disabled.			No effect Iq
		LPC_GPIO2->FIODIR |= 1 << 9; //T2Vgs=0, 3v3 on pin.




		readNEAT();
		int a;
		timer2Start(); //initiate timer2 to 1MHz.

		initUART();
		initBT();
		 I2CINIT();
//		I2CFullCharge();	//reset charge counter to full.(FFFF), note count does not wrap so stays at FFFF if more charge.
//		ms();
		 I2CREAD();
	//	 I2CINIT();
	//	 I2CREAD();


			powerupHEX(); //HEX options like factory reset on power up.


			//	initInput();

			LOOP(); //main loop, everything controlled form LOOP.
			//return here means reset.
		}
	}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Main loop. Everything comes back here.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void LOOP(void) {

	//check BT, check Input, check for all sent, all rx, if none sleep.

	int a, b,c,r;
	c=0;
	char WAKE[] = { 'W' };

	enableInputInterrupt();
	LED3YELLOW();
	timer2Start();
	while (1) {
		//check if BT empty
		// if  rx is empty and tx is empty then sleep.
		if ((c=BTState(c))>=2) {
			powerDown();
			}

		if (SWBT) {
			sendBT(WAKE, sizeof(WAKE));
		}
		rxtxBT(); //receive/transmit Bluetooth.

		LPC_TIM2->TC = 0; // timer2 used for going back to sleep. Reset gives another 3 seconds.


		while (3000000 > LPC_TIM2->TC)
		{
			r=repeatInput(); //check if change of input, send via BT to android if change.
			r=r+processBT(); //process received information.
			r=r+rxtxBT(); //receive/transmit any BT data//may be possible to DMA here.
			if (r)LPC_TIM2->TC = 0;		//reset timer2.
		}

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief When input changes send new state to BT.
///@param void
///@return 0 if inactive, else 1
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE int repeatInput(void) {
	in[1] = inputChange();
	if (in[1] & 0x80) //bit 7 high indicates change
	{

		in[0] = 'H' | (in[1] & 0x01);
		in[1] = (in[1] & 0x7f) | 0x01; //clear bit 7, set bit 1.

		//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
		sendBT(in, ilength);
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief CHECK HEX switch at power up
///@param void
///@return void
///0F=setup BT
///OE = factory reset bluetooth.
///0C=recover from power problems during debug.
/////////////////////////////////////////////////////////////////////////////////////////////////

PRIVATE void powerupHEX(void) {
	char a;
	int i;
	a = HEX();
	switch (a) {
	case 0x0F: //HEX2=0, HEX1=F  Set up BT then bluetooth discovery.
		setupBT();
//		discoverBT();
		break;
	case 0x0E:
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.
		break;
	case 0x0D:		//test turn off after 1 second.

		LED3GREEN();
		for(i=0;i<1000;i++)
		{
			ms();
		}
		LPC_GPIO0->FIOSET |= 1 << 21; //OFF button set high to turn off.
		LPC_GPIO0->FIODIR |= 1 << 21; //
		while(1);
		break;
	case 0x0C: //recover clock
		while (1) {
		}
		; //wait for ever at max clock for debug.
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
