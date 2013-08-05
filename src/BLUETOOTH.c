///@name        	Bluetooth functions.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012


//Private Defines
#define rxlen   2000				//was 2000 length of rx buffer
#define txlen   2000				//was 2000 length of rx buffer


//Includes
#include "HUB.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <string.h>

//local variables
//needs to be accessible from functions in this file only and to be persistent, but can change in interrupt.

PRIVATE volatile byte rx[rxlen];		// BT RX buffer.
PRIVATE volatile int rxoverflow = 0;	// DEBUG detect overflow BT RX.
PRIVATE volatile byte tx[txlen];		// BT TX buffer.
PRIVATE volatile int txoverflow = 0;	// DEBUG detect overflow BT TX.
PRIVATE volatile unsigned int maxtime;			//DEBUG only, measure BT response time.
				int	start;				//for read by android
				int	length;				//read buffer by android

//Public variables
PUBLIC volatile unsigned int rxstart = 0;		// BT RX.
PUBLIC volatile unsigned int rxend = 0;		// BT RX.
PUBLIC volatile unsigned int txstart = 0;		// BT TX.
PUBLIC volatile unsigned int txend = 0;		// BT TX.
PUBLIC int BTACC=0;
PUBLIC	int BUFLEN=0;
PUBLIC  int SEQUENCE=0;			//used in ProcessBT for NEAT and IR sequence.
PUBLIC int HELDtime=0;
PUBLIC int RELEASEtime=0;
//External variables
EXTERNAL volatile word Buffer[]; ///< Whole of RAM2 is Buffer, reused for audio and IR replay and capture
EXTERNAL volatile byte I2CSlaveBuffer[];///<transfer DS2745 battery state, modified in interrupt.
EXTERNAL volatile word SWBT;				///<Bluetooth interrupt flag, modified in interrupt
//EXTERNAL volatile byte PENDALARM;			///<Emergency alarm flag, cancelled by Bluetooth input, modified in interrupt
EXTERNAL int ALARMtime;
EXTERNAL char STATE;
EXTERNAL int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.

//local functions
PRIVATE void sendBTbuffer(int,int);
PUBLIC int receiveBTbuffer(int,int);
PRIVATE int waitBTRX( word);	///<wait for data ready in BT receive channel, timeout after word us.
void SENDTERM(void);
void SENDBTNT(int start,int length);
int BUFFERSIZE(void);
// public functions
PUBLIC void sendBT(byte a[] , unsigned int );
PUBLIC void sendBTNC(byte a[] , unsigned int );		//send command even if not connected.
PUBLIC int processBT(void);
PUBLIC word rxtxBT(void);
PUBLIC void resetBT();
PUBLIC void setupBT(void);
PUBLIC void initBT(void);
PUBLIC void factoryBT(void);
PUBLIC void txBT(void);
PUBLIC void discoverBT(void);

PUBLIC void	BTQUIET(void);
PUBLIC void	BTON(void);
EXTERNAL void SystemOFF(void);


//External functions
EXTERNAL void streamIR(int);
EXTERNAL void NEATTX(byte battery, byte alarm, word ID);
EXTERNAL void IRsynthesis(byte IRtype, byte IRrep, int IRcode);
EXTERNAL void LED1GREEN(void);
EXTERNAL void LED1YELLOW(void);
EXTERNAL void LED1OFF(void);
EXTERNAL void I2CREAD(void);
EXTERNAL int captureIR(void);
EXTERNAL void playIR(void);
EXTERNAL byte HEX(void);
EXTERNAL void us(unsigned int time_us);
EXTERNAL void CPU12MHz(void);
EXTERNAL disableInputInterrupt(void);
EXTERNAL enableInputInterrupt(void);

EXTERNAL void BatteryState();



//local functions(code)

