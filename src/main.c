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

PUBLIC int ALARMtime=100;//30=3s

//Private variables local to this file

//PRIVATE unsigned *p;	//0 if loaded at 0, 0x10000 if loaded over USB.
volatile uintptr_t p;

//External variables
EXTERNAL volatile byte InputState;
EXTERNAL  volatile byte LastInputState;
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
EXTERNAL char STATE;
EXTERNAL volatile byte PENDALARM;
EXTERNAL volatile int debounce;
EXTERNAL int BTACC;
EXTERNAL volatile byte I2CSlaveBuffer[];
EXTERNAL int NIMH;
EXTERNAL int LITHIUM;
extern char QUERYnewbattery(void);
extern int	PCBiss;

//Local functions
PRIVATE void powerupHEX(void);
PRIVATE void LOOP(void);

//External functions
extern void asm_vivo(int);			//asm vivo burst
extern void asm_holtek(void);
extern void asm_freq(void);
EXTERNAL void initUART(void);
EXTERNAL void setupBT(void);
EXTERNAL word rxtxBT(void);
EXTERNAL void factoryBT(void);
EXTERNAL void initBT(void);
EXTERNAL int processBT(void);
EXTERNAL void resetBT();
EXTERNAL void setupBT(void);
EXTERNAL void	BTbaudCPU44L(void);
EXTERNAL void receiveBTbuffer(int,int);

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
EXTERNAL void IRDMAVIVO(void);

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
EXTERNAL void I2CNewBattery(void);
EXTERNAL void EnableWDT10s(void);
EXTERNAL void EnableWDT2s(void);
EXTERNAL void BatteryState(void);
EXTERNAL char READPIO(void);
EXTERNAL void NEATALARM(void);
EXTERNAL char I2CSTATUS(void);
EXTERNAL void NEATSIM(void);
EXTERNAL int storedcharge;
EXTERNAL int ChargeConfidence;
EXTERNAL int PCB(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief  main is entry point for main code, called from cr_starup_lpc176x.c
/// Called by ResetISR in cr_startup_lpc176x.c (which is the first code to run from the reset of the cpu.)
/// ResetISR address comes from vector table.
///@param void
///@return never returns
///
///powerup to main takes about 22.4ms.
///2.6ms to power up
///17.9ms to copy data to RAM
///1.9ms to zero unused dataspace in RAM
///3.3V on to main takes 19.8ms
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int main(void) {
	while (1)

	{



		LED2GREEN();
		timer2CPU4();		//4MHz clock, generate 1MHz system clock for sleep and delays from 4MHz CPU clock.
		LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
		LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
		LPC_TIM3->TCR = 0 | 1 << 1; //disable timer3, reset timer3
		LPC_TIM3->TCR = 1 | 0 << 1; //enable timer3 (start timer3)
		SSPNEATCPU4();	//SSP 1MHz derived from 4MHz for NEAT SSP.
		BTbaudCPU12();

		I2CINIT();			//at 4MHz.
		QUERYnewbattery();
		I2CREAD();

		p=SCB->VTOR;		//vector=0 if loaded at 0, 0x10000 if loaded at 0x10000, ie over USB driver.
		if (p!=0)	//loaded over usb driver.
		{
			CPU12MHz();		//otherwise CPU can lock up for no obvious reason I can understand.
		}

			LED2GREEN();	//nimh battery.

	//main +5ms
		LED1GREEN();

		ALARMtime=30;			//3s


		LPC_GPIO_MICCE FIOCLR = MICCE; 		//MIC CHIPEN=0=disabled.
		LPC_GPIO_MICCE FIODIR |= MICCE; 	//CHIPEN on mic =output.

		LPC_GPIO_FLASHCS FIOSET = FLASHCS; 	//CHIPEN=1=disabled.
		LPC_GPIO_FLASHCS FIODIR |= FLASHCS; //CHIPEN 1 disabled on FLASH.

		LPC_GPIO_T2G FIOSET = T2G; 			//T2g=1=disabled. (USB related)
		LPC_GPIO_T2G FIODIR |= T2G; 		//T2Vgs=0, 3v3 on pin.

			PCBiss=PCB();								//read PCB, place ports in pulldown to save power.

if (PCBiss==3||PCBiss==4)
{
	LPC_GPIO1->FIOCLR |=1<<29			;//IR capture state=0.
	LPC_GPIO1->FIODIR |=1<<29			;//IR capture =output. SET for capture IR, clear for low power.
}


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

	CPU12MHz();
	LPC_TIM2->TC = 0;
	LED2OFF();
	while (1) {
			powerDown();  	//Go to low power state if no comms, and timer3>3s:

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

		byte	a,b,d;

	while(1)
	{

	a = HEX();
	switch (a) {


	case 0x01:			//TEST IR capture and playback.
		NEATRESET();
		CPU12MHz();
		us(100000);
		LED2OFF();
		while(1)
		{
			captureIR();
			playIR();

		}
		break;


	case 0x02:			//TEST NEAT transmit, inc ID every 10s.
	//	CPU12MHz();
	//	us(20000000);
	//	LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		disableInputInterrupt();
		initUART();
		initBT();
		us(5000000);
		LED2OFF();
		LPC_TIM2->TC=0;
		time=0;			//
		NEATRESET();

	//	NEATWR(0x45,0);

	//	NEATINIT();
		while(1)


		for (l=1;l<10;(l++))
		for (k=0;k<10;k++)
		for (j=0;j<10;j++)
		for (i=0;i<10;i++)
		{
			LPC_TIM2->TC=0;
			time=10000000;			//
			h=i+(j<<4)+(k<<8)+(l<<12);



		while (LPC_TIM2->TC<time);
		time=20000000;			//
		LED1GREEN();
	//	NEATALARM();
		NEATTX(0xFF,0x00,h);		//battery state, ALARM type, ID(16 bits)
		LED1OFF();



		}
		break;
	case 0x03:			//TEST NEAT RX, receive an alarm,inc ID then re-transmit.
						//works well with TREX pager 2.
	//	CPU12MHz();
		LED1GREEN();
	//	us(1000000);
		LED2OFF();



			NEATRESET();
	//		enableInputInterrupt();
			LED1OFF();
			while(1)
			{



//		#if release==1
//				SCB->SCR = 0x4;			//sleepdeep bit
//				LPC_SC->PCON = 0x01;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
//		#endif

//		#if release==1
//			__WFI(); //go to power down.
//		#endif



			if(!(NEATINT & LPC_GPIO_NEATINT  FIOPIN))		//NEAT INT is low, read neat.  GPIO in.
			{
				LED1YELLOW();
				CPU12MHz();



	//		SWNEAT=0;
	//		CPU12MHz();
			NEATWR(2,0xA0);			//reset read
			CPU4MHz();						//reduce power.
			us(700000);
			LED1OFF();
			}


			}

		break;


	case 04:					//TEST 4MHz LED flash

		LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals

		initUART();

		initBT();

		setupBT();

		LED2OFF();
		CPU4MHz();
		while(1)
		{
			LED2YELLOW();
			LED2GREEN();
			us(400000);
			a=inputChange();
			if (~a&(1<<4))
			{
				LED2YELLOW();
				LED1YELLOW();
			}
			else if (~a&(1<<5))
			{	LED2GREEN();
			LED1GREEN();
			}
			else
			{
					LED2OFF();
			}
			us(400000);

		}
		break;
	case 0x05:
			CPU4MHz();
			asm_freq();
			break;
/*	case 05:				//TEST 12MHz LED flash
		 LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals
			CPU12MHz();
			LED2OFF();
		while(1)
		{

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

			CPU4MHz();

			BatteryState();
				CPU100MHz();
				us(200000);
				LED1OFF();
				us(200000);
			//	LED1GREEN();

			}
		break;
*/


	case 06:
break;
//experiments:::
		CPU4MHz();

		while (1)
		IRDMAVIVO();



		I2CREAD();
		I2CChargeWR(0x8000);
		us(6000000);

		for (j=0;j<0x100;j+=10)
		{

		for (i=0;i<0x8;i++)
		{Buffer [j+i]=I2CSlaveBuffer[i];
		}
		Buffer [j+i++]=0xEE;
		Buffer [j+i++]=0xEE;
		I2CREAD();
	//	us(100000);

		}
		break;

	case 07:				//test IR synthesis Transmit code Plessey 3 every 5 seconds.
		CPU12MHz();
		us(100000);
		LED2OFF();
		while (1)
		{
			LPC_TIM2->TC=0;

		 IRsynthesis('P',4,0x5);		//Plessey  4 repeats, code 6 for HC603c
			playIR();

			us(4705096);

//us(1000000);


		}
		break;

	case 0x08:				//testing IR

CPU12MHz();


j=0;
i=0;
Buffer[i++] = 0x8 << 28 | 2047 << 16 | 0 << 4 | 0x0 << 0;//0x08=header,2047=20.47us=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<4;k++)
{
	Buffer[i++]=10000+(100000*j)+4096*k;  //start set of 4 pulses (24.4KHz).
}
Buffer[i++] = 0x8 << 28 | 44 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 44=pulse width, 0=period, 0=skip 0

	for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+89*k;  //test for Pioneer 1.125MHz IR codes (fastest known.)
	}

Buffer[i++] = 0x8 << 28 | 50 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 50=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+100*k;  //test for 1MHz
	}
Buffer[i++] = 0x8 << 28 | 100 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 100=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+200*k;  //test for  500KHz
	}
Buffer[i++] = 0x8 << 28 | 200 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 200=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+400*k;  //test for 250KHz
	}
Buffer[i++] = 0x8 << 28 | 400 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 400=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+800*k;  //test for 125KHz
	}
Buffer[i++] = 0x8 << 28 | 500 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 500=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
		{
			Buffer[i++]=10000+(100000*j)+1000*k;  //test for 100KHz
		}
Buffer[i++] = 0x8 << 28 | 625 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 625=pulse width, 0=period, 0=skip 0

	for (k=0,j++;k<16;k++)
			{
				Buffer[i++]=10000+(100000*j)+1250*k;  //test for 80KHz
			}

	Buffer[i++] = 0x8 << 28 | 833 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 833=pulse width, 0=period, 0=skip 0 data

		for (k=0,j++;k<16;k++)
				{
					Buffer[i++]=10000+(100000*j)+1666*k;  //test for 60KHz
				}
Buffer[i++] = 0x8 << 28 | 1000 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 1000=pulse width, 0=period, 0=skip 0


for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+2000*k;  //test for 50KHz
	}
