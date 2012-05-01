/*
 ===============================================================================
 Name        : Flash.c
 Author     : Duncan Irvine
 Version     : test
 Copyright   : Copyright (C)23 April 2012
 Description : Flash read/write routines.
 ===============================================================================
 */
//
/////////////////////////////////////////////////////////////////////////////////////////////////
////PUBLIC FUNCTIONS
//
//
//
////PUBLIC VARIABLES
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <cr_section_macros.h>
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "HUB.h"

initFlash() {
	int a;
	LPC_GPIO0->FIOSET = 1 << 23; //Flash disable
	LPC_GPIO0->FIODIR |= 1 << 23; //CHIPEN on flash.
	LPC_GPIO1->FIOSET = 1 << 21; //NEAT disable


	//set up SSP bits
	LPC_SC->PCONP |= 1 << 21; // bit 8. enable SSP0. (RESET enables.)
	LPC_SC->PCLKSEL1 |= 2 << 10; //100MHz/2= 50MHz. SSP0 clock (CCLK/4 by RESET)


	LPC_PINCON->PINSEL3 |= (3 << 8 | 3 << 14 | 3 << 16); //SPI = P1.20,  P1.23, P1.24.


	LPC_PINCON->PINMODE3 |= (0 << 8 | 0 << 14 | 0 << 16); // pullup on P1.20,  P1.23, P1.24.
	LPC_PINCON->PINMODE_OD1 |= (0 << 20 | 0 << 21 | 0 << 23 | 0 << 24); //normal mode output (open drain disabled.)


	//	a=LPC_GPIO1->FIODIR;
	//	a=a|1 << 21;
	//	LPC_GPIO1->FIODIR =a; 		//CHIPEN on NEAT.


	//1 page is 256 bytes,  2048 bits = approx 0.33ms, +write time approx 1ms.
	//Audio is 32KHz* 16 bits =64KB/s =64B/ms
	//IR is 38KHz continuous ( worst case) =38*4 bytes = 152 B/ms

	//SPANSION S25FL016 supports mode 0 (CPOL=0, CPHA=0) and mode 3 (CPOL=1, CPHA=1).
	//so use mode 0.
	LPC_SSP0->CR0 = 7 << 0 | 0 << 4 | 0 << 6 | 0 << 7 | 3 << 8;
	LPC_SSP0->CR1 = 0 << 0 | 1 << 1 | 0 << 2;
	LPC_SSP0->CPSR = 2; //divide clock for SSP0


}

int readByte() {
	while (0 == ((LPC_SSP0->SR) & (1 << 2)))			//wait for byte.
		;
	return (LPC_SSP0->DR) ;
}

int queryByte() {
	if (0 != ((LPC_SSP0->SR) & (1 << 2)))
		return (LPC_SSP0->DR) & 0xFF;
	else
		return 256;
}

writeByte(int b) {
	while (0 == ((LPC_SSP0->SR) & (1 << 1)))
		; //wait for TX FIFO not full. (ready)
	LPC_SSP0->DR = b;
}

eraseFlash() {
	int a;

	//first command is write enable
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeByte(0x06);			//write enable for erase command.
	a = readByte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
//	a = readStatus();
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeByte(0x20); //page erase
	a = readByte();
	writeByte(0x00); //address
	a = readByte();
	writeByte(0x00); //address
	a = readByte();
	writeByte(0x00); //address
	a = readByte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable

	while(1& readStatus());


}
readStatus() {
	int a, b;
		LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
		writeByte(0x05); //read
		writeByte(0x00); //read address
		a = readByte();
		b = readByte();
		LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
		return b;


}
clearRXFIFO()
{
	int a;
	while (0==(0x1 & LPC_SSP0->SR)); //wait for TX FIFO empty.
	while ((LPC_SSP0->SR) & (1 << 2))
		a=LPC_SSP0->DR;	//read data until FIFO empty.
	while ((LPC_SSP0->SR)&(0x10));
}
waitTXempty()
{
	while (0==(0x1 & LPC_SSP0->SR)); //wait for TX FIFO empty.
}


writeFlash() {
	int a, b, c, d, e, f, g, h, i;
	//write from Buffer to address 00.

	waitTXempty();
	clearRXFIFO();
	a = readStatus();
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeByte(0x06);
	b = readByte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable

	c = readStatus();


	d = readStatus();


	e = readStatus();



	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable

	writeByte(0x02); //page write
	a=readByte();
	writeByte(0x00); //address
	b = readByte();
	writeByte(0x00); //address
	b = readByte();
	writeByte(0x00); //address
	b = readByte();
	for (i = 0; i < 256; i++) {
		writeByte(0xAA); //data
		b = readByte();
	}
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable


}

readFlash() {

	//set up SPI for flash.
	int a, b, c;

	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeByte(0x03); //read command
	a = readByte();
	writeByte(0x00); //read address
	a = readByte();
	writeByte(0x00); //read address
	a = readByte();
	writeByte(0x00); //read address
	a = readByte();
	writeByte(0x00); //dummy read address
	a = readByte();		//byte 1
	writeByte(0x00); //dummy read address
	b = readByte();		//byte 2
	writeByte(0x00); //dummy read address
	c = readByte();		//byte 3
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
}


testFlash()
		{
	//	LPC_GPIO0->FIOCLR = 1 << 23; 		//Flash enable
	int a, b, c, d, e, f, g,h,i,j,k,l,m,n,o,p,q,r,s,t,u;
	//clear the FIFO.
	clearFIFO();

	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeByte(0x4B); //read
	writeByte(0x00); //read address
	writeByte(0x00);
	writeByte(0x00);

	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);

	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);

	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x00);

	a = readByte();
	b = readByte();
	c = readByte();
	d = readByte();

	e = readByte();
	f = readByte();
	g = readByte();
	h = readByte();

	i = readByte();
	j = readByte();
	k = readByte();
	l = readByte();

	m = readByte();


	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	return b;

}