//
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief process BT commands.
///@param void
///@return int =1 if processed data, 0 if no data to process.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int processBT(void)
    {
    byte BluetoothData;




static  word ID=0;				//NEAT sequence used in ProcessBT.
static  byte alarm=0;			//NEAT sequence used in ProcessBT.
static  byte battery=0xFF;		//NEAT sequence used in ProcessBT.

static byte IRtype,IRrep;		//IR sequence used in ProcessBT.
static int IRcode;				//IR sequence used in ProcessBT.
static char ReportBUFLEN[]="1234A";
char Information[] =
	{
			"QWAYO firmware xxx, PCB x. Copyright Possum 2012-13. \0\0\0\0"
	};

    byte ACK[] =
	{
	'A'
	};
    byte NACK[] =
	{
	'n'
	};
    byte STOP[]=
    {
    		"STOP"
    };


//if (PCBiss==4)
 //   {
//	strcpy(I,"QWAYO firmware xxx, PCB x. Copyright Possum 2012-13. \0\0\0\0");//strcpy copies to first \0 only.
	Information[15]=Version&0xFF;
	Information[16]=(Version>>8)&0xFF;
	Information[17]=(Version>>16)&0xFF;
	Information[24]=0x30+PCBiss&0xFF;		//fails if PCBiss>9.

 //   }
 //   else if (PCBiss==3)
 //   {
//    strcpy(I,"QWAYO firmware xxx, PCB x. Copyright Possum 2012-13. \0\0\0\0");
//	I[15]=Version&0xFF;
//	I[16]=(Version>>8)&0xFF;
//	I[17]=(Version>>16)&0xFF;
//	I[24]=0x30+PCBiss$0xFF;
// 	}





    byte Wakeup[] =
	{
	'W'
	};
    if (rxstart != rxend)
	{
	BluetoothData = rx[rxstart];
	BTACC=1;
	LPC_TIM2->TC = 0;
	rxstart=(rxstart+1)%rxlen;	//mod does  work.
//	rxstart=(rxstart++)%rxlen;	//mod does not work.

//	rxstart=rxstart%rxlen;	//mod does work

	if(rxstart>=rxlen)rxstart=0;

	LED1GREEN();
	switch (SEQUENCE)//a sequence of N,alarm, battery, IDMSB, IDLSB.
	{
	case 1:
	{
		battery=BluetoothData;
		SEQUENCE=2;
		break;
	}
	case 2:
	{
		alarm=BluetoothData;
		SEQUENCE=3;
		break;
	}
	case 3:
	{
		ID=BluetoothData<<8;
		SEQUENCE=4;
		break;
	}
	case 4:
	{
		ID=ID|BluetoothData;


//		NEATTX(0xFF,0x01,0x1234);		//battery state, LARM type, ID(16 bits)
//		NEATTX(0xEE,0x08,0x2345);		//battery state, ALARM type, ID(16 bits)
		NEATTX(battery,alarm,ID);
		SEQUENCE=0;
		sendBT(ACK, sizeof(ACK));
		break;
	}
	case 0x100:
	{
		IRtype=BluetoothData;
		SEQUENCE=0x101;
		break;
	}
	case 0x101:

	{	IRrep=BluetoothData;
		SEQUENCE=0x102;
		break;
	}
	case 0x102:
	{	IRcode=BluetoothData<<8;
		SEQUENCE=0x103;
		break;
	}
	case 0x103:
	{
		IRcode=IRcode|BluetoothData;
		IRsynthesis(IRtype,IRrep,IRcode);
		playIR();
		CPU12MHz();
		sendBT(ACK, sizeof(ACK));
		SEQUENCE=0;
		break;
	}

	case 0x200:
	{

		ALARMtime=BluetoothData;
		SEQUENCE=0x201;
		break;
	}
	case 0x201:
	{
		HELDtime=20000*BluetoothData;
		SEQUENCE=0;
		sendBT(ACK, sizeof(ACK));
		break;
	}

	case 0x300:
	{
		start=BluetoothData;
		SEQUENCE=0x301;
		break;
	}
	case 0x301:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x302;
		break;
	}
	case 0x302:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x303;
		break;
	}
	case 0x303:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x304;
		break;
	}
	case 0x304:
	{
		length=BluetoothData;
		SEQUENCE=0x305;
		break;
	}
	case 0x305:
	{
		length=BluetoothData|(length<<8);
		SEQUENCE=0x306;
		break;
	}
	case 0x306:
	{
		length=BluetoothData|(length<<8);
		SEQUENCE=0x307;
		break;
	}
	case 0x307:
	{
		length=BluetoothData|(length<<8);
		sendBTbuffer(start,length); //ends with 4 off 00 bytes=integer 0
		SEQUENCE=0x0;
		break;
	}

	case 0x400:
	{
//		if (BluetoothData=='Q'||BluetoothData=='q')
//		{
	//		disableInputInterrupt();

//try 4 chunks with gaps
//no gap =15K bytes
//100ms gap= 18K bytes
//200ms gap = 28K bytes
//250ms gap = 29Kbytes
//300ms gap = all data.
//400ms gap = all data, gives some margin.
//500ms gap gives bigger margin, 1.5s added time.
/*
    	SENDBTNT(0,0x800); //ends with 4 off 00 bytes=integer 0
    	us(500000);
    	SENDBTNT(0x800,0x800); //ends with 4 off 00 bytes=integer 0
    	us(500000);
    	SENDBTNT(0x1000,0x800); //ends with 4 off 00 bytes=integer 0
    	us(500000);
    	SENDBTNT(0x1800,0x800); //ends with 4 off 00 bytes=integer 0
     	SENDTERM();

    */
     	SEQUENCE=0;
    // 	enableInputInterrupt();
//		}


	case 0x500:

	{
		start=BluetoothData;
		SEQUENCE=0x501;
		break;
	}
	case 0x501:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x502;
		break;
	}
	case 0x502:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x503;
		break;
	}
	case 0x503:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x00;
		receiveBTbuffer(start, 0x2000); //ends with 4 off 00 bytes = integer 0.
		sendBT(ACK, sizeof(ACK));