Buffer[i++] = 0x8 << 28 | 1315 << 16 | 0 << 4 | 0x0 << 0;//0x08=header, 1315=pulse width, 0=period, 0=skip 0

for (k=0,j++;k<16;k++)
	{
		Buffer[i++]=10000+(100000*j)+2632*k;  //test for 38KHz
	}

Buffer[i++]=0;




while(1)
{
 playIR();
 us(1000000);
}
 //		 playIR();


break;


/*
	case 0x09:
		CPU4MHz();
			us(100000);
			LED2OFF();
			while(1)
			{

			 IRsynthesis('H',10,0x0404);		//HC1820  4 repeats, code  for HC1820 socket 1
			 playIR();
				us(10000000);

			 if(0x0A!=HEX())break;
			}

			break;
*/
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

	case 0x0C: 				//DEBUG recover clock
		LPC_GPIO1-> FIODIR |= IRON; 		//output
		LPC_GPIO1-> FIOSET |=IRON	;
		LPC_GPIO_BTCTS FIODIR |= BTCTS; 		//output
		LPC_GPIO_BTCTS FIOCLR |=BTCTS	;
	//	initUART();
		initBT();

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
	case  0x0D:		//VIVO remote code.
//from starting code to first IR is 5.8ms
		CPU4MHz();
		LPC_SC->PCLKSEL0 |= ((1 << 2)|(1<<4)); //TIMER0 PREDIVIDE =1 (system clock)|TIMER1 PREDIVIDE =1 (system clock)



		LPC_TIM1->PR = 0; //set IR sample clock

		LPC_TIM1->TCR = 0 | 1 << 1; //enable timer 1 (reset timer1)
		LPC_TIM1->TCR = 1 | 0 << 1; //enable timer 1 (start timer1)



		I2CSTATUS();
		a=I2CBAT();
		if (a<0x60)
		{
		b=0x10&I2CSTATUS();
			if (b)
			{a=100;
			}
		}
			if(a >=95)//95=3.708V
		{
				b=0x10;
			LED1GREEN();//battery good active
		}
		else
		{
			b=0;
			 LED1YELLOW();//battery low active.
		}



