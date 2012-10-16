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


#if PCBissue==3
	LPC_GPIO1->FIOCLR |=1<<29			;//IR capture state=0.
	LPC_GPIO1->FIODIR |=1<<29			;//IR capture =output.

#elif PCBissue==2

#endif



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
	LED1OFF();

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
	int s,b,c,d,e,f,g,h,i,j,k,l;
	unsigned int time;




	while(1)
	{
	a = HEX();
	switch (a) {

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

	case 0x01:
		CPU12MHz();
		while(1)
		{
			captureIR();
			playIR();
			if(1!=HEX())break;
		}
		break;


	case 0x02:			//NEAT transmit, inc ID every 10s.
		CPU12MHz();
		LPC_TIM2->TC=0;
		time=0;			//
		NEATRESET();
		while(1)
		for (l=0;l<10;(l++))
		for (k=0;k<10;k++)
		for (j=0;j<10;j++)
		for (i=0;i<10;i++)
		{
			h=i+(j<<4)+(k<<8)+(l<<12);
		NEATTX(0xFF,0x00,h);		//battery state, ALARM type, ID(16 bits)
		time=time+10000000;			//+10s
		while (LPC_TIM2->TC<time);
		if(2!=HEX())break;

		}
		break;
	case 0x03:			//NEAT rx, inc ID then transmit.
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


			while(1)
			{
			powerDown();
			if(SWNEAT)
			{
			SWNEAT=0;
			CPU12MHz();
			i=NEATRD(0x61);
			j=NEATRD(0x60);
			NEATWR(2,0xA0);			//reset read
			NEATRESET();
			us(3000000);
			NEATTX(0xFF,0x00,1+i+256*j);		//battery state, ALARM type, ID(16 bits)
			LPC_TIM2->TC =0;
			if(3!=HEX())break;
			}
			}

		break;


	case 04:
		CPU4MHz();
		while(1)
		{	if(4!=HEX())break;
			LED1GREEN();
			us(400000);
			a=inputChange();
			if (~a&(1<<4))
			{
				LED1YELLOW();
			}
			else if (~a&(1<<5))
			{	LED1GREEN();
			}
			else
			{
					LED1OFF();
			}
			us(400000);

		}
		break;
	case 05:
		CPU12MHz();
		while(1)
		{

			if(5!=HEX())break;
			LED1GREEN();
			us(300000);
				a=inputChange();
				if (~a&(1<<4))
				{
					LED1YELLOW();
				}
				else if (~a&(1<<5))
				{	LED1GREEN();
				}
				else
				{
						LED1OFF();
				}
				us(300000);
		}
		break;
	case 06:				//emc test 199MHz
		CPU100MHz();
		while(1)
		{

			if(6!=HEX())break;
			LED1GREEN();
			us(200000);
				a=inputChange();
				if (~a&(1<<4))
				{
					LED1YELLOW();
				}
				else if (~a&(1<<5))
				{	LED1GREEN();
				}
				else
				{
						LED1OFF();
				}
				us(200000);
		}
		break;
	case 07:				//test IR synthesis
		CPU4MHz();
		while(1)
		{
		 LPC_TIM2->TC=0;
		 while (5000000>LPC_TIM2->TC);
		 IRsynthesis('P',2,0x2);		//Plessey  2 repeats, code 3 for HC603c
		 if(7!=HEX())break;
		}
		break;


	case 0x08:		//test turn off after 2 seconds.
		CPU4MHz();
		LED1GREEN();
		us(2000000);
		a=3;
		a=4;
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		while(1);
		break;

	case 0x0B:					//USB.
		break;

	case 0x0C: //recover clock
		while (1) {
			LED1GREEN();

			LED1GREEN();

			LED1GREEN();
			LED1YELLOW();
			LED1GREEN();

			LED1GREEN();

		}
		; //wait for ever for debug.
		break;


	case 0x0F:
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.
		while(1);
		break;

	default:
		break;
	}
}
}

///////////////////////////////////////////////////////////
//TEST section.
//extern int asm_sum(int x, int y);
// Assembler test routine:
//	int j;
//	j = asm_sum(5, 6); //assembler test
