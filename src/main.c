//Notes
//void NVIC_SystemReset(void) to programmatically reset.
//
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
#include "lpc17xx_rtc.h"

//Public variables
//PUBLIC__CRP const unsigned int CRP_WORD = CRP_NO_CRP;///< code protection word
PUBLIC int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.
PUBLIC int ALARMtime=0x53;
//Private variables local to this file




//External variables
EXTERNAL volatile int Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL int FlashAddress; ///<This address needs to be set before flash read and flash write.
EXTERNAL volatile word SW1;
EXTERNAL volatile word SW2;
EXTERNAL volatile word SW3;
EXTERNAL volatile word SWF1;
EXTERNAL volatile word SWF2;
EXTERNAL volatile word SWF3;
EXTERNAL volatile word SWBT;
EXTERNAL volatile word SWNEAT;
//Local functions
PRIVATE void powerupHEX(void);
PRIVATE void LOOP(void);

//External functions
extern void asm_vivo(void);			//asm vivo burst
extern void asm_holtek(void);
EXTERNAL void initUART(void);
EXTERNAL void setupBT(void);
EXTERNAL word rxtxBT(void);
EXTERNAL void factoryBT(void);
EXTERNAL void initBT(void);
EXTERNAL int processBT(void);
EXTERNAL void resetBT();
EXTERNAL void setupBT(void);

EXTERNAL void CPU4MHz(void);
EXTERNAL void CPU12MHz(void);
EXTERNAL void CPU100MHz (void);
EXTERNAL void enableInputInterrupt(void);
EXTERNAL void disableInputInterrupt(void);
EXTERNAL int powerDown(void);
EXTERNAL void discoverBT(void);
EXTERNAL void IRsynthesis(byte IRtype, byte IRrep, int IRcode);
EXTERNAL int captureIR(void);
EXTERNAL void playIR(void);

EXTERNAL void NEATTX(byte battery, byte alarm, word ID);
EXTERNAL void NEATRESET(void);
EXTERNAL byte NEATRD(byte );
EXTERNAL void NEATWR(byte , byte );
EXTERNAL void	NEATOFF(void);


EXTERNAL void	LED1GREEN(void);
EXTERNAL void	LED1YELLOW(void);
EXTERNAL void	LED1OFF(void);
EXTERNAL void	LED2GREEN(void);
EXTERNAL void	LED2YELLOW(void);
EXTERNAL void	LED2OFF(void);
EXTERNAL byte 	HEX(void);
EXTERNAL void	us(unsigned int time_us);
EXTERNAL byte	inputChange(void);
EXTERNAL void I2CREAD(void);
EXTERNAL void I2CINIT(void);
EXTERNAL void I2CSHUTDOWN(void);
EXTERNAL void EnableWDT10s(void);
EXTERNAL void EnableWDT2s(void);
EXTERNAL void BatteryState(void);
EXTERNAL void NEATALARM(void);


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


		 CPU12MHz();
//ext interrupt 2 causes problems after USB. ICER0 bit 20.
		NVIC->ICER[0]=0xFFFFFFFF;		//disable all interrupts.
		NVIC->ICER[1]=0xFFFFFFFF;
		LPC_TIM2->TC=0;
		CPU12MHz();
		LED2YELLOW();

	//	 us(1000);
	//	 while(1)
	//	 {
	//		 LED1YELLOW();
	//		 LED1GREEN();
	//	 }

		LPC_GPIO_OFF FIOCLR = OFF; 			//SD(shutdown) =OFF button set low to keep on, set high to turn off.
		LPC_GPIO_OFF FIODIR |= OFF; 		//SD(shutdown) =OFF. //1K pull-down prevents turning on during power up. (3.3mA is OK)
											//set GPIO0_11 to turn off device.

		LPC_GPIO_IROUT FIOCLR = IROUT; 		//clear IR output (IR off).
		LPC_GPIO_IROUT FIODIR |= IROUT; 	//IR defined as an output.


		LPC_GPIO_MICCE FIOCLR = MICCE; 		//MIC CHIPEN=0=disabled.
		LPC_GPIO_MICCE FIODIR |= MICCE; 	//CHIPEN on mic =output.

		LPC_GPIO_FLASHCS FIOSET = FLASHCS; 	//CHIPEN=1=disabled.
		LPC_GPIO_FLASHCS FIODIR |= FLASHCS; //CHIPEN 1 disabled on FLASH.

		LPC_GPIO_T2G FIOSET = T2G; 			//T2g=1=disabled.
		LPC_GPIO_T2G FIODIR |= T2G; 		//T2Vgs=0, 3v3 on pin.

		LPC_GPIO_BTRESET FIOSET	= BTRESET;	//Bluetooth reset.
		LPC_GPIO_BTRESET FIODIR |= BTRESET;	//
		PCBiss=PCB();								//read PCB, place ports in pulldown to save power.

