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
PUBLIC volatile word SW1;		//rising interrupt state
PUBLIC volatile word SW2;
PUBLIC volatile word SW3;
PUBLIC volatile word SWF1;		//falling interrupt state.
PUBLIC volatile word SWF2;
PUBLIC volatile word SWF3;
PUBLIC volatile word SWBT;
PUBLIC volatile word SWNEAT;
PUBLIC volatile word ACON;
PUBLIC volatile word ACOFF;
PUBLIC volatile byte PENDALARM=0;
PUBLIC volatile int CPUSPEED=0;
PUBLIC int batterygood=1;
PUBLIC int debounce=0;
PUBLIC int storedcharge=0x8000;
PUBLIC int charge;
PUBLIC int ChargeConfidence=3;
PUBLIC int NIMH=1;
PUBLIC int LITHIUM=0;


//Private variables
static int t=0;


//External variables

EXTERNAL int txstart;
EXTERNAL int txend;
EXTERNAL int rxstart;
EXTERNAL int rxend;
EXTERNAL int ALARMtime;
EXTERNAL int BTACC;
EXTERNAL char STATE;
EXTERNAL int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.


//Private functions
PRIVATE void LEDFLASH(void);
PUBLIC void ACSTATE(void);
//public functions
PUBLIC void CPU4MHz(void);
PUBLIC void CPU12MHz(void);
PUBLIC void CPU100MHz (void);
PUBLIC void disableInputInterrupt(void);
PUBLIC void enableInputInterrupt(void);
PUBLIC int powerDown(void);
PUBLIC void BatteryState();
//External functions
EXTERNAL byte I2CSlaveBuffer[];
EXTERNAL void timer2CPU4(void);
EXTERNAL void readNEAT(void);
EXTERNAL void NEATTX(byte, byte, word);
EXTERNAL int repeatInput(void);
EXTERNAL void	LED1GREEN(void);
EXTERNAL void	LED1YELLOW(void);
EXTERNAL void	LED1OFF(void);
EXTERNAL void	LED2GREEN(void);
EXTERNAL void	LED2YELLOW(void);
EXTERNAL void	LED2OFF(void);
EXTERNAL void timer2CPU4(void);
EXTERNAL void timer2CPU12(void);
EXTERNAL void timer2CPU100(void);
EXTERNAL void SSPNEATCPU4(void);
EXTERNAL void SSPNEATCPU12(void);
EXTERNAL void SSPNEATCPU100(void);
EXTERNAL void  DisableWDT(void);
EXTERNAL void  EnableWDT10s(void);
//EXTERNAL void BatteryState(void);
EXTERNAL void NEATALARM(void);
EXTERNAL int I2CBATTERY(void);
EXTERNAL byte	inputChange(void);
EXTERNAL void SystemOFF(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Change CPU to 100MHZ PLL from 12MHz xtal clock.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void CPU100MHz (void)
{

//If already at 100MHz, fullspeed do nothing.
	  if (((LPC_SC->PLL0STAT >> 24) & 3) != 3)		//PLL not connected and enabled.
	  {


			disableInputInterrupt();
                     // Clock Setup
  LPC_SC->SCS       = 1<<5;	//enable main oscillator
             // If Main Oscillator is enabled
    while ((LPC_SC->SCS & (1<<6)) == 0);// Wait for Oscillator to be ready

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
  //CPU100MHz disables GPIO interrupts
	  }

	  timer2CPU100();		//generate 1MHz system clock for sleep and delays from 100MHz cpu clock.
	  SSPNEATCPU100();		//SSP 1MHz derived from 100MHz for NEAT SSP.
	  CPUSPEED=100;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Change CPU to 12MHZ xtal clock.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC void CPU12MHz(void)
{

	/* Clock Setup                        */

	  if (((LPC_SC->PLL0STAT >> 24) & 3) == 3)		//PLL  connected and enabled.
	  {

//if already at 12MHz, do nothing.
			disableInputInterrupt();

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
	LPC_SC->FLASHCFG |= 0x5000;	//0 for up to 20MHz.

	//LPC_SC->CLKSRCSEL = 0x00;
//		LPC_SC->CCLKCFG = 0x0;     //  Select the IRC as clk
//	LPC_SC->SCS = 0x00;		    // not using XTAL anymore
	LPC_SC->CCLKCFG = 0x0;     //  12MHz main clock.



//	LPC_SC->PCONP=0;	//saves 39uA
	enableInputInterrupt();

	  }

	  if(((1<<5)&(LPC_SC->SCS))==0)
	  {
	  LPC_SC->SCS       = 1<<5;	//enable main oscillator
	             // If Main Oscillator is enabled
	    while ((LPC_SC->SCS & (1<<6)) == 0);// Wait for Oscillator to be ready
	  }
	   LPC_SC->CLKSRCSEL = 0x01; //IRC 4MHzmain oscillator.
	  timer2CPU12();		//12MHz clock, generate 1MHz system clock for sleep and delays from 12MHz CPU clock.
	  SSPNEATCPU12();		//SSP 1MHz derived from 12MHz for NEAT SSP.
	  BTbaudCPU12();
	  CPUSPEED=12;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Change CPU to 4MHZ resistor capaitor 1% tolerance.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void CPU4MHz(void)
{

	/* Clock Setup                        */

	  if (((LPC_SC->PLL0STAT >> 24) & 3) == 3)		//PLL  connected and enabled.
	  {
//if already at 12MHz, do nothing.
			disableInputInterrupt();
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
	LPC_SC->FLASHCFG |= 0x5000;  // 0 for up to 20MHz. But 5 is always safe. cycles to access flash memory.

	//LPC_SC->CLKSRCSEL = 0x00;
//		LPC_SC->CCLKCFG = 0x0;     //  Select the IRC as clk
//	LPC_SC->SCS = 0x00;		    // not using XTAL anymore

	LPC_SC->CCLKCFG = 0x0;     // PLL clock divided by.

//	LPC_SC->PCONP=0;	//saves 39uA
	enableInputInterrupt();
	  }
	  //clock now at 12MHz main clock or 4MHz IRC.
	  //clock to 4MHz.
	  		LPC_SC->CLKSRCSEL = 0x00; //Select IRC 4MHzmain oscillator.
	  		LPC_SC->SCS = 0; //disable main oscillator



//clock now at 4MHz IRC.
	  timer2CPU4();		//12MHz clock, generate 1MHz system clock for sleep and delays from 12MHz CPU clock.
	  SSPNEATCPU4();	//SSP 1MHz derived from 4MHz for NEAT SSP.
	  BTbaudCPU12();
		CPUSPEED=4;
}










/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief GPIO interrupt handler. BT, NEAT, INPUTS.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void EINT3_IRQHandler(void)				//GPIO interrupt.
{
//interrupts on input change state and bluetooth character.
	byte a=1;
	byte charge;
	int s,t,u,v,w;
	//int b,c,d,e,f,g,h,i,j,k;



//	s=LPC_GPIOINT->IO0IntEnR;
//	t=LPC_GPIOINT->IO0IntEnF;
//	u=LPC_GPIO2->FIOPIN;
//	v=LPC_GPIO0->FIOPIN;
//	w=LPC_GPIOINT->IO0IntEnF;


	s=LPC_GPIOINT->IO0IntStatF;
	t=LPC_GPIOINT->IO2IntStatF;
	u=LPC_GPIOINT->IO0IntStatR;
	v=LPC_GPIOINT->IO2IntStatR;

if (PCBiss==3||PCBiss==4)
{
	//get interrupt status.
	if((SW2=(LPC_GPIOINT->IO2IntStatR&(0x1<<12)))>>11)
		LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
	if((SW1=(LPC_GPIOINT->IO2IntStatR&(0x1<<11)))>>11)
		LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
	if ((SW3=(LPC_GPIOINT->IO0IntStatR&(0x1<<21)))>>19)
		LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
	if ((SWF2=(LPC_GPIOINT->IO2IntStatF&(0x1<<12)))>>11)
		LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
	if ((SWF1=(LPC_GPIOINT->IO2IntStatF&(0x1<<11)))>>11)
		LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
	if ((SWF3=(LPC_GPIOINT->IO0IntStatF&(0x1<<21)))>>19)
		LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
	if ((SWBT=(LPC_GPIOINT->IO0IntStatR&(0x1<<16)))>>16)
		LPC_GPIOINT->IO0IntClr=0x1<<16;					//BT Interrupt
	if ((SWNEAT=(LPC_GPIOINT->IO2IntStatF&(0x1<<4)))>>4)
		LPC_GPIOINT->IO2IntClr=0x1<<4;					//NEAT Interrupt MOD, wire NEAT INTERRUPt to P2.4 pin 69.

	ACOFF=(LPC_GPIOINT->IO2IntStatR&(0x1<<8))>>8;
	if(ACOFF)LPC_GPIOINT->IO2IntClr=0x1<<8;
	ACON=(LPC_GPIOINT->IO2IntStatF&(0x1<<8))>>8;
	if(ACON)LPC_GPIOINT->IO2IntClr=0x1<<8;
}





	LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */
	LPC_TIM2->TC = 0;
	a=HEX();
	if ((3==a) && SWNEAT)
	{

		return;
	}
	if(0x0F<a)
	{

		return;
	}
// NOTE flags are set up on every interrupt, so always indicate last interrupt cause.
//	No need to separately reset flags.
	else if (SW1|SW2|SW3|SWF1|SWF2|SWF3)
		{
		BTACC=0;		//any input press clears BTACC
		if (debounce==0)
		{
			CPU12MHz();
			repeatInput(); //check if change of input, send via BT to android if change.
			LPC_TIM2->TC = 0;
			PENDALARM=0x30^(inputChange()&0x30);	//NZ if EXT and/or INT pressed.//else 0.
			debounce=1;

		}
		else
		{
			debounce=debounce;
		}

		}

	else if (SWBT)
	{
		BTACC=1;					//any bluetooth sets BTACC.
		if (CPUSPEED==0)		//has been asleep
		{

			CPU12MHz();
			us(1000);					//wait until character has been received.
			BTWAKE();				//if BT wakeup, respond with W.
		}
			LPC_TIM2->TC = 0;
	}
	else 	if (SWNEAT)
	{
			CPU12MHz();
			readNEAT();
			LPC_TIM2->TC = 3000000;
	}

}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Enable GPIO interrupts NEAT BT inputs.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void enableInputInterrupt(void)
{
	int a,b;

if (PCBiss==3||PCBiss==4)
{

	NVIC->ISER[0]=0x1<<21;					//enable eint3/GPIO  interrupt.(SHARED on bit 21.)

	a=LPC_GPIOINT->IO0IntEnR;
	b=LPC_GPIOINT->IO0IntEnF;



	LPC_GPIOINT->IO0IntEnR|=0x1<<21;			//EXT TIP input rising
	LPC_GPIOINT->IO0IntEnF|=0x1<<21;				//EXT TIP input	falling


	a=LPC_GPIOINT->IO0IntEnR;
	b=LPC_GPIOINT->IO0IntEnF;


	LPC_GPIOINT->IO2IntEnR|=0x1<<11|0x1<<12|1<<8;			//INT input	rising SW1|SW2|ACOK
	LPC_GPIOINT->IO2IntEnF|=0x1<<11|0x1<<12|1<<8;			//INT input falling SW1|SW2|ACOK




//	LPC_GPIOINT->IO2IntEnR|=0x1<<12;			//MID input rising
//	LPC_GPIOINT->IO2IntEnF|=0x1<<12;			//MID input falling

	LPC_GPIOINT->IO2IntEnF|=0x1<<4;			//NEAT falling


}


}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Disable GPIO interrupts NEAT BT inputs.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void disableInputInterrupt(void)
{

if (PCBiss==3||PCBiss==4)
{
	NVIC->ICER[0]=(0x1<<21);					//disable eint3/GPIO  interrupt.(SHARED on bit 21.)
}
else if (PCBiss==2)
{
	NVIC->ICER[0]=(0x1<<21);		//disable eint3/GPIO 0/GPIO2 interrupt.
}
}





/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Checks status, powers down if inactive SPI and UART and empty BT buffer.
///@param void
///@return 1 if has powered down, 0 if not powered down.
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC int powerDown(void)
{

	int r=0;		//return value
	byte a;
	int b,s;
	int c,d,e,f,g,h,i,j,k,x;

	//FEED watchdog.
	s=LPC_TIM2->TC;

	if((debounce==1)&&(20000 < LPC_TIM2->TC))
	{
		debounce=0;
		repeatInput(); //check if change of input, send via BT to android if change.
		PENDALARM=0x30^(inputChange()&0x30);	//NZ if EXT and/or INT pressed.//else 0.
	}


#if release==1
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif


	b=LPC_SSP0->SR;			//bit 7=1 is SPI transfers complete
	b=LPC_UART1->LSR;


	if  (  s -t > 10000 ) {

		LPC_TIM2->TC= t;
	}



	LEDFLASH();
	s=LPC_TIM2->TC;
	if (PENDALARM && ALARMtime&&!BTACC) {

			if  (ALARMtime * 100000 < LPC_TIM2->TC) {

	//			LED2GREEN();
	//			LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
			NEATALARM();
	//		LPC_GPIO_BTRESET FIOSET	= BTRESET;	//Bluetooth reset.	RESET BT
			PENDALARM = 0;
			a=0x30&inputChange();
			if	(a!=0x30)	//input still pressed after NEAT alarm.
			{
				disableInputInterrupt();
			LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
			LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
			LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
			LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
			LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
			LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
			LPC_GPIOINT->IO0IntClr=0x1<<16;					//BT Interrupt
			LPC_GPIOINT->IO2IntClr=0x1<<4;					//NEAT Interrupt MOD, wire NEAT INTERRUPt to P2.4 pin 69.
			NVIC->ICPR[0]=(0x1<<21);		//clear pending GPIO interrupt

			while(0x30!=0x30&inputChange());
				//turn off when input released.
			while(1)
			{
				LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
				NEATALARM();

				LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
				NEATOFF();
				LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
				CPU4MHz();
			}
			}


	//	if input still pressed, then go into power down sequence.



			LED1OFF();
		}

	}
	else

	{
//		LED1GREEN();
		if (30000000 < LPC_TIM2->TC)	SystemOFF();		//if >30s then power off.
	if(0x40==((LPC_UART1->LSR)&(0x41)))	//bit 1=0 RX FIFO empty, bit 6=1 TX FIFO empty.
	if(rxstart==rxend)					//nothing waiting in bluetooth rx buffer
	if(txstart==txend)					//nothing waiting in bluetooth tx buffer
	if (3000000 < LPC_TIM2->TC)
	{
//		s=LPC_TIM2->TC;



		disableInputInterrupt();
	r=1;


	LED1OFF();
	LED2OFF();
	CPUSPEED=0;		//0 means asleep, 12=12MHz, 100=100MHz.

    // Deep-Sleep Mode, set SLEEPDEEP bit
//	SCB->SCR = 0x4;
//	LPC_SC->PCON = 0x0;
	// Deep Sleep Mode wake up from EINT/GPIO Does not recover from sleep.


	//Power Down Mode wake up from EINT/GPIO Works but not on debug.
	CPU4MHz();						//reduce power.
	LPC_SC->PCONP=1<<15;		//only GPIO needed during power down.
	CPUSPEED=0;		//0 means asleep, 12=12MHz, 100=100MHz.

	LPC_GPIOINT->IO0IntEnR|=0x1<<16;			//Bluetooth rising interrupt


	 //Power down mode.
#if release==1
		SCB->SCR = 0x4;			//sleepdeep bit
		LPC_SC->PCON = 0x01;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
#endif
#if release==0
	    //Power down mode.
		SCB->SCR = 0x0;
		LPC_SC->PCON = 0x00;
#endif
		enableInputInterrupt();

	__WFI(); //go to power down.



	 LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */

#if release==1
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif

//	ACSTATE();

	BatteryState();
//	LEDFLASH();

	}
	}
	t=LPC_TIM2->TC;
	 return r;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Battery state indicator, LED is yellow for low battery, or green for battery good..
///@param void
///@return void
///
///battery low if volt below 3.591V
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void BatteryState()
{
	byte a;
	a=I2CBATTERY();
if(a >=95)//92=92*8*4.88mV =3.591V, 93=3.63V, 95=3.708V
{
	LED1GREEN();//battery good active
	batterygood=1;
	if (STATE!='P')
	{
	STATE='H';
	}
}
else
{
	 LED1YELLOW();//battery low active.
	 batterygood=0;
		if (STATE!='P')
		{
		STATE='L';
		}
}
}



////////////////////////////////////////////////////////////
///@brief Charge state from Power on and power off.
///@param void
///@return void
///
///set batterygood to true or false. always true if power on.
///set NIMH or lithium if charge is on
///
////////////////////////////////////////////////////////////
void ACSTATE()
{
	if(ACON)
		{
			ACON=0;
			batterygood=1;
				I2CREAD();

		}
		if(ACOFF)
		{
			ACOFF=0;
			if( I2CBATTERY()>95)batterygood=1;
			else batterygood=0;
			I2CREAD();
		}
}


void LEDFLASH()
{

	ACSTATE();

	if(batterygood)
	{
		if (100000 > LPC_TIM2->TC)
	{LED1GREEN();}
	else if (1000000 > LPC_TIM2->TC)
	{LED1OFF();}

	else 	if (!BTACC&&((LPC_TIM2->TC -1100000) / 200000) % 2)
		{LED1GREEN();}
	else
		{LED1OFF();}
	}




	else
	{
		if (100000 > LPC_TIM2->TC)
		{LED1YELLOW();}
		else if  (1000000 > LPC_TIM2->TC)
		{LED1OFF();}

		else if (!BTACC&&((LPC_TIM2->TC -1100000) / 200000) % 2)
			{LED1YELLOW();}
		else
			{LED1OFF();}
		}

	}









