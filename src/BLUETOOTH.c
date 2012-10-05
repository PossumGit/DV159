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
int maxtime;
int SEQUENCE=0;
int ID=0;
char alarm=0;
char battery=0xFF;
char IRtype,IRrep;
int IRcode;
//External variables
EXTERNAL int Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL uint8_t I2CSlaveBuffer[256];
EXTERNAL uint32_t SWBT;
//Private functions
PRIVATE void
sendBTbuffer(void);
PRIVATE void
receiveBTbuffer(void);
PRIVATE int
waitBTRX( uint32_t);///<wait for data ready in BT receive channel, timeout after uint32_t us.
PUBLIC void
setupBT(void);
//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief process BT commands.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int processBT(void)
    {
    char a;
    int i = 0;



    char ACK[] =
	{
	'A'
	};
    char NACK[] =
	{
	'N'
	};


#if PCBissue==3
    char I[] =
 	{
	"Hub version 0.1, PCB version 3."
 	};
#elif PCBissue==2						//issue 2 PCB
    char I[] =
 	{
	"Hub version 0.1, PCB version 2."
 	};
#endif



    char W[] =
	{
	'W'
	};
    if (rxstart != rxend)
	{
	a = rx[rxstart++];
	switch (SEQUENCE)//a sequence of N,alarm, battery, IDMSB, IDLSB.
	{
	case 1:
	{
		battery=a;
		SEQUENCE=2;
		break;
	}
	case 2:
	{
		alarm=a;
		SEQUENCE=3;
		break;
	}
	case 3:
	{
		ID=a<<8;
		SEQUENCE=4;
		break;
	}
	case 4:
	{
		ID=ID|a;


//		NEATTX(0xFF,0x01,0x1234);		//battery state, LARM type, ID(16 bits)
//		NEATTX(0xEE,0x08,0x2345);		//battery state, ALARM type, ID(16 bits)
		NEATTX(battery,alarm,ID);
		SEQUENCE=0;
		sendBT(ACK, sizeof(ACK));
		break;
	}
	case 0x100:
	{
		IRtype=a;
		SEQUENCE=0x101;
		break;
	}
	case 0x101:

	{	IRrep=a;
		SEQUENCE=0x102;
		break;
	}
	case 0x102:
	{	IRcode=a<<8;
		SEQUENCE=0x103;
		break;
	}
	case 0x103:
	{
		IRcode=IRcode|a;
		IRsynthesis(IRtype,IRrep,IRcode);
		sendBT(ACK, sizeof(ACK));
		SEQUENCE=0;
		break;
	}

	case 0:
	{
	switch (a)
	    {

	case '?':
	    {
	    sendBT(W, 1);
	    break;
	    }
	case 'b':
	case 'B':
	{
		I2CREAD();
		sendBT(I2CSlaveBuffer,8);  //register 0x0a to register 0x11 of DS2745.

		//0A(10): Temperature MSB
		//0B(11): Temperature LSB
		//0C(12): Voltage MSB
		//0D(13): Voltage LSB
		//0E(14): Current MSB
		//0F(15): Current LSB
		//10(16): Accumulated charge MSB
		//11(17): Accumulated charge LSB
		break;
	}
	case 'c':
	case 'C': //capture IR
	    {
	    if (captureIR())
		sendBT(ACK, sizeof(ACK));
	    else
		sendBT(NACK, sizeof(NACK));//BT char or short capture NACKs
	    break;
	    }




	case 'i':
	case 'I':
	    {
	    sendBT(I, sizeof(I)-1);
	    break;
	    }

	case 'N':
	case 'n':
	{

		 SEQUENCE=1;
		break;
	}

	case 'M':
	case 'm':
	{
		NEATTX(0xFF,0x00,0xCAFE);		//battery state, LARM type, ID(16 bits)
		sendBT(ACK, sizeof(ACK));
		break;
	}

	case 'p':
	case 'P': //play IR
	    {

	    playIR();
	    sendBT(ACK, sizeof(ACK));
	    break;
	    }
	    //ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
	case 'r': //read buffer
	case 'R':
	    {

	    sendBTbuffer(); //ends with 4 off 00 bytes=integer 0
	    //so no ACK. Does not expect ACK.
	    break;
	    }



	case 's': //synthesise IR
	case 'S':
	    {
	    	SEQUENCE=0x100;


	    break;
	    }
	    //w is acknowledged with A, then send buffer, ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
	case 'w':
	case 'W':
	    {

	    while (0 == (1 << 6 & LPC_UART1->LSR))
		;//wait for tx data ready

	    LPC_UART1->THR = 'A'; //immediate ACK to indicate ready.
	    if (waitBTRX(3000000))
		{
		sendBT(NACK, sizeof(NACK)); //NACK if wait longer than 3s for BT data
		break; //wait for char, break if 3s timeout.
		}
	    //	while (0 == (1 & LPC_UART1->LSR));				//wait for RX data ready.
	    else
		{
		receiveBTbuffer(); //ends with 4 off 00 bytes = integer 0.
		sendBT(ACK, sizeof(ACK));
		}
	    break;
	    }
	case 'x'://fill Buffer with test data.
	case 'X':
	    {
	    i = 0;

	    /*		 Buffer[i++]=0x8<<28|10<<16|400<<4|0x0<<0;//0x08=header,1000=pulsewidth, 2632=period,0=next word = IData.

	     Buffer[i++]=1000;
	     Buffer[i++]=1200;
	     Buffer[i++]=1400;
	     Buffer[i++]=1600;
	     Buffer[i++]=1720;
	     Buffer[i++]=1900;
	     Buffer[i++]=2050;
	     Buffer[i++]=2150;
	     Buffer[i++]=2250;
	     Buffer[i++]=0x00;		//end of data.
	     */

	    //using symbols +repeat
	    Buffer[i++] = 0x8 << 28 | 10 << 16 | 200 << 4 | 0x0 << 0;//0x08=header,1000=pulsewidth, 2632=period,0=next word = IData.
	    //			 Buffer[i++]=0xc<<28|0x2<<25|20<<8|0xff<<0;//SUBSIDIARY 20s if released, 255s if pressed, wait for press.
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9000ed10;//DATA1 0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9000ed10;//DATA1 0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0xc << 28 | 0x0 << 25 | 300; //INPUT|Delay| 300*10ms
	    //			 Buffer[i++]=0x90858810;//long gap SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    //			 Buffer[i++]=0xc<<28|0x00<<25;//ABORT if released.
	    //			 Buffer[i++]=0xc<<28|0x2<<25|20<<8|3<<0;//SUBSIDIARY 20s if released, 3s if pressed.
	    Buffer[i++] = 0xb << 28 | 0 << 25 | 0x20 << 17 | 0x1;//REPEAT|COUNTER|COUNT|ADDRESS
	    Buffer[i++] = 0x00; //end of data.


	    //using symbols
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9000ed10;//DATA1 0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9000ed10;//DATA1 0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x90016c10;//DATA0 0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//SYNC 0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x00; //end of data.


	    //using symbol bank and DATA words.
	    Buffer[i++] = 0x8 | 0x524 | 0xa48 << 4 | 0x3 << 0;//0x08=header,0x524=pulsewidth, 0xa48=period, 3=skip 3 to start of data
	    Buffer[i++] = 0x90016c10;//0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9000ed10;//0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0x9002e810;//0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals)
	    Buffer[i++] = 0xa1211131;//0xA=data,1=symbol in Buffer[header+1], 2=symbol in buffer[header+2] etc.
	    Buffer[i++] = 0xa3121113;//0xA=data,1=symbol in Buffer[header+1], 2=symbol in buffer[header+2] etc.
	    Buffer[i++] = 0x00; //end of data.
	    sendBT(ACK, sizeof(ACK));//send ack.

	    break;
	    }

	    }
	}
	}
	return 1;
	}
    return 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms clear all rweceived data.
///@param void
///@return void
///
/// read all available chars from bluetooth.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void clearBT(void)
    {
    int b;
    for (;;)
	{
	if (waitBTRX(100000))
	    break; //wait for char, break if 100ms timeout.
	b = LPC_UART1->RBR;
	}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT wait until RX char is ready
///@param void
///@return 1 if char, 0 if timeout.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE int waitBTRX(uint32_t us)
    {
    uint32_t p, r, s;

    p = LPC_TIM2->TC; //current value of timer 2 in us.

    while (1)
	{
	r = LPC_TIM2->TC;
	if (r - p > maxtime)
	    maxtime = r - p;

	if (r - p > us) //1s timeout. measured timeout max was 149937 timer counts (=us).
	    return 1;
	s = LPC_UART1->LSR;
	if (1 & s)
	    return 0; //	byte is ready.
	}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 receive Buffer data
///@param void
///@return void
///
///ends with integer 0
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void receiveBTbuffer(void)
    {

    int a, b, c, d, e, i, x;
    char Dollar = ' ';
    maxtime = 0;
    uint32_t m;
    for (i = 0; i < CaptureMax; i++)
	{
	if (waitBTRX(1000000))
	    break; //wait for char, break if 100ms timeout.
	a = LPC_UART1->RBR;
	if (a == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))
		break; //wait for char, break if 100ms timeout.
	    x = LPC_UART1->RBR;
	    }

	if (waitBTRX(1000000))
	    break; //wait for char, break if 100ms timeout.
	b = LPC_UART1->RBR;
	if (b == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))
		break; //wait for char, break if 100ms timeout.
	    x = LPC_UART1->RBR;
	    }

	if (waitBTRX(1000000))
	    break; //wait for char, break if 100ms timeout.
	c = LPC_UART1->RBR;
	if (c == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))
		break; //wait for char, break if 100ms timeout.
	    x = LPC_UART1->RBR;
	    }
	Dollar = c;
	if (waitBTRX(1000000))
	    break; //wait for char, break if 100ms timeout.
	d = LPC_UART1->RBR;
	if (d == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))
		break; //wait for char, break if 100ms timeout.
	    x = LPC_UART1->RBR;
	    }

	e = a << 24 | b << 16 | c << 8 | d;
	Buffer[i] = e;
	if (e == 0)
	    break;
	}
    m = LPC_TIM2->TC;
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
PRIVATE void sendBTbuffer(void)
    {
    char a;
    int i;
    for (i = 0; (Buffer[i] != 0) && i < CaptureMax; i++)
	{
	while ((0 == (1 << 6 & LPC_UART1->LSR)))
	    ;//data available and buffer available
	a = Buffer[i] >> 24; //big endian.
	LPC_UART1->THR = a;
	if (a == '$')
	    {
	    while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	    LPC_UART1->THR = 0xFF; //if char is $, send an extra 0xFF after the $.
	    }

	while ((0 == (1 << 6 & LPC_UART1->LSR)))
	    ;//data available and buffer available
	a = Buffer[i] >> 16; //big endian.
	LPC_UART1->THR = a;
	if (a == '$')
	    {
	    while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	    LPC_UART1->THR = 0xFF;//if char is $, send an extra 0xFF after the $.
	    }

	while ((0 == (1 << 6 & LPC_UART1->LSR)))
	    ;//data available and buffer available
	a = Buffer[i] >> 8; //big endian.
	LPC_UART1->THR = a;
	if (a == '$')
	    {
	    while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	    LPC_UART1->THR = 0xFF;//if char is $, send an extra 0xFF after the $.
	    }

	while ((0 == (1 << 6 & LPC_UART1->LSR)))
	    ;//data available and buffer available
	a = Buffer[i] >> 0; //big endian.
	LPC_UART1->THR = a;
	if (a == '$')
	    {
	    while ((0 == (1 << 6 & LPC_UART1->LSR)))
		;//data available and buffer available
	    LPC_UART1->THR = 0xFF;//if char is $, send an extra 0xFF after the $.
	    }
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
PUBLIC void sendBT(char istat[], int ilength)
    {
    int i;
    for (i = 0; i < ilength; i++)
	{
	tx[(txend++) % txlen] = istat[i];
	if (txend == txstart)
	    {
	    txstart = (txstart + 1) % txlen;
	    txoverflow = 1;
	    }
	}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 Receiver data.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int rxtxBT(void)
    {
    int s, r,a;
	SystemCoreClockUpdate ();
	s=SystemCoreClock; //4MHz
    r = 0;
    s = LPC_UART1->LSR;
    while (1 & LPC_UART1->LSR)
	{
	rx[(rxend++) % rxlen] = LPC_UART1->RBR;
	if (rxend == rxstart)
	    {
	    rxstart = (rxstart + 1) % rxlen;
	    rxoverflow = 1;
	    }
	r = 1;
	}
    a=LPC_UART1->LSR;
    while ((txstart != txend) && (0 != (1 << 5 & LPC_UART1->LSR)))//data available and buffer available
	{
	LPC_UART1->THR = tx[(txstart++) % txlen];
	r = 1;
	}
    return r;

    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 Receiver data. Transmit only
///@param void
///@return void
///for use in IR capture and replay.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void txBT(void)
    {

    while ((txstart != txend) && (0 != (1 << 5 & LPC_UART1->LSR)))//data available and buffer available
	{
	LPC_UART1->THR = tx[(txstart++) % txlen];
	}
    }



PUBLIC void BTWAKE(void)
{
	char WAKE[] = { 'W' };

	if (SWBT) {
	SWBT=0;
	sendBT(WAKE, sizeof(WAKE));
//	txshortBT(WAKE, 1);
}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT transmit short(<16) to BT module RN42/RN41 Receiver data.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void txshortBT(char istat[], int ilength)
    {


	    int i;





    while (0 == (1 << 6 & LPC_UART1->LSR))//wait until FIFO buffer is empty.
    {
    }
    for (i = 0; i < ilength; i++)		//now send short message.
	{
	LPC_UART1->THR = istat[i];
	}




    }



////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BTState returns 0 if busy, else returns 1
///@param void.
///@return 0 if busy, 1 if free.
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int BTState(int a)
    {
    if ((LPC_UART1->LSR & (0x1 << 6 | 0x1 << 0)) ^ 0x1 << 0)
	return a + 1; //if TXempty(bit 6) and RXempty(~bit 0) return 1.
    else
	return 0;
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief initialise ports for BT module RN42/RN41
///@param void.
///@return void.
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initBT(void)
    {
    LPC_GPIO_BTMASTER FIOCLR = BTMASTER; //MASTER PIO6. OUT LOW = SLAVE
    LPC_GPIO_BTMASTER FIODIR |= BTMASTER; //MASTER PIO6. OUT LOW = SLAVE


    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY; //FACTORY RESET PIO4. OUT Set this switch ON, power up unit, and toggle the switch from
    //ON to OFF 3 times to return the unit to factory settings.
    LPC_GPIO_BTFACTORY FIODIR |= BTFACTORY; //FACTORY RESET PIO4. OUT Set this switch ON, power up unit, and toggle the switch from
    //ON to OFF 3 times to return the unit to factory settings.



    LPC_GPIO_BTDISCOVERY FIOCLR = BTDISCOVERY; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
    //PIO7 Vcc (=low) BAUD rate 115K
    LPC_GPIO_BTDISCOVERY FIODIR |= (BTDISCOVERY); //DISCOVERY PIO3. OUT HIGH=auto-discovery.

    LPC_GPIO_BTDATASTATUS FIODIR &= ~(BTDATASTATUS); //DATASTATUS PIO8. IN RF DATA

    LPC_GPIO_BTCSTATUS FIODIR &= ~(BTCSTATUS); //CSTATUS. IN PIO5 toggles depending on status, LED drive.

    LPC_GPIO_BTCONNECTED FIODIR &= ~(BTCONNECTED); //CONNECTED PIO2. INPUT HIGH if CONNECTED.



    LPC_GPIO_BTSPICS FIOCLR = (BTSPICS);		//BTSPI is high with NC.
    LPC_PINCON->PINMODE3|=2<<31;			//no pullup or pulldown.
    LPC_GPIO_BTSPICS FIODIR  &= ~(BTSPICS);			//input



    LPC_GPIO_BTRESET FIOSET = BTRESET; //NRESET OUT High, Low to reset.
      LPC_GPIO_BTRESET FIODIR |= BTRESET; //NRESET OUT High, Low to reset.



  //  SysTick->CTRL = 1 << 0 | 1 << 2; //enabled| use processor clock
    //	clearBT();
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Normal reset for BT module RN42/RN41
///@param void.
///@return void.
///
/// not the same as power up.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void resetBT()
    {
    int i;
    LPC_GPIO_BTRESET FIOCLR = BTRESET; //NRESET OUT High, Low to reset.
    us(10000);
    LPC_GPIO_BTRESET FIOSET = BTRESET; //NRESET OUT High, Low to reset.

	us(100000); //10ms delay
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Factory reset for BT module RN42/RN41
///@param void.
///@return void.
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void factoryBT(void)
    {
    int i;
    LED1GREEN();
  	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
 	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
 	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
  	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
	us(500000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
	us(500000); //1s delay
    LED1OFF();
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief enable discovery mode for BT module RN42/RN41
///@param void.
///@return void.
///
///exit when HEX!=0x0F
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void discoverBT(void)
    {
    initBT();
    LPC_GPIO_BTDISCOVERY FIOSET = BTDISCOVERY; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
    while (HEX() == 0x0F)
	{

	}
    LPC_GPIO_BTDISCOVERY FIOCLR = BTDISCOVERY; //DISCOVERY PIO3. OUT LOW=disable auto-discovery.
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief setup command config unlimited time
///@param void.
///@return void.
///
///Only execute at the start of the program. No checks for wrap around on rx buffer.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void setupBT(void)
    {
    int i,s;
	SystemCoreClockUpdate ();
	s=SystemCoreClock; //4MHz
    char CMD[] =
	{
	'$', '$', '$'
	}; //,'\r','\n'};    \r is CR, \n is LF
    char ENDCMD[] =
	{
	'-', '-', '-', '\r', '\n'
	};
    char NoTimeOut[] =
	{
	'S', 'T', ',', '2', '5', '5', '\r', '\n'
	}; //		Disable config timeout = 255.


    char BTsleep[] =
    {
    	'S','W',',','0','1','0','0','\r','\n'			//number in hex 256*0.625ms=160ms
    };


    LED1GREEN();
    for (i = 0; i < 1000; i++, us(1000))
	; //1s delay

    rxstart = rxend;
    sendBT(CMD, sizeof(CMD));
    for (i = 0; i < 3000; i++)
	{
	us(1000);
	rxtxBT();
	if ((rxstart + 5) % rxlen <= rxend)
	    {
	    if (rx[rxstart] != 'C')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'C') && (rx[(rxstart + 1) % rxlen] == 'M')
		    && (rx[(rxstart + 2) % rxlen] == 'D'))
		{
		LED1YELLOW();
		i = 20000;				//
		}
	    }
	}

    rxstart = rxend;
    sendBT(NoTimeOut, sizeof(NoTimeOut));
    for (i = 0; i < 3000; i++)
	{
	us(1000);
	rxtxBT();
	if ((rxstart + 5) % rxlen <= rxend)
	    {
	    if (rx[rxstart] != 'A')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'A') && (rx[(rxstart + 1) % rxlen] == 'O')
		    && (rx[(rxstart + 2) % rxlen] == 'K'))
		{
		LED1GREEN();
		i = 20000;
		}
	    }
	}

    rxstart = rxend;
    sendBT(BTsleep, sizeof(BTsleep));
    for (i = 0; i < 3000; i++)
	{
	us(1000);
	rxtxBT();
	if ((rxstart + 5) % rxlen <= rxend)
	    {
	    if (rx[rxstart] != 'A')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'A') && (rx[(rxstart + 1) % rxlen] == 'O')
		    && (rx[(rxstart + 2) % rxlen] == 'K'))
		{
		LED1GREEN();
		i = 20000;
		}
	    }
	}
    rxstart = rxend;
    sendBT(ENDCMD, sizeof(ENDCMD));
    for (i = 0; i < 3000; i++)
	{
	us(1000);
	rxtxBT();
	if ((rxstart + 5) % rxlen <= rxend)
	    {
	    if (rx[rxstart] != 'E')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'E') && (rx[(rxstart + 1) % rxlen] == 'N')
		    && (rx[(rxstart + 2) % rxlen] == 'D'))
		{
		LED1YELLOW();
		i = 20000;
		}
	    }
	}
    }