//		BatteryState();			//LED Orange/yellow depending on battery state.//takes about 5.8ms
//main+10.8ms
		 I2CSHUTDOWN();
		EnableWDT10s();		//10s watchdog until LOOP.
		LPC_WDT->WDTC = 60000000;	//set timeout 10s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed
		  LPC_SC->FLASHCFG  =0;


		  if(!(0x4&LPC_SC->RSID))		//if not watchdog
		  {
		asm_vivo(0); //vivo code, never return.
		  }

//should not return.
//
//
			LED1YELLOW();
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		NEATOFF();
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
		while(1)
			{
			LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
	//		SCB->SCR = 0x4;			//sleepdeep bit
	//		LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
	//		__WFI();
			}



		break;

	case 0x0E:				//FREEWAY REMOTE code
		//from starting code to first IR is 5.8ms

		CPU4MHz();


		I2CSTATUS();
		a=I2CBAT();
		if (a<0x60)
		{
		b=0x10&I2CSTATUS();
			if (b)
			{a=100;
			}
		}
			if(a >=95)//95=3.708V
		{
				b=0x10;
			LED1GREEN();//battery good active
		}
		else
		{
			b=0;
			 LED1YELLOW();//battery low active.
		}



//		BatteryState();			//LED Orange/yellow depending on battery state.//takes about 5.8ms
//main+10.8ms
		 I2CSHUTDOWN();


			LED2OFF();
			EnableWDT10s();		//10s watchdog until LOOP.
			LPC_WDT->WDTC = 60000000;	//set timeout 10s watchdog timer
			LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
			LPC_WDT->WDFEED=0x55;			//watchdog feed

			 LPC_SC->FLASHCFG  =0;
			  if(!(0x4&LPC_SC->RSID))		//if not watchdog
			asm_holtek();

