///@name        	Simple peripherals: HEX, SWITCHES, LEDS.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx.h"

//Public variables

//Private variables

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
///note on prototype bit 4,5 (HEX2 bit 0,1)lack pullup, so only use as 0 for now.
PUBLIC char HEX(void)
{
	char a,b,c,d,r,s;
	a=(LPC_GPIO0->FIOPIN &1<<18)>>18;
	b=(LPC_GPIO1->FIOPIN &1<<25)>>24;
	c=(LPC_GPIO4->FIOPIN &1<<29)>>27;
	d=(LPC_GPIO1->FIOPIN &1<<27)>>24;
	r= 15-(a+b+c+d);
	a=(LPC_GPIO0->FIOPIN &1<<28)>>28;
	b=(LPC_GPIO0->FIOPIN &1<<27)>>26;
	c=(LPC_GPIO0->FIOPIN &1<<25)>>23;
	d=(LPC_GPIO0->FIOPIN &1<<26)>>23;
	s= 15-(1+2+c+d);			//a,b fixed as high.

///@todo remove when pullups fitted.
	return r|s<<4;
}


PUBLIC char	INPUT(void)
{
	char	a,b,c;
	a=1^(LPC_GPIO2->FIOPIN &1<<11)>>11;
	b=(LPC_GPIO0->FIOPIN &1<<1)>>0;
	c=(LPC_GPIO2->FIOPIN &1<<13)>>11;
	return a|b|c|0x30;
}





/*
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read HEX1 switch
///@param void
///@return int with value equal to HEX1 switch (0x0-0xF).
PUBLIC int HEX1(void)
{
	int a,b,c,d;
	a=(LPC_GPIO0->FIOPIN &1<<18)>>18;
	b=(LPC_GPIO1->FIOPIN &1<<25)>>24;
	c=(LPC_GPIO4->FIOPIN &1<<29)>>27;
	d=(LPC_GPIO1->FIOPIN &1<<27)>>24;
	return 15-(a+b+c+d);

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read HEX2 switch
///@param void
///@return int with value equal to HEX2 switch (0x0-0xF).
///
///note P0_27 and P0_28 lack pullups, so do not reliably read high.
///use bit 0,1=0 to avoid trouble on prototype board.
PUBLIC int HEX2(void)
{
	int a,b,c,d;
	a=(LPC_GPIO0->FIOPIN &1<<28)>>28;
	b=(LPC_GPIO0->FIOPIN &1<<27)>>26;
	c=(LPC_GPIO0->FIOPIN &1<<25)>>23;
	d=(LPC_GPIO0->FIOPIN &1<<26)>>23;
	return 15-(a+b+c+d);
}

*/
///HEX1 HEX2
///0	0	USB
///F	0	BT discover.


PUBLIC void	msStart(void)
{
	SysTick->CTRL=1<<0|1<<2;	//enabled| use processor clock
	SysTick->LOAD=200000;
}

PUBLIC char	msEnd(void)
{
	if(SysTick->VAL>100000) return 1;	//<100000 equals >1ms
	else	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief 1ms delay
///@param void
///@return void
///
///assumes 100MHz main clock.
///uses SysTick clock, but no interrupts.
PUBLIC void	ms(void)
{
SysTick->CTRL=1<<0|1<<2;	//enabled| use processor clock
SysTick->LOAD=100000;
while (SysTick->VAL>50000);
while(SysTick->VAL<50000);
}