//		}




	 	 break;
	}
	case 0x600:

	{
		start=BluetoothData;
		SEQUENCE=0x601;
		break;
	}
	case 0x601:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x602;
		break;
	}
	case 0x602:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x603;
		break;
	}
	case 0x603:
	{
		start=BluetoothData|(start<<8);
		SEQUENCE=0x604;
		break;
	}
	case 0x604:
		{
			length=BluetoothData;
			SEQUENCE=0x605;
			break;
		}
		case 0x605:
		{
			length=BluetoothData|(length<<8);
			SEQUENCE=0x606;
			break;
		}
		case 0x606:
		{
			length=BluetoothData|(length<<8);
			SEQUENCE=0x607;
			break;
		}
		case 0x607:
		{
			SEQUENCE=0x0;
			length=BluetoothData|(length<<8);
			sendBT(ACK, sizeof(ACK));
			txBT();
	//		receiveBTDMA(start, length); //ends with 4 off 00 bytes = integer 0.



			break;
		}

















	case 0:
	{
	switch (BluetoothData)
	    {

	case '?':
	    {
	    sendBT(Wakeup, 1);
	    break;
	    }

	case 'A':
	case 'a':
		break;


	case 'b':
	case 'B':
	{
		I2CREAD();

		I2CSlaveBuffer[0xA]=0;
		I2CSlaveBuffer[0xB]=0;
		I2CSlaveBuffer[0xC]=0;
		I2CSlaveBuffer[0xD]=0;
		I2CSlaveBuffer[0xE]=0;
		I2CSlaveBuffer[0xF]=0;



		sendBT(I2CSlaveBuffer,16);  //register 0x0a to register 0x11 of DS2745.

		//0A(0): Temperature MSB
		//0B(1): Temperature LSB
		//0C(2): Voltage MSB
		//0D(3): Voltage LSB
		//0E(4): Current MSB
		//0F(5): Current LSB
		//10(6): Accumulated charge MSB
		//11(7): Accumulated charge LSB
		//1(8); status
		//xx(9); 1= charging
		//xx(10);	charge confidence 1= new battery, 5= very confident.
		//xx(11); corrected charge MSB
		//xx(12); corrected charge LSB
		//DISCHARGE =(0x800A-correctedcharge)/20
		//

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

	case 'd':
		case 'D': //capture IR
		    {
		 //   	BUFLEN= BUFFERSIZE();
		    	ReportBUFLEN[0]=BUFLEN>>24;
		    	ReportBUFLEN[1]=BUFLEN>>16;
		    	ReportBUFLEN[2]=BUFLEN>>8;
		    	ReportBUFLEN[3]=BUFLEN;
			sendBT(ReportBUFLEN, sizeof(ReportBUFLEN-1));

		    break;
		    }


	case 'i':
	case 'I':
	    {

	    sendBT(Information, sizeof(Information)-1);
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

	case 'l':
	case 'L':
	{
		IRsynthesis('P',4,0x2);		//Plessey  4 repeats, code 3
		playIR();

		CPU12MHz();
		sendBT(ACK, sizeof(ACK));
		break;
	}

	case 'p':
	case 'P': //play IR
	    {

	    playIR();		//convert to 100MHz and disable interrupt.
		CPU12MHz();
	    sendBT(ACK, sizeof(ACK));
	//    LPC_TIM2->TC=3000000;
	    break;
	    }

	case 'q': //read buffer
	case 'Q':
	    {
	    	//try 4 chunks with gaps
	    	//no gap =15K bytes
	    	//100ms gap= 18K bytes
	    	//200ms gap = 28K bytes
	    	//250ms gap = 29Kbytes
	    	//300ms gap = all data.
	    	//400ms gap = all data, gives some margin.
	    	//500ms gap gives bigger margin, 1.5s added time.

	    	int x;

	    	SENDBTNT(0,0x2000); //ends with 4 off 00 bytes=integer 0


	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	 //   	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(1000000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	 //   	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	 //   	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	 //   	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	 //   	us(100000);
	 //   	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	 //   	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(100000);
	//    	SENDBTNT(0,0x100); //ends with 4 off 00 bytes=integer 0
	//    	us(1000000);

//	    	SENDBTNT(0x500,0x500); //ends with 4 off 00 bytes=integer 0
//	    	us(1000000);
//	    	SENDBTNT(0x800,0x200); //ends with 4 off 00 bytes=integer 0
//	    	us(500000);
//	    	SENDBTNT(0xc00,0x400); //ends with 4 off 00 bytes=integer 0
//	    	us(1000000);
//	    	SENDBTNT(0x1800,0x800); //ends with 4 off 00 bytes=integer 0

	     	SENDTERM();
	 	    	break;




	    //so no ACK. Does not expect ACK.

	    }


	    //ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
	case 'r': //read buffer
	case 'R':
	    {
	    	SEQUENCE=0x300;
		    	break;




	    //so no ACK. Does not expect ACK.

	    }


	case 's': //synthesise IR
	case 'S':
	    {
	    	SEQUENCE=0x100;


	    break;
	    }

	case 't': //synthesise IR
	case 'T':
	    {
	    	SEQUENCE=0x200;		//set alarm time.
	    	STATE='H';
	    	BatteryState();

	    break;
	    }


	    //w is acknowledged with A, then send buffer, ending with 4 bytes of 0. transfer must be multiple of 4 bytes.
	case 'w':
	case 'W':
	    {


	    while (0 == (1 << 6 & LPC_UART1->LSR))
		;//wait for tx data ready

	    LPC_UART1->THR = 'A'; //immediate ACK to indicate ready.
	    if (waitBTRX(5000000))
		{
		sendBT(NACK, sizeof(NACK)); //NACK if wait longer than 3s for BT data
		break; //wait for char, break if 3s timeout.
		}
	    //	while (0 == (1 & LPC_UART1->LSR));				//wait for RX data ready.
	    else
		{
		if (receiveBTbuffer(0, 0x2000))sendBT(NACK, sizeof(NACK));
		else

		sendBT(ACK, sizeof(ACK));
		}
	    break;
	    }
	    //go to sleep


//	case 'x':		//copy from BT to buffer from address 1234 (4 bytes bigendian)
//	case 'X':
//	    {
//	    SEQUENCE=0x500;
//	    break;
//	    }
	    //go to sleep


	case 'v':			//copy from BT to buffer from address 1234 and start playing IR.
	case 'V':
	    {


//uses receiveBTDMA, no escape sequence implemented with this.
//	    SEQUENCE=0x600;
	    break;

	    }
	    //go to sleep


	case 'Z':
	{

		 SystemOFF();

		break;
	    }
	    }

	}
	}
//	LED1OFF();
	return 1;
	}
