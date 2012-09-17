
///@name        	NEAT read/write routines.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012


//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx.h"
//Public variables

//Private variables

//External variables


//Private functions
PRIVATE void NEATWR(char r, char d);
PRIVATE char NEATRD(char r);

//External functions

EXTERNAL void initSSP0Flash(void);
EXTERNAL void writeSSP0Byte(char b);
EXTERNAL char readSSP0Byte(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC int readNEAT(void) {
	initSSP0Flash();

		; //wait for not busy
	//set up SPI for flash.
	char a, b, c,d;
	char rx[30];
	int i;

	LPC_GPIO_NEATCS FIODIR |= NEATCS; //CHIPEN on NEAT.
	LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
	us200();


	writeSSP0Byte(0x81); //0x81 = read reset
	rx[0] = readSSP0Byte();
	us200();
	writeSSP0Byte(0xFF); //enable rx.
	rx[1] = readSSP0Byte();


	us200();
	LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
	us200();






	LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
	us200();
	writeSSP0Byte(0x02); //0x80 = read command, 0x01 is register
	rx[2] = readSSP0Byte();
	us200();
	writeSSP0Byte(0xA0); //address 16-23
	rx[3] = readSSP0Byte();


	us200();
	LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
	us200();

	LPC_GPIO_NEATCS FIODIR &= ~(NEATINT); //NEAT INt is input.
	//while(LPC_GPIO2->FIOPIN &1<<00);		//wait for INT from RX
	enableInputInterrupt();
	powerDown();
	b=NEATRD(0x60);
	a=NEATRD(0x61);
	i=a|b<<8;

	if(i==0x9c0f)
	{
	LED1GREEN();
	for (i=1;i<1000;i++)
	{ms();}
	LED1OFF();
	}
	return a|b<<8;




	return c;
}

void NEATRESET()
{
	int i;
	initSSP0Flash();
	LPC_GPIO_NEATCS FIODIR |=NEATCS; //CHIPEN on NEAT.


	NEATWR(1,0);				//reset NEAT/as power up reset.

	for(i=0;i<200;i++)			//120ms wait for reset.
	{
		ms();
	}
	NEATRD(1);				//reset NEAT INT.
}


void NEATTX(char battery, char alarm, int ID)
{	int a,b,c,d;
	a=NEATRD(3);
	while (0x80&NEATRD(3));
	b=NEATRD(3);
	NEATWR(0x08,04);
	NEATWR(0x09,04);
	NEATWR(0x42,0x00);		//select 40,41 as address
	NEATWR(0x40,ID>>8);		//MSB of transmit ID
	NEATWR(0x41,ID);		//LSB of transmit ID
	NEATWR(0x45,alarm);		//alarm type
	NEATWR(0x46,battery);	//battery state



	c=NEATRD(3);
	NEATWR(0x03,0x80);		//transmit code.
	d=NEATRD(3);
}



PRIVATE void NEATWR(char r, char d)
{
	//r is register
	//d is data to write
	char a,b;
			us200();
			LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
			us200();
			writeSSP0Byte(r&0x7F); //transmit control
			a=readSSP0Byte();
			us200();
			writeSSP0Byte(d); //transmit
			b=readSSP0Byte();
			us200();
			LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
}

PRIVATE char NEATRD(char r)
{
	//r is register
		//d is data read
	char d;

			us200();
			LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
			us200();
			writeSSP0Byte(r|0x80); //transmit control
			readSSP0Byte();
			us200();
			writeSSP0Byte(0xFF); //transmit
			d = readSSP0Byte();
			us200();
			LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
	return d;
}

