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
EXTERNAL uint32_t SWNEAT;
//Local functions
PRIVATE void powerupHEX(void);
PRIVATE void LOOP(void);

//External functions
//EXTERNAL void fullSpeed(void);
EXTERNAL int powerDown(void);


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
	while (1)

	{
		int a;
		LPC_GPIO_OFF FIOCLR = OFF; //SD(shutdown) =OFF button set low to keep on, set high to turn off.
		LPC_GPIO_OFF FIODIR |= OFF; //SD(shutdown) =OFF. //1K pull-down prevents turning on during power up. (3.3mA is OK)
									//set GPIO0_11 to turn off device.
		//set various inputs/outputs to default state.
		LPC_GPIO_IROUT FIOCLR = IROUT; //clear IR output (IR off).
		LPC_GPIO_IROUT FIODIR |= IROUT; //IR defined as an output. Small reduction Iq -190uA


		LPC_GPIO_MICCE FIOCLR = MICCE; //MIC CHIPEN=0=disabled. PORTCHANGE
		LPC_GPIO_MICCE FIODIR |= MICCE; //CHIPEN on mic =output. Small increase Iq +50uA PORTCHANGE

		LPC_GPIO_FLASHCS FIOSET = FLASHCS; //CHIPEN=1=disabled.
		LPC_GPIO_FLASHCS FIODIR |= FLASHCS; //CHIPEN 1 disabled on FLASH.	Small reduction Iq -30uA

		LPC_GPIO_T2G FIOSET = T2G; //T2g=1=disabled.			No effect Iq
		LPC_GPIO_T2G FIODIR |= T2G; //T2Vgs=0, 3v3 on pin.





//		I2CFullCharge();	//reset charge counter to full.(FFFF), note count does not wrap so stays at FFFF if more charge.
//		ms();
	//	 I2CREAD();
	//	 I2CINIT();
	//	 I2CREAD();


			powerupHEX(); //HEX options like factory reset on power up.

			for(;;);

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


	inputChange();

/////////////////////////////////////////////
///main loop.
///processor always comes back here.
///after 3s it goes to sleep in powerDown();
///it wakes up from input, BT or NEAT interrupts.
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	while (1) {
			powerDown();  	//Go to low power state if no comms, and timer2>3s:
			processBT(); 	//process received information.
			rxtxBT(); 		//receive/transmit any BT data//may be possible to DMA here.
	}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
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
	int s,b,c,d,e,f,g,h,i;
	a = HEX();





	switch (a) {
	case 0x0F:
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.
		break;
	case 0x0D:		//test turn off after 1 second.

		LED1GREEN();
	//			while(1)
	//			{
	//			a=readNEAT();
//
	//			}

	//			readNEAT();
				NEATTX(0xFF,0x00,0xAAAA);		//battery state, LARM type, ID(16 bits)

			us(1000000);
			while(1);
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		LPC_GPIO_OFF FIODIR |= OFF; //
		while(1);
		break;
	case 0x0C: //recover clock
		while (1) {
		}
		; //wait for ever at max clock for debug.
		break;


	case 0x0B:
		SystemCoreClockUpdate ();
		s=SystemCoreClock; //4MHz
		CPU100MHz();
		SystemCoreClockUpdate ();
		s=SystemCoreClock; //4MHz

			b=NEATRD(0x10);
			c=NEATRD(0x11);
			d=NEATRD(0x12);
			e=NEATRD(0x13);
			f=NEATRD(0x14);
			g=NEATRD(0x15);
			NEATWR(2,0xA0);			//reset read
			NEATWR(1,0x01);			//reset NEAT module.





		break;
	case 0x0A:
		CPU12MHz();
		initBT();
			setupBT();



		break;


		s=LPC_TIM2->TC;


		if (3000000 < LPC_TIM2->TC)
	case 0x09:
		while(1)
		{
		LPC_TIM2->TC=0;
		 while (2000000>LPC_TIM2->TC);
		 IRsynthesis('P',4,0x2);		//Plessey  4 repeats, code 3

		 LED1OFF();

		}
	case 0x00:
//		timer2Start(); //initiate timer2 to 1MHz.
		CPU12MHz();

		enableInputInterrupt();

		initUART();
		initBT();
		resetBT();
		setupBT();
		LED1YELLOW();
		I2CINIT();
		CPU12MHz();
		NEATRESET();

		LOOP();			//never exits this loop.
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