//
//
//
			LED1YELLOW();
			LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
			NEATOFF();
			LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT

			while(1)
			{
				LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
	//				SCB->SCR = 0x4;			//sleepdeep bit
	//				LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
	//				__WFI();
			}

		break;

	case 0x0F:				//factory reset Blue Tooth.
		 CPU12MHz();
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.

		LED1OFF();
		LED2OFF();
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		while(1);
		us(1000000);
		resetBT();
		us(1000000);
		LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */
		initUART();
//921kbaud

#if baud==92
				CPU44MHz();
				BTbaudCPU44L();
#elif baud==46
				CPU44MHz();
				BTbaudCPU44L();
#elif baud==23
				CPU44MHz();
				BTbaudCPU44L();
#elif baud==11
				CPU44MHz();
				BTbaudCPU44L();
#endif

//now default value of 115.2KBAUD.
				LED1GREEN();
				LED2OFF();
		initBT();

		setupBT();



		us(10000);
		LED1GREEN();
		LED2YELLOW();
		while(1);
		LED1OFF();
		LED2OFF();
		LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		while(1);
		break;


	case	0x10:			//radio transmit BT disabled
	LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
			LED2OFF();
		LPC_TIM2->TC=0;
		time=0;			//
		NEATRESET();
		while(1)
		for (l=1;l<10;(l++))
		for (k=0;k<10;k++)
		for (j=0;j<10;j++)
		for (i=0;i<10;i++)
		{
			LPC_TIM2->TC=0;
			time=15000000;			//
			h=i+(j<<4)+(k<<8)+(l<<12);
		while (LPC_TIM2->TC<time);
		time=20000000;			//
		LED1GREEN();

		NEATTX(0xFF,0x00,h);		//battery state, ALARM type, ID(16 bits)
		LED1OFF();
		}
		break;

	case 0x11:
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT

		//	CPU12MHz();
			LED1GREEN();
		//	us(1000000);
			LED2OFF();



				NEATRESET();
		//		enableInputInterrupt();
				LED1OFF();
				while(1)
				{



	//		#if release==1
	//				SCB->SCR = 0x4;			//sleepdeep bit
	//				LPC_SC->PCON = 0x01;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
	//		#endif

	//		#if release==1
	//			__WFI(); //go to power down.
	//		#endif



				if(!(NEATINT & LPC_GPIO_NEATINT  FIOPIN))		//NEAT INT is low, read neat.  GPIO in.
				{
					LED1YELLOW();
					CPU12MHz();



		//		SWNEAT=0;
		//		CPU12MHz();
				NEATWR(2,0xA0);			//reset read
				CPU4MHz();						//reduce power.
				us(700000);
				LED1OFF();
				}


				}

			break;

