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
PUBLIC int ALARMtime=30;//=3s

//Private variables local to this file

PRIVATE unsigned *p;	//0 if loaded at 0, 0x10000 if loaded over USB.


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
EXTERNAL char STATE;
EXTERNAL volatile byte PENDALARM;
EXTERNAL volatile int debounce;
EXTERNAL int BTACC;

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
EXTERNAL char READPIO(void);
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

		char a;
//		 CPU12MHz();
//ext interrupt 2 causes problems after USB. ICER0 bit 20.
		 LPC_GPIO_BTRESET FIOSET = BTRESET; //BLUETOOTH NRESET OUT High, Low to reset.
		p=SCB->VTOR;		//vector=0 if loaded at 0, 0x10000 if loaded at 0x10000, ie over USB driver.
		if (p!=0)	//loaded over usb driver.
		{
			CPU12MHz();
		}
	//	LPC_TIM2->TC=0;
	//	CPU12MHz();
		I2CINIT();
		a= READPIO();		//takes about 5ms
		if (a&0x08)
		{
			LED2YELLOW();	//lithium battery
		}
		else
		{
			LED2GREEN();	//nimh battery.
		}
	//main +5ms
		LED1OFF();
		STATE='P';
		ALARMtime=30;			//3s

	//	 us(1000);
	//	 while(1)
	//	 {
	//		 LED1YELLOW();
	//		 LED1GREEN();
	//	 }



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
		NEATRESET();
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
			time=5000000;			//
			h=i+(j<<4)+(k<<8)+(l<<12);



		while (LPC_TIM2->TC<time);
		time=20000000;			//
		LED1GREEN();

		NEATTX(0xFF,0x00,h);		//battery state, ALARM type, ID(16 bits)
		LED1OFF();



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
		CPU12MHz();
		initUART();
		initBT();
		setupBT();
		us(100000);
		LED2OFF();
		CPU4MHz();
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
	case  0x0D:		//VIVO remote code.
//from starting code to first IR is 5.8ms
		CPU4MHz();
		I2CINIT();
//		I2CREAD();		//first read is not valid, so throw it away.
//		us(2000000);

		BatteryState();			//LED Orange/yellow depending on battery state.//takes about 5.8ms
//main+10.8ms

		EnableWDT10s();		//10s watchdog until LOOP.
		LPC_WDT->WDTC = 60000000;	//set timeout 10s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

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

	case 0x0E:				//FREEWAY REMOTE code
		//from starting code to first IR is 5.8ms
		CPU4MHz();

	//		LED1GREEN();
		I2CINIT();
//		I2CREAD();		//first read is not valid, so throw it away.


		BatteryState();			//LED Orange/yellow depending on battery state. //takes about 5.8ms
			LED2OFF();
			EnableWDT10s();		//10s watchdog until LOOP.
			LPC_WDT->WDTC = 60000000;	//set timeout 10s watchdog timer
			LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
			LPC_WDT->WDFEED=0x55;			//watchdog feed
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
		 CPU12MHz();
		factoryBT(); //HEX2=0, HEX1=E Factory reset BT.
		LED2OFF();
		while(1);
		break;








	case 0x00:		//main startup and execute for QWAYO
	default:
#if release==1

		EnableWDT10s();		//10s watchdog until LOOP.
		LPC_WDT->WDTC = 5000000;	//5s after next FEED(in powerDown in LOOP). set timeout 5s watchdog timer

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
	//	resetBT();
	//	setupBT();
	//	LED1YELLOW();
		I2CINIT();
		I2CREAD();		//first read is not valid, so throw it away.

	//	CPU12MHz();
//		NEATRESET();
		initSSP0();
		LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
		LPC_GPIO_NEATCS FIODIR |=NEATCS; //CHIPEN on NEAT.
		LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
		NEATWR(1,1);				//reset NEAT/as power up reset.

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
		BatteryState();			//LED Orange/yellow depending on battery state.
	//	BTACC=0;
		LED2OFF();
		LOOP();			//never exits this loop.
		break;
;
	}



}
}
