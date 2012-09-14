///@name        	Power Management.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

#define SCS_Val               0x00000020
#define CLKSRCSEL_Val         0x00000001

#define PLL0CFG_Val           0x00050063

#define PLL1CFG_Val           0x00000023
#define CCLKCFG_Val           0x00000003
#define USBCLKCFG_Val         0x00000000
#define PCLKSEL0_Val          0x00000000
#define PCLKSEL1_Val          0x00000000
#define PCONP_Val             0x042887DE
#define CLKOUTCFG_Val         0x00000000
#define FLASHCFG_Val          0x00004000

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx.h"
//Public variables
PUBLIC uint32_t SW1;		//rising interrupt state
PUBLIC uint32_t SW2;
PUBLIC uint32_t SW3;
PUBLIC uint32_t SWF1;		//falling interrupt state.
PUBLIC uint32_t SWF2;
PUBLIC uint32_t SWF3;
PUBLIC uint32_t SWBT;
PUBLIC uint32_t SWNEAT;
//Private variables

//External variables
EXTERNAL char I2CSlaveBuffer[];

//Private functions

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void fullSpeed (void)
{
                     // Clock Setup
  LPC_SC->SCS       = SCS_Val;
  if (SCS_Val & (1 << 5)) {             // If Main Oscillator is enabled
    while ((LPC_SC->SCS & (1<<6)) == 0);// Wait for Oscillator to be ready
  }
  LPC_SC->CCLKCFG   = CCLKCFG_Val;      //3 Setup Clock Divider
  LPC_SC->PCLKSEL0  = PCLKSEL0_Val;     //0 Peripheral Clock Selection
  LPC_SC->PCLKSEL1  = PCLKSEL1_Val;		//0
  LPC_SC->CLKSRCSEL = CLKSRCSEL_Val;    //1 Select Clock Source for PLL0
  LPC_SC->PLL0CFG   = PLL0CFG_Val;      //0x50063 configure PLL0
  LPC_SC->PLL0FEED  = 0xAA;
  LPC_SC->PLL0FEED  = 0x55;

  LPC_SC->PLL0CON   = 0x01;             // PLL0 Enable
  LPC_SC->PLL0FEED  = 0xAA;
  LPC_SC->PLL0FEED  = 0x55;
  while (!(LPC_SC->PLL0STAT & (1<<26)));// Wait for PLOCK0

  LPC_SC->PLL0CON   = 0x03;             // PLL0 Enable & Connect
  LPC_SC->PLL0FEED  = 0xAA;
  LPC_SC->PLL0FEED  = 0x55;
  while (!(LPC_SC->PLL0STAT & ((1<<25) | (1<<24))));// Wait for PLLC0_STAT & PLLE0_STAT

  LPC_SC->CLKOUTCFG = CLKOUTCFG_Val;    // Clock Output Configuration

  LPC_SC->FLASHCFG  = (LPC_SC->FLASHCFG & ~0x0000F000) | FLASHCFG_Val;

}


/*
PUBLIC void	fullSpeed(void)
{

											// Clock Setup
	  LPC_SC->SCS       = SCS_Val;			//20
	  if (SCS_Val & (1 << 5)) {             // If Main Oscillator is enabled
	    while ((LPC_SC->SCS & (1<<6)) == 0);// Wait for Oscillator to be ready
	  }

	  LPC_SC->CCLKCFG   = CCLKCFG_Val;      //3  Setup Clock Divider

	  LPC_SC->PCLKSEL0  = PCLKSEL0_Val;     //0 Peripheral Clock Selection
	  LPC_SC->PCLKSEL1  = PCLKSEL1_Val;		//0

	  LPC_SC->CLKSRCSEL = CLKSRCSEL_Val;    //1 Select Clock Source for PLL0


	  LPC_SC->PLL0CFG   = PLL0CFG_Val;      // configure PLL0
	  LPC_SC->PLL0FEED  = 0xAA;
	  LPC_SC->PLL0FEED  = 0x55;

	  LPC_SC->PLL0CON   = 0x01;             // PLL0 Enable
	  LPC_SC->PLL0FEED  = 0xAA;
	  LPC_SC->PLL0FEED  = 0x55;
	  while (!(LPC_SC->PLL0STAT & (1<<26)));//Wait for PLOCK0
	  LPC_SC->PLL0CON   = 0x03;             // PLL0 Enable & Connect
	  LPC_SC->PLL0FEED  = 0xAA;
	  LPC_SC->PLL0FEED  = 0x55;
	  while (!(LPC_SC->PLL0STAT & ((1<<25) | (1<<24))));// Wait for PLLC0_STAT & PLLE0_STAT

	  LPC_SC->CLKOUTCFG = CLKOUTCFG_Val;    //0 Clock Output Configuration


                 // Flash Accelerator Setup



}
*/