//    LED1OFF();
    return 0;
    }
    }


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT wait until RX char is ready
///@param word time in us to wait in routine
///@return int 1 if char, 0 if timeout.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE int waitBTRX(word us)
    {
    int p, r, s;
 //   LPC_TIM3->TC=0;
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
int receiveBTbuffer(int start, int length)
    {

    int Byte24, Byte16, Byte8, Byte0, ReceivedWord, i, EscapeByte, end;
    byte Dollar = ' ';
    word m;

    maxtime = 0;

#if release==1
		LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif


    i=start;
    end=start+length;
    if((start>=0) && (start<CaptureMax))
    {
if (end>CaptureMax){
	end=CaptureMax;



}
	waitBTRX(5000000) ;		//wait for 5s for first char.
    for (; i<end; i++)
	{

	if (waitBTRX(1000000))  break; //wait for char, break if 100ms timeout.
    	Byte24 = LPC_UART1->RBR;
	if (Byte24 == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000)) break; //wait for char, break if 100ms timeout.
	    EscapeByte = LPC_UART1->RBR;
	    }

	if (waitBTRX(1000000))	    break; //wait for char, break if 100ms timeout.
	Byte16 = LPC_UART1->RBR;
	if (Byte16 == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
	    EscapeByte = LPC_UART1->RBR;
	    }

	if (waitBTRX(1000000))	    break; //wait for char, break if 100ms timeout.
	Byte8 = LPC_UART1->RBR;
	if (Byte8 == '$') //ignore next char.
	    {

	    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
	    EscapeByte = LPC_UART1->RBR;
	    }
	Dollar = Byte8;
	if (waitBTRX(1000000))	    break; //wait for char, break if 100ms timeout.
	Byte0 = LPC_UART1->RBR;
	if (Byte0 == '$') //ignore next char.
	    {
	    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
	    EscapeByte = LPC_UART1->RBR;
	    }

	ReceivedWord = Byte24 << 24 | Byte16 << 16 | Byte8 << 8 | Byte0;
	Buffer[i] = ReceivedWord;
	if (ReceivedWord == 0)	    break;
#if release==1
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif

	}
    while((i==0x2000)&&(ReceivedWord!=0))		//allow for 0000 after full buffer case.
    {
	    if (waitBTRX(1000000))	break;
 //     	while(!(1& LPC_UART1->LSR));	//wait for char.
    	Byte24 = LPC_UART1->RBR;
 	    if (waitBTRX(1000000))	break;
 //     	while(!(1& LPC_UART1->LSR));	//wait for char.
    	Byte16 = LPC_UART1->RBR;
 	    if (waitBTRX(1000000))	break;
 //     	while(!(1& LPC_UART1->LSR));	//wait for char.
    	Byte8 = LPC_UART1->RBR;
 	    if (waitBTRX(1000000))	break;
//      	while(!(1& LPC_UART1->LSR));	//wait for char.
    	Byte0 = LPC_UART1->RBR;
      	ReceivedWord = Byte24 << 24 | Byte16 << 16 | Byte8 << 8 | Byte0;
    	i++;
    }


    BUFLEN=i+1;
    m = LPC_TIM2->TC;
    for (; i < CaptureMax; i++) //fill rest of Buffer with 0.
	{
	Buffer[i] = 0;
	}
    }
