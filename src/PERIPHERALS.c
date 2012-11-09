///@name        	Simple peripherals: HEX, SWITCHES, LEDS.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx.h"

//Public variables

//PUBLIC volatile word LastInputTime=0;

//Private variables

	word ErrorResetTime;
	int watchCount=0;

//External variables

//Private functions

//External functions
EXTERNAL void sendBT(byte istat[], unsigned int ilength);
EXTERNAL int I2CBATTERY(void);
//public functions

PUBLIC int repeatInput(void);
PUBLIC byte	inputChange(void);

PUBLIC void	LED1GREEN(void);
PUBLIC void	LED1YELLOW(void);
PUBLIC void	LED1OFF(void);
PUBLIC void	LED2GREEN(void);
PUBLIC void	LED2YELLOW(void);
PUBLIC void	LED2OFF(void);
PUBLIC byte HEX(void);
PUBLIC  void	us(unsigned int time_us);
PUBLIC void timer2CPU4(void);
PUBLIC void timer2CPU12(void);
PUBLIC void timer2CPU100(void);
PUBLIC word	inputTime(void);
PUBLIC void ErrorTimeOut(word timeout);
PUBLIC void ErrorReset(int error);
PUBLIC void EnableWDT10s(void);
PUBLIC void EnableWDT2s(void);



