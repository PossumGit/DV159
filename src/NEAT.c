
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
EXTERNAL volatile word SWNEAT;	///<NEAT flag indicates NEAT interrupt.


//Public functions
PUBLIC void NEATWR(byte , byte );
PUBLIC byte NEATRD(byte );
PUBLIC void NEATRESET(void);
PUBLIC void NEATTX(byte , byte , word);
PUBLIC void NEATINIT(void);
PUBLIC void	NEATOFF(void);
PUBLIC void NEATALARM(void);
//External functions

EXTERNAL void initSSP0(void);
EXTERNAL void writeSSP0Byte(byte );
EXTERNAL byte readSSP0Byte(void);

EXTERNAL void sendBT(byte in[] , unsigned int );
EXTERNAL void	us(unsigned int );
EXTERNAL void disableInputInterrupt(void);
EXTERNAL void enableInputInterrupt(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATINIT initialises NEAT on power up.
///@param void
///@return void
//Rx control register 2:
//Rx On	Meaning
//b7 = 1	Normal mode
//b7 = 0	Power down mode

//Pwr	Meaning
//b6 = 1	Always active
//b6 = 0	Power save mode (will only react on long preambles)

//Rec All	Meaning
//b5 = 1	Receive all ID codes
//b5 = 0	Receive only pre-programmed ID codes

//Ack	Meaning
//b4 = 1	Send ACK to radio transmitter
//b4 = 0	Do not send ACK to radio transmitter

//Rcode	Meaning
//b3 = 1	Radio code received (must be cleared in software)
//b3 = 0	No radio code received. INT output will return to high state.

//Bits 0-2 are reserved and should be set to zero.

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
	byte NEATASCII[0x10];


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

				a=0x30+(NEAT[5]>>4);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[5]=a;
				a=0x30+(NEAT[5]&0xF);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[6]=a;
				a=0x30+(NEAT[6]>>4);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[7]=a;
				a=0x30+(NEAT[6]&0x0F);
				(a>0x39)?(a+=0x07):a;
				NEATASCII[8]=a;
				NEATASCII[9]=' ';




				sendBT(NEATASCII, 10);





	//			NEAT[0]='N';
	//			NEATlength=1;
	//		sendBT(NEAT, NEATlength);
				r=1;
				NEATRESET();
				}
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief NEATOFF turns NEAT off to minimise power consumption if not used.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void	NEATOFF()
{
initSSP0();
LPC_GPIO_NEATCS FIOSET = NEATCS; //NEAT disable
LPC_GPIO_NEATCS FIODIR |=NEATCS; //CHIPEN on NEAT.
LPC_GPIO_NEATINT FIODIR &= ~(NEATINT); //NEAT INt is input.
us(10000);			//10ms
NEATWR(1,1);				//reset NEAT/as power up reset.
us(200000);
NEATWR(2,0x00);			//reset receiver
us(1000);
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
///@brief NEATTX transmits an alarm from position 0x10 preset by NEAT.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void NEATALARM()
{	int a,b,c,d;

	NEATRESET();

	while (0x80&NEATRD(3));		//wait for ready
	NEATWR(0x08,03);		//number of short preamble packages sent
	NEATWR(0x09,03);		//number of long preamble packages sent.
	NEATWR(0x42,0x01);		//select 10,11 as address
//	NEATWR(0x40,ID>>8);		//MSB of transmit ID
//	NEATWR(0x41,ID);		//LSB of transmit ID
//	NEATWR(0x45,0);		//alarm type
//	NEATWR(0x46,0xFF);	//battery state

	NEATWR(0x03,0x80);		//transmit code.

	disableInputInterrupt();
	LPC_WDT->WDTC = 18000000;	//set timeout 10s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
	LPC_WDT->WDTC = 5000000;	//set timeout 5s watchdog timer
	enableInputInterrupt();
	while (0x80&NEATRD(3));

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

	disableInputInterrupt();
	LPC_WDT->WDTC = 10000000;	//set timeout 10s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
	LPC_WDT->WDTC = 5000000;	//set timeout 5s watchdog timer
	enableInputInterrupt();
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

