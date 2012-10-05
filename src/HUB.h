/*
 * HUB.h
 *
 *  Created on: 4 Apr 2012
 *      Author: Duncan Irvine
 */
//operators
//bitwise	~	&	|
//logical	!	&&	||
//
//xor ^
//ternary  ? :
//
//
//
#define PCBissue 3		//pcb issue, options 2, 3
//for conditional compile based on PCB issue.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PrescaleIR 1
#define PRIVATE static					//within file only
#define PUBLIC							//accessible outside of file.
#define EXTERNAL extern								//defined outside of file
#define CaptureMax  8192				//8192*4 bytes = 32768 bytes of RAM. Size of Buffer.
#define BufferMax   8192				//alias of CaptureMax.
#define	CaptureExt	6000				//size of Buffer2
#define Peripherals 1<<1|1<<2|1<<4|1<<8|1<<10|1<<15|1<<19|1<<21|1<<22|1<<27|1<<31;        // Power Control for Peripherals      */




#if PCBissue==3		//issue 3 PCB
//GPIO pin definitions Issue 3

#define OFF   1<<11						// P0.11
#define LPC_GPIO_OFF 	LPC_GPIO0->

#define IROUT	1<<28					// P1.28
#define LPC_GPIO_IROUT	LPC_GPIO1->

#define MICCE	1<<8					// P1.8
#define LPC_GPIO_MICCE	LPC_GPIO1->

#define FLASHCS	1<<10					// P1.10
#define LPC_GPIO_FLASHCS LPC_GPIO1->

#define T2G		1<<9					// P2.9
#define LPC_GPIO_T2G LPC_GPIO2->

#define LNR		1<<9					// P1.9
#define LPC_GPIO_LNR LPC_GPIO1->

#define BTMASTER	1<<1				// P1.1
#define LPC_GPIO_BTMASTER LPC_GPIO1->

#define BTRESET		1<<4				// P1.4
#define LPC_GPIO_BTRESET LPC_GPIO1->

#define BTFACTORY	1<<23				// P0.23
#define LPC_GPIO_BTFACTORY LPC_GPIO0->

#define BTDISCOVERY	1<<25				// P0.25
#define LPC_GPIO_BTDISCOVERY LPC_GPIO0->

#define BTDATASTATUS	1<<6			// P2.6
#define LPC_GPIO_BTDATASTATUS LPC_GPIO2->

#define BTCSTATUS		1<<24			// P0.24
#define LPC_GPIO_BTCSTATUS LPC_GPIO0->

#define BTCONNECTED		1<<26			// P0.26
#define LPC_GPIO_BTCONNECTED LPC_GPIO0->

#define NEATCS			1<<21			// P1.21
#define LPC_GPIO_NEATCS LPC_GPIO1->

#define NEATINT			1<<10			// P1.25 MOD to P2.10 B on PCB.
#define LPC_GPIO_NEATINT LPC_GPIO2->

#define LED1G			1<<16			// P1.17
#define LPC_GPIO_LED1G LPC_GPIO1->

#define LED1Y			1<<17			// P1.16
#define LPC_GPIO_LED1Y LPC_GPIO1->

#define BTCTS			1<<22			// P0.22
#define LPC_GPIO_BTCTS LPC_GPIO0->

#define BTRTS			1<<17			// P0.17
#define LPC_GPIO_BTCTS LPC_GPIO0->

#define BTSPICS			1<<31			//P1.31
#define LPC_GPIO_BTSPICS LPC_GPIO1->


#elif PCBissue==2						//issue 2 PCB
#define OFF  		1<<21				// P0.21
#define LPC_GPIO_OFF 	LPC_GPIO0->

#define IROUT		1<<28				// P1.28
#define LPC_GPIO_IROUT	LPC_GPIO1->

#define MICCE	1<<28					// P4.28
#define LPC_GPIO_MICCE	LPC_GPIO4->

#define FLASHCS	1<<23					// P0.23
#define LPC_GPIO_FLASHCS LPC_GPIO0->

#define T2G		1<<9					// P2.9
#define LPC_GPIO_T2G LPC_GPIO2->

#define LNR		1<<17					// P1.17
#define LPC_GPIO_LNR LPC_GPIO1->

#define BTMASTER	1<<1				// P1.1
#define LPC_GPIO_BTMASTER LPC_GPIO1->

#define BTRESET		1<<4				// P1.4
#define LPC_GPIO_BTRESET LPC_GPIO1->

#define BTFACTORY	1<<0				// P1.0
#define LPC_GPIO_BTFACTORY LPC_GPIO1->

#define BTDISCOVERY	1<<9				// P1.9
#define LPC_GPIO_BTDISCOVERY LPC_GPIO1->

#define BTDATASTATUS	1<<14			// P1.14
#define LPC_GPIO_BTDATASTATUS LPC_GPIO1->

#define BTCONNECTED		1<<10			// P1.10
#define LPC_GPIO_BTCONNECTED LPC_GPIO1->

#define BTCSTATUS		1<<8			// P1.8
#define LPC_GPIO_BTCSTATUS LPC_GPIO1->

#define NEATCS			1<<21			// P1.21
#define LPC_GPIO_NEATCS LPC_GPIO1->

#define NEATINT			1<<0			// P2.0
#define LPC_GPIO_NEATINT LPC_GPIO2->

#define LED1G			1<<19			// P1.19
#define LPC_GPIO_LED1G LPC_GPIO1->

#define LED1Y			1<<22			// P1.22
#define LPC_GPIO_LED1Y LPC_GPIO1->

#define BTCTS			1<<22			// P0.22
#define LPC_GPIO_BTCTS LPC_GPIO0->

#define BTRTS			1<<17			//P0.17
#define LPC_GPIO_BTCTS LPC_GPIO0->

#endif





//functions in main.c
void LED1GREEN(void);
void LED1YELLOW(void);
void LED1OFF(void);
//functions in IR.c
int captureIR(void);
void playIR(void);
void initIR(void);
char HEX(void);



void initAudio(void);
void recordAudio(void);
void playAudio(void);



void eraseFlash(void);
void readFlashHigh(void);
void readFlashLow(void);
void writeFlashHigh(void);
void writeFlashLow(void);







void writeSSP0Byte(char);///< Write one byte over SSP serial port.
char readSSP0Byte(void);///< Read one byte over SSSP serial port.



void initSSP0(void);

void I2CINIT(void);
void I2CREAD(void);
void I2CFullCharge(void);
int I2CBATTERY(void);
void initBT(void);

void us (int);

void resetBT(void);

void	msStart(void);
char	msEnd(void);


void	factoryBT(void);
void	discoverBT(void);
void	sendBT(char [] ,int );
int	rxtxBT(void);
void txBT(void);
int processBT(void);
char	INPUT(void);
char	inputChange(void);
void clearBT(void);

void initUART(void);
void timer2Start(void);

 void	CPU100MHz(void);
 void 	CPU12MHz(void);

void enableExtInterrupt(void);
void disableInputInterrupt(void);

void goToSleepQ(void);
int powerDown(void);
 void txshortBT(char istat[], int ilength);
 void BTWAKE(void);
 void BatteryState(void);
 int repeatInput(void);
  void timer2CPU12(void);
  void timer2CPU100(void);
  void SSPNEATCPU4(void);
  void SSPNEATCPU12(void);
  void SSPNEATCPU100(void);
  void	BTbaudCPU12(void);
  void	BTbaudCPU100(void);
  void	BTbaudCPU4(void);