/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 GREEN
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1GREEN(void)
	{
	LPC_GPIO1->FIODIR |= LED1G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED1Y; 		//IR defined as an output.
	LPC_GPIO1->FIOCLR = LED1Y; 		//
	LPC_GPIO1->FIOSET = LED1G; 		//LED3	GREEN
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 Yellow
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1YELLOW(void)
	{
	LPC_GPIO1->FIODIR |= LED1G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED1Y; 		//IR defined as an output.
	LPC_GPIO1->FIOSET = LED1Y; 		//
	LPC_GPIO1->FIOCLR = LED1G; 		//LED3 YELLOW
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 off
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED1OFF(void)
	{
	LPC_GPIO1->FIOCLR = LED1Y; 		//
	LPC_GPIO1->FIOCLR = LED1G; 		//LED3 OFF
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 GREEN
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED2GREEN(void)
	{
	LPC_GPIO1->FIODIR |= LED2G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED2Y; 		//IR defined as an output.
	LPC_GPIO1->FIOCLR = LED2Y; 		//
	LPC_GPIO1->FIOSET = LED2G; 		//LED3	GREEN
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 Yellow
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED2YELLOW(void)
	{
	LPC_GPIO1->FIODIR |= LED2G; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= LED2Y; 		//IR defined as an output.
	LPC_GPIO1->FIOSET = LED2Y; 		//
	LPC_GPIO1->FIOCLR = LED2G; 		//LED3 YELLOW
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 off
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
///HEX1 HEX2
///0	0	USB
///F	0	BT discover.
///E    0   BT factory reset.
///note on prototype PCB1 bit 4,5 (HEX2 bit 0,1)lacked pullups.
///note with 0's at top of HEX switches, H1 is on right and is LSNibble.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte HEX(void)
{
	byte a,b,c,d,r,s;

#if PCBissue==3
	a=(LPC_GPIO0->FIOPIN &(1<<1))>>1;
	b=(LPC_GPIO0->FIOPIN &(1<<18))>>17;
	c=(LPC_GPIO2->FIOPIN &(1<<13))>>11;
	d=(LPC_GPIO1->FIOPIN &(1<<19))>>16;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &(1<<0))>>0;
	b=(LPC_GPIO3->FIOPIN &(1<<25))>>24;
	c=(LPC_GPIO1->FIOPIN &(1<<18))>>16;
	d=(LPC_GPIO3->FIOPIN &(1<<26))>>23;
	s= 15-(a+b+c+d);			//
#elif PCBissue==2
	a=(LPC_GPIO0->FIOPIN &(1<<18))>>18;
	b=(LPC_GPIO4->FIOPIN &(1<<29))>>28;
	c=(LPC_GPIO1->FIOPIN &(1<<25))>>23;
	d=(LPC_GPIO1->FIOPIN &(1<<27))>>24;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &(1<<28))>>28;
	b=(LPC_GPIO0->FIOPIN &(1<<25))>>24;
	c=(LPC_GPIO0->FIOPIN &(1<<27))>>25;
	d=(LPC_GPIO0->FIOPIN &(1<<26))>>23;
	s= 15-(1+b+4+d);			//a,b fixed as high.
	s=0;
#endif
	return s|r<<4;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read input change.
///@param void
///@return 30-37 depending on input state change or 0xFF if no change.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte	inputChange(void)
{
	static volatile byte InputState=0;
	byte	a,b,c,d;

#if PCBissue==3
	LPC_GPIO2->FIODIR&=~(1<<11);			//internal
	LPC_GPIO0->FIODIR&=~(1<<21);			//external tip
	LPC_GPIO2->FIODIR&=~(1<<12);			//external mid
	a=(LPC_GPIO2->FIOPIN &(1<<11))>>6;	//bit 5	//bit 0=>>11;		//INTERNAL
	b=(~LPC_GPIO0->FIOPIN &(1<<21))>>17; 	//bit 4	//bit 1=>>0;		//EXTERNAL
	c=(~LPC_GPIO2->FIOPIN &(1<<12))>>12;	//bit 0	//bit 2=>>11;		//EXTERNAL MID/connected
#elif PCBissue==2
	LPC_GPIO2->FIODIR&=~(1<<11);
	LPC_GPIO0->FIODIR&=~(1<<1);
	LPC_GPIO2->FIODIR&=~(1<<13);
	a=(LPC_GPIO2->FIOPIN &(1<<11))>>6;	//bit 5	//bit 0=>>11;		//INTERNAL
	b=(~LPC_GPIO0->FIOPIN &(1<<1))<<3; 	//bit 4	//bit 1=>>0;		//EXTERNAL
	c=(~LPC_GPIO2->FIOPIN &(1<<13))>>13;	//bit 0	//bit 2=>>11;		//EXTERNAL MID/connected
#endif
	d=a|b|c|0xe;						//0xe sets bits 1,2,3 for TECLA spec.
	if (d!=InputState){
		InputState=d;
//		LastInputTime=LPC_TIM2->TC;			//store time of last change of input
		return 0x80|d;						//bit 7 set indicates change.
	}
	else return d;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read time for last input change.
///@param void
///@return time in ms since last input change.
/////////////////////////////////////////////////////////////////////////////////////////////////
//PUBLIC word	inputTime(void)
//{
//		return LPC_TIM2->TC-LastInputTime;			//time in us since last input change.
//}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief ErrorSetTime used with ErrorReset  to time out infinite loops.
///@param int timeout. How long to time out in us.
///@param int error. Error number, identifies loop where error ocurred.
///uses Timer2.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void ErrorTimeOut(word timeout)
{
	ErrorResetTime=timeout+LPC_TIM2->TC;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief ErrorReset used to time out infinite loops.
///@param int time. How long to time out in us.
///@param int error. Error number, identifies loop where error ocurred.
///uses Timer2.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////




PUBLIC void ErrorReset(int error)
	{
	if(LPC_TIM2->TC > ErrorResetTime)
	{
		NVIC_SystemReset();						//system reset
	}
	}

PUBLIC void timer2CPU4(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 1-1; //(4MHz/4)/1 counts at 1MHz. Max time 4295s
	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
}

PUBLIC void timer2CPU12(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 3-1; //(12MHz/4)/3 counts at 1MHz. Max time 4295s
	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
}

PUBLIC void timer2CPU100(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
	LPC_TIM2->PR = 25-1; //(100Mhz/4)/25 counts at 1MHz. Max time 4295s
	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
}








/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief delay of time us.
///@param int:time in us
///@return void
///
///uses main clock, but no interrupts.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	us(unsigned int time_us)
{

//change to use TIMER2, always clocks at 1MHz

	time_us=time_us+LPC_TIM2->TC;
	while (time_us>LPC_TIM2->TC);

}





void BatteryState()
{
if(I2CBATTERY() >=95)//92=92*8*4.88mV =3.591V, 93=3.63V, 95=3.708V
{

	LED1GREEN();//battery good active
}
else
{
	 LED1YELLOW();//battery low active.
}
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief When input changes send new state to BT.
///@param void
///@return 0 if inactive, else 1
/////////////////////////////////////////////////////////////////////////////////////////////////
 int repeatInput(void) {
	 byte in[] = { 'I', ' ' };
	in[1] = inputChange();
	if (in[1] & 0x80) //bit 7 high indicates change
	{

		in[0] = 'H' | (in[1] & 0x01);
		in[1] = (in[1] & 0x7f) | 0x01; //clear bit 7, set bit 1.

		//		in[1]=((in[1]&0x2)<<4)|((in[1]&0x1)<<4);	//TECLA, bit 5 is ext, bit 4 is int, bit 3 is plugged in.
		sendBT(in, sizeof(in));
		return 1;
	}
	return 0;
}






 ///////////////////////////////////////////////////////////
 ///
 ///Watchdog 10s
 void EnableWDT10s()
 {
		LPC_WDT->WDMOD =1<<0;		//watchdog enabled, set only, cannot disable. Bit 1 set for reset, else interrupt.
	// Select internal RC for watchdog
	//select timeout
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDCLKSEL=0;	//IRC source for watchdog. Set bit 31 to fix this selection.
		LPC_WDT->WDFEED=0xAA;
		LPC_WDT->WDFEED=0x55;
		NVIC->ISER[0]|=0x1<<0;	//WDT interrupt.
 }

 ///Watchdog 2s
 void EnableWDT2s()
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
///WDT IRQ Handler traps the watchdog reset.
///This interrupt only occurs once and cannot be cleared, it can however be disabled.
///used only for debugging watchdog.
 //note watchdog after interrupt counts down to 0, then reloads WDTC.
 //if feed, then counts down again.
 void WDT_IRQHandler(void)
 {
	 NVIC_SystemReset();						//system reset
	 NVIC->ICER[0]|=0x1<<0;	//disable WDT interrupt.
 }


