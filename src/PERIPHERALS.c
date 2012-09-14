///@name        	Simple peripherals: HEX, SWITCHES, LEDS.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx.h"

//Public variables

PUBLIC uint32_t LastInputTime=0;

//Private variables
PRIVATE char InputState=0;
//External variables

//Private functions

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 GREEN
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED3GREEN(void)
	{
	LPC_GPIO1->FIODIR |= 1 << 19; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= 1 << 22; 		//IR defined as an output.
	LPC_GPIO1->FIOCLR = 1 << 22; 		//
	LPC_GPIO1->FIOSET = 1 << 19; 		//LED3	GREEN
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 Yellow
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED3YELLOW(void)
	{
	LPC_GPIO1->FIODIR |= 1 << 19; 		//IR defined as an output.
	LPC_GPIO1->FIODIR |= 1 << 22; 		//IR defined as an output.
	LPC_GPIO1->FIOSET = 1 << 22; 		//
	LPC_GPIO1->FIOCLR = 1 << 19; 		//LED3 YELLOW
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn LED3 off
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	LED3OFF(void)
	{
	LPC_GPIO1->FIOCLR = 1 << 22; 		//
	LPC_GPIO1->FIOCLR = 1 << 19; 		//LED3 OFF
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read HEX switches
///@param void
///@return char with value equal to HEX2<<4|HEX1.
///
///HEX1 HEX2
///0	0	USB
///F	0	BT discover.
///E    0   BT factory reset.
///note on prototype bit 4,5 (HEX2 bit 0,1)lack pullup, so only use as 0 for now.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC char HEX(void)
{
	char a,b,c,d,r,s;
	a=(LPC_GPIO0->FIOPIN &1<<18)>>18;
	b=(LPC_GPIO4->FIOPIN &1<<29)>>28;
	c=(LPC_GPIO1->FIOPIN &1<<25)>>23;
	d=(LPC_GPIO1->FIOPIN &1<<27)>>24;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &1<<28)>>28;
	b=(LPC_GPIO0->FIOPIN &1<<25)>>24;
	c=(LPC_GPIO0->FIOPIN &1<<27)>>25;
	d=(LPC_GPIO0->FIOPIN &1<<26)>>23;
	s= 15-(1+b+4+d);			//a,b fixed as high.



///todo mdify when pullups fitted
	return r|s<<4;
}
/*
 *
 *
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read input
///@param void
///@return 30-37 depending on input state.
PUBLIC char	INPUT(void)
{
	char	a,b,c,d;
		a=1^(LPC_GPIO2->FIOPIN &1<<11)>>11;
		b=(LPC_GPIO0->FIOPIN &1<<1)>>0;
		c=(LPC_GPIO2->FIOPIN &1<<13)>>11;
		d=a|b|c;
		return d;	//return current state
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read input change.
///@param void
///@return 30-37 depending on input state change or 0xFF if no change.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC char	inputChange(void)
{
	char	a,b,c,d;
	a=(LPC_GPIO2->FIOPIN &1<<11)>>6;	//bit 5	//bit 0=>>11;		//INTERNAL
	b=(~LPC_GPIO0->FIOPIN &1<<1)<<3; 	//bit 4	//bit 1=>>0;		//EXTERNAL
	c=(~LPC_GPIO2->FIOPIN &1<<13)>>13;	//bit 0	//bit 2=>>11;		//EXTERNAL MID/connected
	d=a|b|c|0xe;						//0xe sets bits 1,2,3 for TECLA spec.
	if (d!=InputState){
		InputState=d;
		LastInputTime=LPC_TIM2->TC;			//store time of last change of input
		return 0x80|d;						//bit 7 set indicates change.
	}
	else return d;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read time for last input change.
///@param void
///@return time in ms since last input change.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC uint32_t	inputTime(void)
{
		return LPC_TIM2->TC-LastInputTime;			//time in us since last input change.
}






/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Timer2 used in some timing situations also for going to sleep.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void timer2Start(void)
{
	LPC_SC->PCONP|=1<<22	;	//enable timer 2.
//	LPC_SC->PCLKSEL1 |= 0 << 12; //TIMER1 PREDIVIDE =4 (system clock/4)=default.
	LPC_TIM2->PR = 25; //counts at 1MHz. Max time 4295s

	LPC_TIM2->TCR = 0 | 1 << 1; //disable timer2, reset timer2
	LPC_TIM2->TCR = 1 | 0 << 1; //enable timer2 (start timer2)
}







/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief 1ms delay
///@param void
///@return void
///
///assumes 100MHz main clock.
///uses SysTick clock, but no interrupts.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	ms(void)
{
SysTick->CTRL=1<<0|1<<2;	//enabled| use processor clock
SysTick->LOAD=100000;
while (SysTick->VAL>50000);
while(SysTick->VAL<50000);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief 200us delay
///@param void
///@return void
///
///assumes 100MHz main clock.
///uses SysTick clock, but no interrupts.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void	us200(void)
{
SysTick->CTRL=1<<0|1<<2;	//enabled| use processor clock
SysTick->LOAD=20000;
while (SysTick->VAL>10000);
while(SysTick->VAL<10000);
}



