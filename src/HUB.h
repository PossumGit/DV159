/*
 * HUB.h
 *
 *  Created on: 4 Apr 2012
 *      Author: Duncan Irvine
 */

#define PrescaleIR 1
#define PRIVATE static					//within file only
#define PUBLIC							//accessible outside of file.
#define EXTERNAL extern								//defined outside of file
#define CaptureMax  8192				//8192*4 bytes = 32768 bytes of RAM. Size of Buffer.
#define BufferMax   8192				//alias of CaptureMax.
#define	CaptureExt	6000				//size of Buffer2
#define Peripherals 1<<1|1<<2|1<<4|1<<8|1<<10|1<<15|1<<19|1<<21|1<<22|1<<27|1<<31;        // Power Control for Peripherals      */


//functions in main.c
void LED3GREEN(void);
void LED3YELLOW(void);
void LED3OFF(void);
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



void initSSP0Flash(void);

void I2CINIT(void);
void I2CREAD(void);
void I2CFullCharge(void);
int I2CBATTERY(void);
void initBT(void);

void ms (void);

void resetBT(void);

void	msStart(void);
char	msEnd(void);


void	factoryBT(void);
void	discoverBT(void);
void	sendBT(char [] ,int );
int	rxtxBT(void);
int processBT(void);
char	INPUT(void);
char	inputChange(void);
void clearBT(void);

void initUART(void);
void timer2Start(void);

 void	fullSpeed(void);
 void 	lowPower(void);

void enableExtInterrupt(void);

void goToSleepQ(void);
 void powerDown(void);