PUBLIC void lowPower(void)
{
	char a;
	/* Clock Setup                        */




	LPC_SC->PLL0CON = 0x1; // disconnect PLL0
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;
	while (LPC_SC->PLL0STAT&(1<<25));
	LPC_SC->PLL0CON = 0x0;    // power down
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;
	while (LPC_SC->PLL0STAT&(1<<24));


	LPC_SC->PLL1CON = 0x1; // disconnect PLL1
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;
	while (LPC_SC->PLL1STAT&(1<<25));
	LPC_SC->PLL1CON = 0x0;    // power down
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;
	while (LPC_SC->PLL1STAT&(1<<24));

	LPC_SC->FLASHCFG &= 0x0fff;  // This is the default flash read/write setting for IRC
	LPC_SC->FLASHCFG |= 0x5000;

	//LPC_SC->CLKSRCSEL = 0x00;
//		LPC_SC->CCLKCFG = 0x0;     //  Select the IRC as clk
//	LPC_SC->SCS = 0x00;		    // not using XTAL anymore
	LPC_SC->CCLKCFG = 0x1;     //  12MHz/2 =6MHz main clock.



//	LPC_SC->PCONP=0;	//saves 39uA
	a=a;
	enableInputInterrupt();





//	LPC_SC->CCLKCFG = 0x0;   		//IRC as clock.



}


/*
			LED3OFF();
		//	LPC_SC->PCON=1;		//		power down.
	//		SCB->SCR|=0x4;			//deepsleep bit.
			__WFI();
			a=SW1|SW2|SW3;
			b=SWF1|SWF2|SWF3;
*/





void EINT3_IRQHandler(void)				//GPIO interrupt.
{

//interrupts on input change state and bluetooth character.
	char a;
	a=a;

	//get interrupt status.

	SW2=LPC_GPIOINT->IO0IntStatR&0x1<<1;			//SW2 bit 1		EXT
	SW1=(LPC_GPIOINT->IO2IntStatR&0x1<<11)>>11;		//SW1 bit 0 INT
	SW3=(LPC_GPIOINT->IO2IntStatR&0x1<<13)>>11;		//SW3 bit 2 MID
	SWF2=LPC_GPIOINT->IO0IntStatF&0x1<<1;			//SW2 bit 1		EXT
	SWF1=(LPC_GPIOINT->IO2IntStatF&0x1<<11)>>11;	//SW1 bit 0 INT
	SWF3=(LPC_GPIOINT->IO2IntStatF&0x1<<13)>>11;	//SW3 bit 2 MID
	SWBT=(LPC_GPIOINT->IO0IntStatF&0x1<<16)>>16;	//BT
	SWNEAT=(LPC_GPIOINT->IO2IntStatF&0x1<<0)>>0;	//NEAT


	//clear interrupts.
	LPC_GPIOINT->IO0IntClr=0x1<<1|0x1<<16;
	//LPC_GPIOINT->IO0IntClr=0x1<<16;
	LPC_GPIOINT->IO2IntClr=0x1<<0|0x1<<11|0x1<<13;		//NEAT|INT|MID
	//LPC_GPIOINT->IO2IntClr=0x1<<13;	//clear interrupt.


}





PUBLIC void enableInputInterrupt(void)
{
	NVIC->ISER[0]|=0x1<<21;		//enable eint3/GPIO 0/GPIO2 interrupt.
	LPC_GPIOINT->IO0IntEnR|=0x1<<1;				//EXT input rising
	LPC_GPIOINT->IO2IntEnR|=0x1<<11;			//INT input	rising
	LPC_GPIOINT->IO2IntEnR|=0x1<<13;			//MID input rising

	LPC_GPIOINT->IO0IntEnF|=0x1<<1;				//EXT input	falling
	LPC_GPIOINT->IO0IntEnF|=0x1<<16;			//Bluetooth falling
	LPC_GPIOINT->IO2IntEnF|=0x1<<0;				//NEAT falling
	LPC_GPIOINT->IO2IntEnF|=0x1<<11;			//INT input falling
	LPC_GPIOINT->IO2IntEnF|=0x1<<13;			//MID input falling
}

PUBLIC void powerDown(void)
{
	int a;

	LPC_SC->PLL0CON = 0x1; 						// disconnect PLL0//clear bit 1 to 0
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;
	while (LPC_SC->PLL0STAT&(1<<25));
	LPC_SC->PLL0CON = 0x0;    					// power down		//clear bit 0 to 0.
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;
	while (LPC_SC->PLL0STAT&(1<<24));

//	LPC_SC->CCLKCFG = 0x0;    					//  Select the IRC as clk
//	LPC_SC->SCS = 0x00;		    				// not using XTAL anymore


	LED3OFF();
	LPC_SC->PCONP=1<<15;		//only GPIO needed during power down.


///TODO enable power down
	//set to power down, next 2 instructions give power down state.
//	LPC_SC->PCON=1;									//power down option.
//	SCB->SCR|=0x4;									//deepsleep bit.


	__WFI(); //go to power down.


	 LPC_SC->PCONP     = Peripherals ;       // Power Control for Peripherals      */



	 if(I2CBATTERY() >=95)//92=92*8*4.88mV =3.591V, 93=3.63V, 95=3.708V
	 {

		LED3GREEN();//battery good active
	 }
	 else
	 {
		 LED3YELLOW();//battery low active.
	 }





	fullSpeed();





}