#if PCBissue==3||PCBissue==4
	LPC_GPIO1->FIOCLR |=1<<29			;//IR capture state=0.
	LPC_GPIO1->FIODIR |=1<<29			;//IR capture =output. SET for capture IR, clear for low power.

#elif PCBissue==2

#endif

			powerupHEX(); //HEX options like factory reset on power up.

	//should not return, but if it does:

			LOOP(); //main loop, everything controlled form LOOP.
			//return here means reset.
		}
	}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Main loop. Everything comes back here.
///@param void
///@return void
///after 3s it goes to sleep in powerDown();
///it wakes up from input, BT or NEAT interrupts.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void LOOP(void) {

	while (1) {
			powerDown();  	//Go to low power state if no comms, and timer2>3s:
			processBT(); 	//process received information.
			rxtxBT(); 		//receive/transmit any BT data//may be possible to DMA here.

	}
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


	unsigned int time;
	int h,i,j,k,l;

		byte	a;

	while(1)
	{

	a = HEX();
	switch (a) {


	case 0x01:			//TEST IR capture and playback.
		CPU12MHz();
		us(100000);
		LED2OFF();
		while(1)
		{
			captureIR();
			playIR();
			CPU12MHz();
			if(1!=HEX())break;
		}
		break;


	case 0x02:			//TEST NEAT transmit, inc ID every 10s.
		CPU12MHz();
		us(1000000);
		LED2OFF();
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
			LED1GREEN();
		NEATTX(0xFF,0x00,h);		//battery state, ALARM type, ID(16 bits)
		LED1OFF();
		time=time+10000000;			//+10s
		while (LPC_TIM2->TC<time);
		if(2!=HEX())break;

		}
		break;
	case 0x03:			//TEST NEAT RX, receive an alarm,inc ID then re-transmit.
						//works well with TREX pager 2.
		CPU12MHz();
		us(1000000);
		LED2OFF();


	//		LED1YELLOW();
		EnableWDT10s();		//10s watchdog until LOOP.
		LPC_WDT->WDTC = 5000000;	//5s after next FEED(in powerDown in LOOP). set timeout 5s watchdog timer


		CPU12MHz();
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		us(10000);
		LPC_GPIO_BTRESET FIOSET	= BTRESET;	//Bluetooth reset.
		us(50000);
		initUART();
		initBT();
		resetBT();
		setupBT();
		LED1YELLOW();
		I2CINIT();
		I2CREAD();		//first read is not valid, so throw it away.


			NEATRESET();
			enableInputInterrupt();
			LPC_TIM2->TC =3000000;

			while(1)
			{
			LED1OFF();
			powerDown();
			if(SWNEAT)
			{

			SWNEAT=0;
			CPU12MHz();
			i=NEATRD(0x61);
			j=NEATRD(0x60);


			LED1GREEN();

			NEATWR(2,0xA0);			//reset read
			NEATRESET();
			us(3000000);
			LED1YELLOW();
			NEATTX(0xFF,0x00,1+i+256*j);		//battery state, ALARM type, ID(16 bits)
			LED1OFF();
			LPC_TIM2->TC =3000000;
			SWNEAT=0;		//ignore repeats within 4s.
			NEATWR(2,0xA0);			//reset read

			if(3!=HEX())break;
			}


			}

		break;


	case 04:					//TEST 4MHz LED flash
		LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */
		CPU4MHz();
		us(100000);
		LED2OFF();
		while(1)
		{	if(4!=HEX())break;
			LED2YELLOW();
			LED2GREEN();
			us(400000);
			a=inputChange();
			if (~a&(1<<4))
			{
				LED2YELLOW();
			}
			else if (~a&(1<<5))
			{	LED2GREEN();
			}
			else
			{
					LED2OFF();
			}
			us(400000);

		}
		break;
	case 05:				//TEST 12MHz LED flash
		 LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */
			CPU12MHz();
			LED2OFF();
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
	case 06:				//TEST 100MHz (emc test) LED Flash
		CPU100MHz();
		us(100000);
		LED2OFF();
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
	case 07:				//test IR synthesis Transmit code Plessey 3 every 5 seconds.
		CPU12MHz();
		us(100000);
		LED2OFF();
		while (1)
		{


			us(2000000);

					{
		 IRsynthesis('P',3,0x2);		//Plessey  2 repeats, code 3 for HC603c
			playIR();

						}



		}
		break;


	case 0x08:				//TEST turn off after 2 seconds.
		LED2OFF();
		CPU12MHz();
		I2CINIT();
		 I2CSHUTDOWN();
		 IRsynthesis('P',2,0x2);
			playIR();
			LED1OFF();
			LED2OFF();

		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
			NEATOFF();
			LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		CPU4MHz();

		while(1)
			{
			disableInputInterrupt();
			SCB->SCR = 0x4;			//sleepdeep bit
			LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
			__WFI();
			}

		break;


	case 0x09:
		CPU12MHz();
		EnableWDT2s();


		if(LPC_RTC->GPREG4)
		{
			LPC_RTC->GPREG4=0;
			LED1GREEN();
		}
		else
		{
			LPC_RTC->GPREG4=1;
			LED1YELLOW();
		}

		us(100000);
		LED2OFF();
		for(;;);
		break;
	case 0x0A:

		CPU4MHz();
		us(100000);
		LED2OFF();
		while(1)
		{
		us(2000000);
		 IRsynthesis('H',1,0x0401);		//HC1820  4 repeats, code  for HC1820 socket 1
		 playIR();
		us(2000000);
		 IRsynthesis('H',1,0x0402);		//HC1820  4 repeats, code  for HC1820 socket 1
		 playIR();
		us(2000000);
		 IRsynthesis('H',1,0x0401);		//HC1820  4 repeats, code  for HC1820 socket 1
		 playIR();
		us(2000000);
		 IRsynthesis('H',1,0x0402);		//HC1820  4 repeats, code  for HC1820 socket 1
		 playIR();

		 if(0x0A!=HEX())break;
		}

		break;
	case 0x0B:				//Enable USB programming.
		break;

	case 0x0C: 				//DEBUG recover clock
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
	case  0x0D:

		CPU4MHz();
	//	LED1GREEN();
		I2CINIT();
//		I2CREAD();		//first read is not valid, so throw it away.
//		us(2000000);

		BatteryState();			//LED Orange/yellow depending on battery state.
		LED2OFF();
		asm_vivo(); //vivo code, never return.
//should not return.
//
//
		LED1OFF();
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		NEATOFF();
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		while(1)
			{
			disableInputInterrupt();
			SCB->SCR = 0x4;			//sleepdeep bit
			LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
			__WFI();
			}



		break;

	case 0x0E:				//factory reset Blue Tooth.
		CPU4MHz();
	//		LED1GREEN();
		I2CINIT();
//		I2CREAD();		//first read is not valid, so throw it away.


		BatteryState();			//LED Orange/yellow depending on battery state.
			LED2OFF();
			while (1)
			{
			asm_holtek();
	//		BatteryState();
			}

//
//
//
			LED1OFF();
			LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
			while(1)
			{
			disableInputInterrupt();
			SCB->SCR = 0x4;			//sleepdeep bit
			LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
			__WFI();
			}

		break;

	case 0x0F:				//factory reset Blue Tooth.
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.
		LED2OFF();
		while(1);
		break;

	case 0x10:
		LED2OFF();
		NEATRESET();

		byte z[0x80];
for	(i=0;i<0x80;i++)
{
			z[i]=NEATRD(i);
}
//		NEATWR(0x1D,00);
//		NEATWR(0x1F,00);
//		NEATWR(0x1D,0xAF);
//		NEATWR(0x1F,0xB0);
			i=NEATRD(0x11);
			j=NEATRD(0x1D);
			k=NEATRD(0x1F);
			 NEATALARM();

	//		NEATWR(0x1F,00);		//number of short preamble packages sent
	//		j=NEATRD(0x1E);
	//		k=NEATRD(0x1F);
	//		NEATWR(0x1F,0xAA);
	//		l=NEATRD(0x1F);
			while(1)
			{
		}






		break;
	case 0x20:
		LED2OFF();
		captureIR();
	while (1)
	{
	a=0xf0&inputChange();
		if (a==0x90||a==0xa0)
		{

	playIR();
		}
	}

	break;

	case 0x00:		//main startup and execute for QWAYO
	default:
		EnableWDT10s();		//10s watchdog until LOOP.
		LPC_WDT->WDTC = 5000000;	//5s after next FEED(in powerDown in LOOP). set timeout 5s watchdog timer


		CPU12MHz();
		LPC_TIM2->TC = 0;
		LED1GREEN();
		CPU12MHz();
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		us(50000);
		LPC_GPIO_BTRESET FIOSET	= BTRESET;	//Bluetooth reset.
		LED1YELLOW();
		us(50000);

	//	enableInputInterrupt();
		initUART();
		initBT();
		resetBT();
		setupBT();
		LED1YELLOW();
		I2CINIT();
		I2CREAD();		//first read is not valid, so throw it away.
		CPU12MHz();
		NEATRESET();
//		j=NEATRD(0x1D);
//		k=NEATRD(0x1F);
//		if(j!=k)
//		{
//		NEATWR(0x1D,0x00);
//		NEATWR(0x1D,0xC6);			//6/2 =3s is how long if no bluetooth.
//		NEATWR(0x1D,0x00);
//		NEATWR(0x1F,0xC6);			//C/2=6s is how long even if BT acc.
//		}
//		ALARMtime=NEATRD(0x1F);
		ALARMtime=0x06;
		inputChange();
		LED1OFF();
		LED2OFF();
		LPC_TIM2->TC = 3000000;

		enableInputInterrupt();
		LOOP();			//never exits this loop.
		break;
;
	}



}
}

///////////////////////////////////////////////////////////
//TEST section.
//extern int asm_sum(int x, int y);
// Assembler test routine:
//	int j;
//	j = asm_sum(5, 6); //assembler test