#if release==1
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif
		return ReceivedWord;
    }



/*
 /////////////////////////////////////////////////////////////////////////////////////////////////
 ///@brief BT comms with BT module RN42/RN41 receive Buffer data
 ///@param void
 ///@return void
 ///
 ///ends with integer 0
 /////////////////////////////////////////////////////////////////////////////////////////////////
  void receiveBTDMA(int start, int length)
     {

     int ReceivedData, i, x, end;
     word b,c,d,e;


 #if release==1
 		LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
 		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
 		LPC_WDT->WDFEED=0x55;			//watchdog feed

 #endif


     i=start;
     end=start+length;
     if((start>=0) && (start<CaptureMax))
     {
 if (end>CaptureMax){
 	end=CaptureMax;
 }
     for (; i<end; i++)
 	{
     if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
 //  	while(!(1& LPC_UART1->LSR));	//2.6us wait for char.
   	ReceivedData = LPC_UART1->RBR;				//1.9us
    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
 	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
 	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
    if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
 	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
 	Buffer[i] = ReceivedData;
 	if (ReceivedData == 0)   break;
 	}
    while((i==0x2000)&&(ReceivedData!=0))
     {
       if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
     	ReceivedData = LPC_UART1->RBR;
        if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
     	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
        if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
     	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
        if (waitBTRX(1000000))	break; //wait for char, break if 100ms timeout.
 //      	while(!(1& LPC_UART1->LSR));	//wait for char.
     	ReceivedData = LPC_UART1->RBR|ReceivedData<<8;
     	i++;
      }
     BUFLEN=i+1;
     for (; i < CaptureMax; i++) //fill rest of Buffer with 0.
 	{
 	Buffer[i] = 0;
 	}
     }
 #if release==1
 		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
 		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
 		LPC_WDT->WDFEED=0x55;			//watchdog feed
 #endif

     }
*/



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 transmit Buffer until 0 data
///@param void
///@return void
///
///finally sends integer 0.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void sendBTbuffer(int start, int length)
    {
    SENDBTNT(start,length);
    SENDTERM();
    }



