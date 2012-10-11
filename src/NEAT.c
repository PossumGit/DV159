
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
PUBLIC void NEATWR(char r, char d);
PUBLIC char NEATRD(char r);

//External functions

EXTERNAL void initSSP0(void);
EXTERNAL void writeSSP0Byte(char b);
EXTERNAL char readSSP0Byte(void);
EXTERNAL uint32_t SWNEAT;
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC NEATINIT(void) {
	int a,b,c,d,e,f,g,i;
	initSSP0();

	LPC_GPIO_NEATCS FIODIR |= NEATCS; //CHIPEN on NEAT is output.
	LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
	//while(LPC_GPIO2->FIOPIN &(NEATINT);		//wait for INT from RX

	NEATRESET();
//	NEATWR(2,0xA0);			//reset read
//	NEATWR(1,0x01);			//reset NEAT module.


}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief readNEAT checks if NEAT available and reads if available and sends to BT TX buffer.
///@param void
///@return 0 if no read, 1 if read.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int readNEAT(void)
{	char a,b,c,d,e,f;
	int r=0,i;
	char NEAT[0x28];
	int NEATlength=0x21;
//	if ((SWNEAT==1)||((LPC_GPIO_NEATINT FIOPIN) &(NEATINT))==0)
				{
				SWNEAT=0;

				NEAT[2]=NEATRD(0x61);
				NEAT[1]=NEATRD(0x60);
				for (i=3;i<0x20;i++)
				{


				NEAT[i]=NEATRD(0x60+i);
				}

				NEATWR(2,0xA0);			//reset read
	//			NEATWR(1,0x01);			//reset NEAT module.

				NEAT[0]='N';
				NEATlength=1;
			sendBT(NEAT, NEATlength);
				r=1;
				NEATRESET();
				}
	return r;
}





void NEATRESET()
{
	int a,b,c,d,e,f,g;
	int i;
	initSSP0();
	LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
	LPC_GPIO_NEATCS FIODIR |=NEATCS; //CHIPEN on NEAT.
	LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
	us(10000);			//10ms

	NEATWR(1,1);				//reset NEAT/as power up reset.


	{
		us(200000);
	}
	NEATWR(2,0xA0);			//reset receiver

	us(1000);

}


void NEATTX(char battery, char alarm, int ID)
{	int a,b,c,d;

//	NEATRESET();
	a=NEATRD(3);
	while (0x80&NEATRD(3));
	b=NEATRD(3);
	NEATWR(0x08,03);		//number of short preamble packages sent
	NEATWR(0x09,03);		//number of long preamble packages sent.
	NEATWR(0x42,0x00);		//select 40,41 as address
	NEATWR(0x40,ID>>8);		//MSB of transmit ID
	NEATWR(0x41,ID);		//LSB of transmit ID
	NEATWR(0x45,alarm);		//alarm type
	NEATWR(0x46,battery);	//battery state



	c=NEATRD(3);
	NEATWR(0x03,0x80);		//transmit code.
	while (0x80&NEATRD(3));

	d=NEATRD(3);
}



PUBLIC void NEATWR(char address, char data)
{
	//r is register
	//d is data to write
	char a,b;
			us(200);
			LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
			us(200);
			writeSSP0Byte(address&0x7F); //transmit control
			a=readSSP0Byte();
			us(200);
			writeSSP0Byte(data); //transmit
			b=readSSP0Byte();
			us(200);
			LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
			us(20000);
}

PUBLIC char NEATRD(char address)
{
	//r is register
		//d is data read
	char data,a;

			us(200);
			LPC_GPIO_NEATCS FIOCLR = NEATCS; //NEAT enable
			us(200);
			writeSSP0Byte(address|0x80); //transmit control
			readSSP0Byte();
			us(200);
			writeSSP0Byte(0xFF); //transmit
			data = readSSP0Byte();
			us(200);
			writeSSP0Byte(0xFF); //transmit
			a = readSSP0Byte();
			LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
	return data;
}