/*
	case	0x20:

		Charge=0x7900;
		I2CChargeWR(Charge);
		while(1);

		break;
*/











	case 0x00:		//main startup and execute for QWAYO
	default:
#if release==1

		EnableWDT10s();		//10s watchdog until LOOP.
	//	LPC_WDT->WDTC = 10000000;	//5s after next FEED(in powerDown in LOOP). set timeout 5s watchdog timer

#endif
		CPU12MHz();
		LPC_TIM2->TC = 0;
	//	LED1GREEN();
	//	CPU12MHz();
	//	resetBT();


	//	LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
	//	us(50000);
	//	LPC_GPIO_BTRESET FIOSET	= BTRESET;	//Bluetooth reset.
	//	LED1YELLOW();

		initUART();
		initBT();
		us(500000);
	//	resetBT();
	//	setupBT();
	//	LED1YELLOW();
		I2CINIT();
		I2CREAD();		//first read is not valid, so throw it away.
		BatteryState();			//LED Orange/yellow depending on battery state.
	//	CPU12MHz();
		NEATRESET();
//		initSSP0();
//		LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
//		LPC_GPIO_NEATCS FIODIR |=NEATCS; //CHIPEN on NEAT.
//		LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
//		NEATWR(1,1);				//reset NEAT/as power up reset.
//		NEATWR(0x45,0);				//set alarm type as 0.

	//	LED1OFF();
	//	LED2OFF();
	//	LPC_TIM2->TC = 0;



	//send codes for down up if input not pressed, or input state if pressed.
		a=0x30&inputChange();
		PENDALARM=0x30^a;	//NZ if EXT and/or INT pressed.//else 0.
		debounce=0;
		if(0x30==a)
		{//input is in up state, so woken by momentary input press or power up.
			 byte up[] = { 'P', 0x1f};	//send press and release internal input.

				//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
				sendBT(up, sizeof(up));
				us(100000);		//wait 100ms
				up[1]=0x3f;
				sendBT(up, sizeof(up));

		}
		else
		{
			byte down[]={'P',0x0f};
			down[1] = (a & 0x3f) | 0x0F; //clear bit 7, set bit 1.

			//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
			sendBT(down, sizeof(down));
		}





		LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
		LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
		LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
		LPC_GPIOINT->IO2IntClr=0x1<<12;					//SW2 bit 1	EXT
		LPC_GPIOINT->IO2IntClr=0x1<<11;					//SW1 bit 0 INT
		LPC_GPIOINT->IO0IntClr=0x1<<21;					//SW3 bit 2 MID
		LPC_GPIOINT->IO0IntClr=0x1<<16;					//BT Interrupt
		LPC_GPIOINT->IO2IntClr=0x1<<4;					//NEAT Interrupt MOD, wire NEAT INTERRUPt to P2.4 pin 69.
		NVIC->ICPR[0]=(0x1<<21);		//clear pending GPIO interrupt
		enableInputInterrupt();
		CPU12MHz();
		LPC_TIM2->TC = 0;
		LPC_SC->PCONP     = Peripherals ;       // Enable Power for Peripherals      */

	//	BTACC=0;
//		LED2OFF();
		inputChange();
		inputChange();
		LastInputState=InputState;
		NEATSIM();
		LOOP();			//never exits this loop.
		break;
;
	}



}
}