void SENDTERM(void)
{
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
	   //  BUFLEN=i+1;
}



void SENDBTNT(int start,int length)
{
    byte a;
    int i;
    int end;

#if release==1
		LPC_WDT->WDTC = 80000000;	//set timeout 40s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif


    if ((start<CaptureMax)&& (start>=0))
    {
    end=start+length;
  //  if(end>=CaptureMax)end=CaptureMax;

   for (i=start;(i <end) && (i<CaptureMax)   ; i++)
 //   for (i = 0; (Buffer[i] != 0) && i < CaptureMax-1; i++)
	{
	   if (i>=CaptureMax)break;
	   if (Buffer[i] == 0)break;
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
#if release==1
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
	}

#if release==1
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
}
}


/////////////////////////////////////////////////////////////////
//find buffer size, return number of bytes that will be sent via BT.
//look for Buffer[x]==0 for end of buffer.
//
//
int BUFFERSIZE(void)
{
	int a=0;
	int b=0;
	int c=4;		//always get 00000000 at end, even if buffer full.
	while(a<CaptureMax)
	{
	b=Buffer[a++];
	if (b==0)break;
	c+=4;
	//extra chars to escape $s.
	if ((b&0xFF)==0x24)
		c++;
	if ((b&0xFF00)==0x2400)
		c++;
	if((b&0xFF0000)==0x240000)
		c++;
	if((b&0xFF000000)==0x24000000)
		c++;

	}
	 return c;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 transmit data
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void sendBT(byte istat[], unsigned int ilength)
    {
    int i,a;
	a=LPC_GPIO0->FIOPIN&(1<<26);		//is bluetooth connected? P0_26 = 1 if connected, = 0 if not connected.
#if release==0
	a=1;		//force always send if in debug mode
#endif
	if (a)

		{
	//only send if BT is connected.
	//note that it takes around 20s for disconnected to be detected.


    for (i = 0; i < ilength; i++)
	{
	tx[(txend++)] = istat[i];
	if(txend>=txlen)txend=0;


	if (txend == txstart)
	    {
	txstart=txstart+1;
	if(txstart>=txlen)txstart=0;
//	    txstart = (txstart + 1) % txlen;		//overflow, lose old data
	    txoverflow = 1;							//record that overflow occurred.
	    }
	}
    }
	else		//disconnected.	Clear TX buffer.
	{
		txstart=0;
		txend=0;
	}
    }



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 transmit data
///@param void
///@return void
//send even if not connected for commands.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void sendBTNC(byte istat[], unsigned int ilength)
    {
    int i,a;

	//only send if BT is connected.
	//note that it takes around 20s for disconnected to be detected.


    for (i = 0; i < ilength; i++)
	{
	tx[(txend++)] = istat[i];
	if(txend>=txlen)txend=0;


	if (txend == txstart)
	    {
	txstart=txstart+1;
	if(txstart>=txlen)txstart=0;
//	    txstart = (txstart + 1) % txlen;		//overflow, lose old data
	    txoverflow = 1;							//record that overflow occurred.
	    }
	}

    }




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 Receiver data.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC word rxtxBT(void)
    {
    int s, r,a;
//	SystemCoreClockUpdate ();
//	s=SystemCoreClock; //4MHz
    r = 0;
    s = LPC_UART1->LSR;
    while (1 & LPC_UART1->LSR)
	{
	rx[rxend++] = LPC_UART1->RBR;
//	rxend=(rxend++)%rxlen;			//inc works, mod does not work

	if(rxend>=rxlen)rxend=0;

	if (rxend == rxstart)
	    {
//	    rxstart = (rxstart + 1) % rxlen;			//overflow, lose old data
		rxstart=rxstart=1;
		if(rxstart>=rxlen)rxstart=0;

	    rxoverflow = 1;								//record that overflow occurred.
	    }
	r = 1;
	}
    a=LPC_UART1->LSR;
    while ((txstart != txend) && (0 != (1 << 5 & LPC_UART1->LSR)))//data available and buffer available
	{
	LPC_UART1->THR = tx[(txstart++)];
	if (txstart>=txlen)txstart=0;


	r = 1;
	}
    return r;

    }

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief BT comms with BT module RN42/RN41 Receiver data. Transmit only
///@param void
///@return void
///
///for use in IR capture and replay.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void txBT(void)
    {

    while ((txstart != txend) && (0 != (1 << 5 & LPC_UART1->LSR)))//data available and buffer available
	{
	LPC_UART1->THR = tx[(txstart++)];
	if (txstart>=txlen)txstart=0;

	}
    }


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief on waking up from BT interrupt responds by transmitting bluetooth W
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void BTWAKE(void)
{
	byte WAKE[] = { 'W' };

	if (SWBT) {
//	SWBT=0;
	sendBT(WAKE, sizeof(WAKE));

}
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

    LPC_GPIO_BTRESET FIOCLR = BTRESET; //NRESET OUT High, Low to reset.
    us(100000);
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

    LED1GREEN();
    LED2GREEN();

    LPC_GPIO_BTFACTORY FIODIR |= BTFACTORY;
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LPC_GPIO_BTRESET FIOSET = BTRESET;
  	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
    LED2YELLOW();
 	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
    LED2GREEN();
	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
    LED2YELLOW();
 	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
    LED2GREEN();
  	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOCLR = BTFACTORY;
    LED1YELLOW();
    LED2YELLOW();
	us(1000000); //1s delay
    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;
    LED1GREEN();
    LED2GREEN();
	us(1000000); //1s delay
 //   LED1OFF();
    }

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief enable discovery mode for BT module RN42/RN41
///@param void.
///@return void.
///
///exit when HEX!=0x0E
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void discoverBT(void)
    {
    initBT();
    LPC_GPIO_BTDISCOVERY FIOSET = BTDISCOVERY; //DISCOVERY PIO3. OUT HIGH=auto-discovery.
    LED1YELLOW();
    while (HEX() == 0x0E)
	{

	}
    LPC_GPIO_BTDISCOVERY FIOCLR = BTDISCOVERY; //DISCOVERY PIO3. OUT LOW=disable auto-discovery.
    LED1OFF();
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
    byte CMD[] =
	{
	'$', '$', '$'
	}; //,'\r','\n'};    \r is CR, \n is LF
    byte ENDCMD[] =
	{
	'-', '-', '-', '\r', '\n'
	};
    byte NoTimeOut[] =
	{
	'S', 'T', ',', '2', '5', '5', '\r', '\n'
	}; //		Disable config timeout = 255.


    byte BTsleep[] =
    {
    	'S','W',',','0','1','0','0','\r','\n'			//number in hex 256*0.625ms=160ms
    };


    byte BTSpecial[]=
    {
    		"SJ,0800\r\n"			//Page scan continuous.
    };
#if baud==92
    byte BaudK[] =			//baud rate 230k
       {
    		   'S','U',',','9','2','\r','\n'
       };
#elif baud==46
    byte BaudK[] =			//baud rate 230k
       {
    		   'S','U',',','4','6','\r','\n'
       };

#elif baud==23
    byte BaudK[] =			//baud rate 230k
       {
    		   'S','U',',','2','3','\r','\n'
       };
#elif baud==11
    byte BaudK[] =			//baud rate 230k
       {
    		   'S','U',',','1','1','\r','\n'
       };
#endif


    LED1OFF();
    LED2OFF();
    us(1000000);

    rxstart = rxend;


    sendBTNC(CMD, sizeof(CMD));
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


    us(1000000);
    LED2OFF();
    rxstart = rxend;
    sendBTNC(NoTimeOut, sizeof(NoTimeOut));
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
    sendBTNC(BTsleep, sizeof(BTsleep));
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
		LED1YELLOW();
		i = 20000;
		}
	    }
	}
    rxstart = rxend;

   sendBTNC(BaudK, sizeof(BaudK));
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
    sendBTNC(BTSpecial, sizeof(BTSpecial));
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
 		LED1YELLOW();
 		i = 20000;
 		}
 	    }
 	}
    rxstart = rxend;


    sendBTNC(ENDCMD, sizeof(ENDCMD));
    for (i = 0; i < 3000; i++)
	{
	us(1000);
	rxtxBT();
	if ((rxstart + 5) % rxlen == rxend)
	    {
	    if (rx[rxstart] != 'E')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'E') && (rx[(rxstart + 1) % rxlen] == 'N')
		    && (rx[(rxstart + 2) % rxlen] == 'D'))
		{
	    	rxstart=rxend;
		LED1GREEN();
		i = 20000;
		}
	    }
	}
    rxstart=rxend;

    }



