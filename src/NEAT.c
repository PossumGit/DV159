
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

//Private global variables

//External variables



//Public functions
PUBLIC void NEATWR(byte r, byte d);
PUBLIC byte NEATRD(byte r);
PUBLIC void NEATRESET();
PUBLIC void NEATTX(byte battery, byte alarm, word ID);
PUBLIC void NEATINIT(void);
//External functions

EXTERNAL void initSSP0(void);
EXTERNAL void writeSSP0Byte(byte b);
EXTERNAL byte readSSP0Byte(void);
EXTERNAL word SWNEAT;
EXTERNAL void sendBT(byte* , unsigned int );
EXTERNAL void	us(unsigned int time_us);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATINIT initialises NEAT on power up.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void NEATINIT(void) {

	initSSP0();

	LPC_GPIO_NEATCS FIODIR |= NEATCS; //CHIPEN on NEAT is output.
	LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
	//while(LPC_GPIO2->FIOPIN &(NEATINT);		//wait for INT from RX

	NEATRESET();
//	NEATWR(2,0xA0);			//reset read
//	NEATWR(1,0x01);			//reset NEAT module.
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief readNEAT reads NEAT receive alarm registers available and sends to BT TX buffer.
///@param void
///@return 0 if no read, 1 if read.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int readNEAT(void)
{
	int r=0,i;
	byte a;
	byte NEAT[0x28];
	byte NEATASCII[5];
	unsigned int NEATlength=0x21;
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

				NEATASCII[0]='N';
				a=0x30+(NEAT[1]>>4);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[1]=a;
				a=0x30+(NEAT[1]&0xF);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[2]=a;
				a=0x30+(NEAT[2]>>4);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[3]=a;
				a=0x30+(NEAT[2]&0x0F);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[4]=a;
				sendBT(NEATASCII, 5);
	//			NEAT[0]='N';
	//			NEATlength=1;
	//		sendBT(NEAT, NEATlength);
				r=1;
				NEATRESET();
				}
	return r;
}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATRESET resets NEAT board, needed after read/write for reliability.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void NEATRESET()
{


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

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATTX transmits an alarm.
///@param byte battery: 0=empty, 0xFF = full
///@param byte alarm: type of alarm
///@param int ID 16 bit, any of 65536 codes.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void NEATTX(byte battery, byte alarm, word ID)
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


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATWR write a byte over SPI to NEAT radio
///@param byte address NEAT register to write
///@param byte data NEAT data to write to NEAT register.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void NEATWR(byte address, byte data)
{
	//r is register
	//d is data to write
	byte a,b;
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
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATRD read NEAT board register.
///@param byte address register to read.
///@return byte data held in register.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC byte NEATRD(byte address)
{
	//r is register
		//d is data read
	byte data,a;

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

