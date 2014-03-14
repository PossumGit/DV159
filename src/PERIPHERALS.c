///@name        	Simple peripherals: HEX, SWITCHES, LEDS.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx.h"

//Public variables
PUBLIC char STATE='P';
PUBLIC word LastInputTime=1;
//PUBLIC volatile word LastInputTime=0;
PUBLIC volatile byte InputState=0x3E;
PUBLIC  volatile byte LastInputState=0x3E;
PUBLIC volatile byte lastInputSent=0x3E;
PUBLIC int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.

volatile byte LASTinput =0;

//Private variables

//External variables
EXTERNAL int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.
EXTERNAL int HELDtime;
EXTERNAL int RELEASEtime;
EXTERNAL volatile word SWF1;		//falling interrupt state.
EXTERNAL volatile word SWF2;
EXTERNAL volatile word SWF3;
EXTERNAL volatile word SW1;		//rising interrupt state
EXTERNAL volatile word SW2;
EXTERNAL volatile word SW3;


//Private functions
PRIVATE void WDT_IRQHandler(void);


//External functions
EXTERNAL void sendBT(byte in[] , unsigned int);
//EXTERNAL int I2CBATTERY(void);

//public functions
PUBLIC byte	inputTest(void);
PUBLIC int repeatInput(void);
PUBLIC int copyInput(void);
PUBLIC byte	inputChange(void);
PUBLIC int	inputCheck(void);
PUBLIC void	LED1GREEN(void);
PUBLIC void	LED1YELLOW(void);
PUBLIC void	LED1OFF(void);
PUBLIC void	LED2GREEN(void);
PUBLIC void	LED2YELLOW(void);
PUBLIC void	LED2OFF(void);
PUBLIC byte HEX(void);
PUBLIC void	us(unsigned int);
PUBLIC void timer2CPU4(void);
PUBLIC void timer2CPU12(void);
PUBLIC void timer2CPU44(void);
PUBLIC void timer2CPU100(void);
PUBLIC void EnableWDT10s(void);
PUBLIC void EnableWDT2s(void);
PUBLIC int PCB(void);
PUBLIC void BatteryState(void);
PUBLIC void SystemOFF(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief return PCB number
///@param void
///@return int PCB issue
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC int PCB()
{
	int R35,R36,R37;

	LPC_GPIO0->FIODIR&=~(1<<3);				//pin 99, R35
	LPC_GPIO0->FIODIR&=~(1<<2);				//pin 98, R36
	LPC_GPIO1->FIODIR&=~(1<<0);				//pin 95, R37
	LPC_PINCON->PINMODE0&=~(0x3<<6);			//P0.3
	LPC_PINCON->PINMODE0&=~(0x3<<4);			//P0.2 pulldown.
	LPC_PINCON->PINMODE2&=~(0x3<<0);			//P1.0


	R35=(~LPC_GPIO0->FIOPIN &(1<<3))>>3;		//1 if R35 fitted
	R36=(~LPC_GPIO0->FIOPIN &(1<<2))>>2; 		//1 id R36 fitted
	R37=(~LPC_GPIO1->FIOPIN &(1<<0))>>0;		//1 if R37 fitted

	LPC_PINCON->PINMODE0|=0x3<<6;			//P0.3 pulldown.
	LPC_PINCON->PINMODE0|=0x3<<4;			//P0.2 pulldown.
	LPC_PINCON->PINMODE2|=0x3<<0;			//P1.0 pulldown.


	//PCB is completely obsolete, so do not return PCB=2.
	if (R35==0&&R36==0&&R37==0)return 3;
	if (R35==1&&R36==0&&R37==0)return 4;
	return 0;


}




/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED1 GREEN
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1GREEN(void)
	{
	LPC_GPIO1->FIODIR |= LED1G; 		//output
	LPC_GPIO1->FIODIR |= LED1Y; 		//output
	LPC_GPIO1->FIOCLR = LED1G; 		//
	LPC_GPIO1->FIOSET = LED1Y; 		//LED3	GREEN
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED1 Yellow
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1YELLOW(void)
	{
	LPC_GPIO1->FIODIR |= LED1G; 		//output
	LPC_GPIO1->FIODIR |= LED1Y; 		//output
	LPC_GPIO1->FIOSET = LED1G; 		//
	LPC_GPIO1->FIOCLR = LED1Y; 		//LED3 YELLOW
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED1 off
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1OFF(void)
	{
	LPC_GPIO1->FIOCLR = LED1Y; 		//
	LPC_GPIO1->FIOCLR = LED1G; 		//LED3 OFF
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED2 GREEN
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED2GREEN(void)
	{
	LPC_GPIO1->FIODIR |= LED2G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED2Y; 		//IR defined as an output.
	LPC_GPIO1->FIOCLR = LED2G; 		//
	LPC_GPIO1->FIOSET = LED2Y; 		//LED3	GREEN
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED2 Yellow
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED2YELLOW(void)
	{
	LPC_GPIO1->FIODIR |= LED2G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED2Y; 		//IR defined as an output.
	LPC_GPIO1->FIOSET = LED2G; 		//
	LPC_GPIO1->FIOCLR = LED2Y; 		//LED3 YELLOW
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED2 off
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED2OFF(void)
	{
	LPC_GPIO1->FIOCLR = LED2Y; 		//
	LPC_GPIO1->FIOCLR = LED2G; 		//LED3 OFF
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read HEX switches
///@param void
///@return byte with value equal to HEX2<<4|HEX1.
///
///HEX2 HEX1
///
///0	0	Normal QWAYO
///
///0	8	SOLO simulate with Plessey code 3.
///
///0	E	BT discover.
///
///0    F   BT factory reset.
///
///note with 0's at top of HEX switches, H1 is on right and is LSNibble.
///
///0 is all inputs open=high=1
///
///F is all inputs shorted to 0V=0
///missing HEX input reads as 0.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte HEX(void)
{
	byte a,b,c,d,r,s;

if (PCBiss==3||PCBiss==4)
{	a=(LPC_GPIO0->FIOPIN &(1<<1))>>1;
	b=(LPC_GPIO0->FIOPIN &(1<<18))>>17;
	c=(LPC_GPIO2->FIOPIN &(1<<13))>>11;
	d=(LPC_GPIO1->FIOPIN &(1<<19))>>16;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &(1<<0))>>0;
	b=(LPC_GPIO3->FIOPIN &(1<<25))>>24;
	c=(LPC_GPIO1->FIOPIN &(1<<18))>>16;
	d=(LPC_GPIO3->FIOPIN &(1<<26))>>23;
	s= 15-(a+b+c+d);			//
}
	else if (PCBiss==2)
	{	a=(LPC_GPIO0->FIOPIN &(1<<18))>>18;
	b=(LPC_GPIO4->FIOPIN &(1<<29))>>28;
	c=(LPC_GPIO1->FIOPIN &(1<<25))>>23;
	d=(LPC_GPIO1->FIOPIN &(1<<27))>>24;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &(1<<28))>>28;
	b=(LPC_GPIO0->FIOPIN &(1<<25))>>24;
	c=(LPC_GPIO0->FIOPIN &(1<<27))>>25;
	d=(LPC_GPIO0->FIOPIN &(1<<26))>>23;
	s= 15-(1+b+4+d);			//a,b fixed as high.
	s=0;}

	return s|r<<4;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read input state, set bit 7 if input change.
///@param void
///@return byte:TECLA codes. bit 5 is internal, bit 4 is external, bit 0 is MID, bit 7 is change.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte	inputChange(void)
{

	byte	a,b,c,d;

if (PCBiss==3||PCBiss==4)
{
	LPC_GPIO2->FIODIR&=~(1<<11);			//internal
	LPC_GPIO0->FIODIR&=~(1<<21);			//external tip
	LPC_GPIO2->FIODIR&=~(1<<12);			//external mid
	a=(LPC_GPIO2->FIOPIN &(1<<11))>>6;		//bit 6	INTERNAL
	b=(~LPC_GPIO0->FIOPIN &(1<<21))>>17; 	//bit 5	EXTERNAL
	c=(~LPC_GPIO2->FIOPIN &(1<<12))>>12;	//bit 4	EXTERNAL MID also means mono jack inserted
}


d=a|b|c|0xe;								//0xe sets bits 1,2,3 for TECLA spec.
	if (d!=InputState){
		LastInputState=InputState;
		InputState=d;
		LastInputTime=LPC_TIM2->TC;			//store time of last change of input

		return 0x80|d;						//bit 7 set indicates change.
	}
	else return d;
}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read input state, set bit 7 if input change.
///@param void
///@return byte:TECLA codes. bit 5 is internal, bit 4 is external, bit 0 is MID, bit 7 is change.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte	inputTest(void)
{

	byte	a,b,c,d;

if (PCBiss==3||PCBiss==4)
{
	LPC_GPIO2->FIODIR&=~(1<<11);			//internal
	LPC_GPIO0->FIODIR&=~(1<<21);			//external tip
	LPC_GPIO2->FIODIR&=~(1<<12);			//external mid
	a=(LPC_GPIO2->FIOPIN &(1<<11))>>6;		//bit 5	INTERNAL
	b=(~LPC_GPIO0->FIOPIN &(1<<21))>>17; 	//bit 4	EXTERNAL
	c=(~LPC_GPIO2->FIOPIN &(1<<12))>>12;	//bit 0	EXTERNAL MID also means mono jack inserted
}


d=a|b|c|0xe;						//0xe sets bits 1,2,3 for TECLA spec.

	 return d;
}








///////////////////////////////////////////////////////////////////
//
//
//
PUBLIC int	inputCheck(void)
{

//	byte	a,b,c,d;
	 byte in[] = { 'I', ' ' };
	 int a,b;

	if (InputState<lastInputSent){//switch has been pressed.


		a=LPC_TIM3->TC;
		b=LPC_TIM2->TC;
		if (HELDtime<LPC_TIM2->TC){		//send state was TIM3
			in[1] = InputState|0x0E;
			in[0] = STATE;
			lastInputSent=InputState;
			if(LASTinput!=in[1])
			{
			LASTinput=in[1];
			sendBT(in, sizeof(in));
			}

		}
		return 1;
	}
	else if(InputState>lastInputSent)//switch has been released.
	{

		if (RELEASEtime<LPC_TIM2->TC)		//send state was TIM3
			in[1] = InputState|0x0E;
			in[0] = STATE;
			lastInputSent=InputState;
			if(LASTinput!=in[1])
				{
			LASTinput=in[1];
			sendBT(in, sizeof(in));
				}
		return 1;
	}
	else return 0;			// no change.
}





/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief When input changes send new state to BT.
///@param void
///@return 0 if inactive, else 1
/////////////////////////////////////////////////////////////////////////////////////////////////
 PUBLIC  copyInput(void) {
	 byte in[] = { 'I', ' ' };

	 byte INT,EXT,MID;
		LPC_GPIO2->FIODIR&=~(1<<11);			//internal
		LPC_GPIO0->FIODIR&=~(1<<21);			//external tip
		LPC_GPIO2->FIODIR&=~(1<<12);			//external mid
		INT=(LPC_GPIO2->FIOPIN &(1<<11))>>11;		//bit 0	INTERNAL
		EXT=(~LPC_GPIO0->FIOPIN &(1<<21))>>21; 	//bit 0	EXTERNAL
		MID=(~LPC_GPIO2->FIOPIN &(1<<12))>>12;	//bit 0	EXTERNAL MID also means mono jack inserted






		if (in[1] & 0x80) //bit 7 high indicates change
		{

			in[0] = STATE;//'H' | (in[1] & 0x01);
			in[1] = ((in[1] & 0x3f)) | 0x0F; //clear bit 7, set bit 1.

			//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
			if(LASTinput!=in[1])
				{
			LASTinput=in[1];
			sendBT(in, sizeof(in));
				}
			return 1;
		}
		return 0;
	}





/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief When input changes send new state to BT.
///@param void
///@return 0 if inactive, else 1
/////////////////////////////////////////////////////////////////////////////////////////////////
 PUBLIC int repeatInput(void) {
	 byte in[] = { 'I', ' ' };
	in[1] = inputChange();
	if (in[1] & 0x80) //bit 7 high indicates change
	{

		in[0] = STATE;//'H' | (in[1] & 0x01);
		in[1] = ((in[1] & 0x3f));// | 0x0F; //clear bit 7, set bit 1.

		//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
		if(LASTinput!=in[1])
			{
		LASTinput=in[1];
		sendBT(in, sizeof(in));
			}
		return 1;
	}
	return 0;
}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief timer2CPU4
///@param void
///@return void
///
///sets up TIMER2 for 4MHz clock
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC void timer2CPU4(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 1-1; //(4MHz/4)/1 counts at 1MHz. Max time 4295s
//	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
//	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)


	LPC_SC->PCONP|=1<<23	;	//enable timer 3.
	LPC_TIM3->PR = 1-1; //(4MHz/4)/1 counts at 1MHz. Max time 4295s
//	LPC_TIM3->TCR = 0 | 1 << 1; //disable timer3, reset timer2
//	LPC_TIM3->TCR = 1 | 0 << 1; //enable timer3 (start timer2)



}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief timer2CPU4
///@param void
///@return void
///
///sets up TIMER2 for 12MHz clock
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC void timer2CPU12(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 3-1; //(12MHz/4)/3 counts at 1MHz. Max time 4295s
//	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
//	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)

	LPC_SC->PCONP|=1<<23	;	//enable timer 3.
	LPC_TIM3->PR = 3-1; //(12MHz/4)/3 counts at 1MHz. Max time 4295s
//	LPC_TIM3->TCR = 0 | 1 << 1; //disable timer3, reset timer2
//	LPC_TIM3->TCR = 1 | 0 << 1; //enable timer3 (start timer2)
}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief timer2CPU4
///@param void
///@return void
///
///sets up TIMER2 for 100MHz clock
/////////////////////////////////////////////////////////////////////////////////////////////////


PUBLIC void timer2CPU44(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 11-1; //(44MHz/4)/11 counts at 1MHz. Max time 4295s
//	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
//	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)

	LPC_SC->PCONP|=1<<23	;	//enable timer 3.
	LPC_TIM3->PR = 11-1; //(44MHz/4)/11 counts at 1MHz. Max time 4295s
//	LPC_TIM3->TCR = 0 | 1 << 1; //disable timer3, reset timer2
//	LPC_TIM3->TCR = 1 | 0 << 1; //enable timer3 (start timer2)
}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief timer2CPU4
///@param void
///@return void
///
///sets up TIMER2 for 100MHz clock
/////////////////////////////////////////////////////////////////////////////////////////////////


PUBLIC void timer2CPU100(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 25-1; //(100Mhz/4)/25 counts at 1MHz. Max time 4295s
//	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
//	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
	LPC_SC->PCONP|=1<<23	;	//enable timer 3.
	LPC_TIM3->PR = 25-1; //(100Mhz/4)/25 counts at 1MHz. Max time 4295s
//	LPC_TIM3->TCR = 0 | 1 << 1; //disable timer3, reset timer2
//	LPC_TIM3->TCR = 1 | 0 << 1; //enable timer3 (start timer2)
}








/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief delay of time us.
///@param unsigned int :time in us
///@return void
///
///compensates for main clock speed does not use interrupts.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	us(unsigned int time_us)
{

//change to use TIMER2, always clocks at 1MHz


	time_us=time_us+LPC_TIM2->TC;
	while (time_us>LPC_TIM2->TC);

}







 ///////////////////////////////////////////////////////////
 ///@brief Watchdog 10s
 ///@param void
 ///@return void
 ///////////////////////////////////////////////////////////
 PUBLIC void EnableWDT10s()
 {
		LPC_WDT->WDMOD =1<<0|1<<1;		//watchdog enabled, set only, cannot disable. Bit 1 set for reset, else interrupt.
	// Select internal RC for watchdog
	//select timeout
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDCLKSEL=0|1<<31;	//IRC source for watchdog. Set bit 31 to fix this selection.
		LPC_WDT->WDFEED=0xAA;
		LPC_WDT->WDFEED=0x55;
		NVIC->ISER[0]|=0x1<<0;	//WDT interrupt.
 }
 ///////////////////////////////////////////////////////////
 ///@brief Watchdog 2s
 ///@param void
 ///@return void
 ///////////////////////////////////////////////////////////
 PUBLIC void EnableWDT2s()
  {
		LPC_WDT->WDMOD =3<<0;		//watchdog enabled, and Watchdog reset set only, cannot disable. Bit 1 set for reset, else interrupt.
 	// Select internal RC for watchdog
 		//select timeout
 	LPC_WDT->WDTC = 2000000;		//set timeout 5s watchdog timer
 		LPC_WDT->WDCLKSEL=0;	//IRC source for watchdog. Set bit 31 to fix this selection.
 		LPC_WDT->WDFEED=0xAA;
 		LPC_WDT->WDFEED=0x55;
 		NVIC->ISER[0]|=0x1<<0;	//WDT interrupt.
  }



///////////////////////////////////////////////////////////////////////////////////////////////////

 ///@brief WDT IRQ Handler traps the watchdog reset during debug only
 ///@param void
 ///@return void
///
///This interrupt only occurs once and cannot be cleared, it can however be disabled.
//note watchdog after interrupt counts down to 0, then reloads WDTC.
//if feed, then counts down again.
 PRIVATE void WDT_IRQHandler(void)
 {
	 NVIC_SystemReset();						//system reset
	 NVIC->ICER[0]|=0x1<<0;	//disable WDT interrupt.
 }
//////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////
 //
 //
 //
PUBLIC void SystemOFF(void)
{
		LED1OFF();
		LED2OFF();
		disableInputInterrupt();
	LPC_GPIO_OFF FIOSET =OFF; //OFF button set high to turn off.
		NEATOFF();
		LPC_GPIO_BTRESET FIOCLR	= BTRESET;	//Bluetooth reset.	RESET BT
	CPU4MHz();

	while(1)
		{

		SCB->SCR = 0x4;			//sleepdeep bit
		LPC_SC->PCON = 0x03;	//combined with sleepdeep bit gives power down mode. IRC is disabled, so WDT disabled.
		__WFI();
		}

}