void	BTQUIET(void)
{
    int i,s;
	SystemCoreClockUpdate ();
	s=SystemCoreClock; //4MHz
    byte CMD[] =
	{
	'$', '$', '$'
	}; //,'\r','\n'};    \r is CR, \n is LF
    byte ENDCMD[] =
	{
	'-', '-', '-', '\r', '\n'
	};


    byte BT2s[] =
    {
    	'S','W',',','0','C','8','0','\r','\n'			//number in hex 256*0.625ms=2s
    };



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
    sendBT(BT2s, sizeof(BT2s));
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
	if ((rxstart + 5) % rxlen == rxend)
	    {
	    if (rx[rxstart] != 'E')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'E') && (rx[(rxstart + 1) % rxlen] == 'N')
		    && (rx[(rxstart + 2) % rxlen] == 'D'))
		{
	    	rxstart=rxend;
		LED1YELLOW();
		i = 20000;
		}
	    }
	}
    rxstart=rxend;
}

void	BTON(void)
{
    int i,s;
	SystemCoreClockUpdate ();
	s=SystemCoreClock; //4MHz
    byte CMD[] =
	{
	'$', '$', '$'
	}; //,'\r','\n'};    \r is CR, \n is LF
    byte ENDCMD[] =
	{
	'-', '-', '-', '\r', '\n'
	};


    byte BT2s[] =
    {
    	'S','W',',','0','1','0','0','\r','\n'			//number in hex 256*0.625ms=160ms
    };



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
    sendBT(BT2s, sizeof(BT2s));
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
	if ((rxstart + 5) % rxlen == rxend)
	    {
	    if (rx[rxstart] != 'E')
		rxstart = (rxstart + 1) % rxlen;
	    if ((rx[rxstart] == 'E') && (rx[(rxstart + 1) % rxlen] == 'N')
		    && (rx[(rxstart + 2) % rxlen] == 'D'))
		{
	    	rxstart=rxend;
		LED1YELLOW();
		i = 20000;
		}
	    }
	}
    rxstart=rxend;


}
