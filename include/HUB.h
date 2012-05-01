/*
 * HUB.h
 *
 *  Created on: 4 Apr 2012
 *      Author: Duncan Irvine
 */

#define PrescaleIR 1
#define PRIVATE static
#define CaptureMax  8192				//8192*4 bytes = 32768 bytes of RAM. Size of Buffer.
#define	CaptureExt	6000				//size of Buffer2



//functions in main.c
void LED3GREEN(void);
void LED3YELLOW(void);
void LED3OFF(void);
//functions in IR.c
void captureIR(void);
void playIR(void);
void initIR(void);
int HEX1(void);
int HEX2(void);


void initAudio(void);
void recordAudio(void);
void playAudio(void);

void eraseFlash(void);
void writeByte(int);
int	readByte(void);
int	queryByte(void);
void writeFlash(void);
void readFlash(void);
int	readStatus(void);
void initFlash(void);
void testFlash(void);			//not used.
void clearRXFIFO(void);
void clearTXFIFO(void);
