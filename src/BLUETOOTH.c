///@name        	Bluetooth functions.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines
#define rxlen   2000				//length of rx buffer
#define txlen   2000				//length of rx buffer
//Includes
#include "HUB.h"
#include "lpc17xx_pinsel.h"

//Public variables
PUBLIC char rx[rxlen];
PUBLIC int rxstart = 0;
PUBLIC int rxend = 0;
PUBLIC int rxoverflow = 0;

PUBLIC char tx[txlen];
PUBLIC int txstart = 0;
PUBLIC int txend = 0;
PUBLIC int txoverflow = 0;

//Private variables

//External variables
EXTERNAL int Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
//Private functions
PRIVATE void sendBTbuffer(void);
PRIVATE void receieveBTbuffer(void);
//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


PUBLIC void processBT(void) {
	char a;
	char D[] = { 'A'};
	char I[] = {"Hub version 0.0, PCB version 2."};
	if (rxstart != rxend) {
		a = rx[rxstart++];
		switch (a) {
		case 'i':
		case 'I':
		{
			sendBT(I, sizeof(I));
			break;
		}

		case 'c':
		case 'C': //capture IR
		{
			captureIR();
			sendBT(D, sizeof(D));

			break;
		}
		case 'p':
		case 'P': //play IR
		{

			playIR();
			sendBT(D, sizeof(D));
			break;
		}
		//ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
		case 'r': //read buffer
		case 'R': {

			sendBTbuffer();		//ends with 4 off 00 bytes=integer 0

			break;
		}

		//w is acknowledged with A, then send buffer, ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
		case 'w':
		case 'W': {

			while (0 == (1 << 6 & LPC_UART1->LSR));//wait for data ready.
				LPC_UART1->THR = 'A';				//send A for ack.

			receieveBTbuffer();			//ends with 4 off 00 bytes = integer 0.

			break;
		}

		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 receive Buffer data
///@param void
///@return void
///
///ends with integer 0
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void receieveBTbuffer(void) {

	int a,b,c,d,e,i,s,t;
	for (i = 0; i < CaptureMax; i++) {
		while (0 == (1 & LPC_UART1->LSR));		//wait for byte to be ready.
		s=LPC_UART1->LSR;
		a = LPC_UART1->RBR ;
		while (0 == (1 & LPC_UART1->LSR));
		t=LPC_UART1->LSR;
		b= LPC_UART1->RBR ;
		while (0 == (1 & LPC_UART1->LSR));

		c = LPC_UART1->RBR;
		while (0 == (1 & LPC_UART1->LSR));

		d= LPC_UART1->RBR;
		e= a<<24|b<<16|c<<8|d;
		Buffer[i]=e;
		if (e == 0)
			break;
	}
	for (; i < CaptureMax; i++) //fill rest of Buffer with 0.
	{
		Buffer[i] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 transmit Buffer until 0 data
///@param void
///@return void
///
///finally sends integer 0.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void sendBTbuffer(void) {
	char a;
	int i;
	for (i = 0; Buffer[i] != 0; i++) {
		while ((0 == (1 << 6 & LPC_UART1->LSR)))
			;//data available and buffer available
		a = Buffer[i] >> 24; //big endian.
		LPC_UART1->THR = a;
		while ((0 == (1 << 6 & LPC_UART1->LSR)))
			;//data available and buffer available
		a = Buffer[i] >> 16; //big endian.
		LPC_UART1->THR = a;
		while ((0 == (1 << 6 & LPC_UART1->LSR)))
			;//data available and buffer available
		a = Buffer[i] >> 8; //big endian.
		LPC_UART1->THR = a;
		while ((0 == (1 << 6 & LPC_UART1->LSR)))
			;//data available and buffer available
		a = Buffer[i] >> 0; //big endian.
		LPC_UART1->THR = a;
	}
	//finalise with 4 off 00.
	while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	LPC_UART1->THR = 0;
	while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	LPC_UART1->THR = 0;
	while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	LPC_UART1->THR = 0;
	while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	LPC_UART1->THR = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 transmit data
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void sendBT(char istat[], int ilength) {
	int i;
	for (i = 0; i < ilength; i++) {
		tx[(txend++) % txlen] = istat[i];
		if (txend == txstart) {
			(txstart++) % txlen;
			txoverflow = 1;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 Receiver data.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void rxtxBT(void) {
	int s;
	s = LPC_UART1->LSR;
	while (1 & LPC_UART1->LSR) {
		rx[(rxend++) % rxlen] = LPC_UART1->RBR;
		if (rxend == rxstart) {
			(rxstart++) % rxlen;
			rxoverflow = 1;
		}
	}
	while ((txstart != txend) && (0 != (1 << 6 & LPC_UART1->LSR)))//data available and buffer available
	{
		LPC_UART1->THR = tx[(txstart++) % txlen];
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
///@brief initialise ports for BT module RN42/RN41
///@param void.
///@return void.
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initBT(void) {
	LPC_GPIO1->FIOCLR = 1 << 1; //MASTER PIO6. OUT LOW = SLAVE
	LPC_GPIO1->FIOSET = 1 << 4; //NRESET OUT High, Low to reset.
	LPC_GPIO1->FIOSET = 1 << 0; //FACTORY RESET PIO4. OUT Set this switch ON, power up unit, and toggle the switch from
	//ON to OFF 3 times to return the unit to factory settings.
	LPC_GPIO1->FIOCLR = 1 << 9; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
	//PIO7 Vcc (=low) BAUD rate 115K
	LPC_GPIO1->FIODIR |= 1 << 1; //MASTER PIO6. OUT LOW = SLAVE
	LPC_GPIO1->FIODIR |= 1 << 4; //NRESET OUT High, Low to reset.
	LPC_GPIO1->FIODIR &= ~(1 << 14); //DATASTATUS PIO8. IN RF DATA
	LPC_GPIO1->FIODIR |= 1 << 0; //FACTORY RESET PIO4. OUT Set this switch ON, power up unit, and toggle the switch from
	//ON to OFF 3 times to return the unit to factory settings.
	LPC_GPIO1->FIODIR &= ~(1 << 8); //CSTATUS. IN PIO5 toggles depending on status, LED drive.
	LPC_GPIO1->FIODIR |= 1 << 9; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
	LPC_GPIO1->FIODIR &= ~(1 << 10); //CONNECTED PIO2. INPUT HIGH if CONNECTED.
}

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Normal reset for BT module RN42/RN41
///@param void.
///@return void.
///
/// not the same as power up.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void resetBT() {
	int i;
	LPC_GPIO1->FIOCLR = 1 << 4; //NRESET OUT High, Low to reset.
	ms();
	LPC_GPIO1->FIOSET = 1 << 4; //NRESET OUT High, Low to reset.
	for (i = 0; i < 10; i++)
		ms(); //10ms delay
}

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Factory reset for BT module RN42/RN41
///@param void.
///@return void.
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void factoryBT(void) {
	int i;
	LED3GREEN();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOCLR = 1 << 0;
	LED3YELLOW();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOSET = 1 << 0;
	LED3GREEN();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOCLR = 1 << 0;
	LED3YELLOW();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOSET = 1 << 0;
	LED3GREEN();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOCLR = 1 << 0;
	LED3YELLOW();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LPC_GPIO1->FIOSET = 1 << 0;
	LED3GREEN();
	for (i = 0; i < 500; i++)
		ms(); //1s delay
	LED3OFF();
}

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief enable discovery mode for BT module RN42/RN41
///@param void.
///@return void.
///
///exit when HEX!=0x0F
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void discoverBT(void) {
	initBT();
	LPC_GPIO1->FIOSET = 1 << 9; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
	while (HEX() == 0x0F)
		;
	LPC_GPIO1->FIOCLR = 1 << 9; //DISCOVERY PIO3. OUT LOW=disable auto-discovery.
}
