///@name        	SSP0 serial comms routines.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_clkpwr.h"

//Public variables

//Private variables

//External variables

//Private functions

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read one byte over SSP serial port.
///@param void
///@return char 8 bit byte.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC char readSSP0Byte(void) {
	while (0 == ((LPC_SSP0->SR) & (1 << 2)))
		//wait for byte.
		;
	return (LPC_SSP0->DR);
}

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Write one byte over SSP serial port.
///@param char 8 bit byte
///@return void.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void writeSSP0Byte(char b) {
	while (0 == ((LPC_SSP0->SR) & (1 << 1)))
		; //wait for TX FIFO not full. (ready)
	LPC_SSP0->DR = b;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Setup SSP0 to communicate with flash memory.
///@param void.
///@return void.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initSSP0Flash(void) {

	LPC_GPIO0->FIOSET = 1 << 23; //Flash disable
	LPC_GPIO0->FIODIR |= 1 << 23; //CHIPEN on flash.
	LPC_GPIO1->FIOSET = 1 << 21; //NEAT disable
	LPC_GPIO0->FIODIR |= 1 << 21; //CHIPEN on NEAT.
	//set up SSP bits
	LPC_SC->PCONP |= 1 << 21; // bit 8. enable SSP0. (RESET enables.)
	LPC_SC->PCLKSEL1 |= 1 << 10; //100MHz/2= 50MHz. SSP0 clock (CCLK/4 by RESET)
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
//	LPC_SSP0->CPSR = 2; //divide clock for SSP0 //gives 25MHz clock for FLASH
	LPC_SSP0->CPSR = 50; //divide by 50(must be even) gives 1MHz clock for NEAT (max 1.8MHz.)

}
