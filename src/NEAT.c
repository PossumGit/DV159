
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

//External functions

EXTERNAL void initSSP0Flash(void);
EXTERNAL void writeSSP0Byte(char b);
EXTERNAL char readSSP0Byte(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC char readNEAT(void) {
	initSSP0Flash();

		; //wait for not busy
	//set up SPI for flash.
	char a, b, c,d;

	LPC_GPIO1->FIODIR |= 1 << 21; //CHIPEN on NEAT.
	LPC_GPIO1->FIOCLR = 1 << 21; //NEAT enable
	ms();
	writeSSP0Byte(0x47); //0x80 = read command, 0x01 is register
	a = readSSP0Byte();
	ms();
	writeSSP0Byte(0x00); //address 16-23
	b = readSSP0Byte();
	ms();

	writeSSP0Byte(0xC7); //0x80 = read command, 0x01 is register
	c = readSSP0Byte();
	ms();
	writeSSP0Byte(0xFF); //address 16-23
	d = readSSP0Byte();
	ms();



//	writeSSP0Byte(0xFF); //address 8-15
//	c = readSSP0Byte();
//	ms();

	LPC_GPIO1->FIOSET = 1 << 21; //NEAT disable

	return c;
}
