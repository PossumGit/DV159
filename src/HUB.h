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



//functions in main.c
void LED3GREEN(void);
void LED3YELLOW(void);
void LED3OFF(void);
//functions in IR.c
void captureIR(void);
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


//int	queryByte(void);
//void writePage(void);
//void readFlash(void);

void initSSP0Flash(void);
//void testFlash(void);			//not used.
//void clearRXFIFO(void);
//void clearTXFIFO(void);



void initI2C(void);
void initBT(void);


void ms (void);

void resetBT(void);

void	msStart(void);
char	msEnd(void);


void	factoryBT(void);
void	discoverBT(void);
void	sendBT(char [] ,int );
void	rxtxBT(void);
void processBT(void);
char	INPUT(void);

void initUART(void);

