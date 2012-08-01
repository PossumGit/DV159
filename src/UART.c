///@name        	UART comms
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"


//Public variables

//Private variables

//External variables

//Private functions
PRIVATE void baud115(void);
PRIVATE void baud921(void);

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Setup UART communicate with BLUETOOTH module.
///@param void.
///@return void.
///
///115KBaud
///Use RTS/CTS for flow control.
///DMA is an option, but may get out of step time wise after long IR transmission.
///FIFO has 16 RX and 16 TX bytes.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initUART(void)
{


	LPC_GPIO0->FIOCLR	=1 << 22;					//RTS(CPU). OUT  HIGH to stop BT sending data, LOW to enable BT sending.
	LPC_GPIO0->FIODIR	|= 1 << 22;					//RTS(CPU) UARTCTS. HIGH to stop BT sending data
	LPC_GPIO0->FIODIR	|= 1 << 17;					//CTS(CPU) UARTRTS. IN HIGH means stop transmitting.

	LPC_SC->PCONP |= 1 << 4;						//enable UART1 (RESET enables.) OK

	LPC_PINCON->PINSEL0 |= (1 << 30);				//UART1 TXD1 OK
	LPC_PINCON->PINSEL1 |= (1 << 0|1<<2|1<<12); 	//UART1 RXD1//|CTS1|RTS1 OK
	LPC_UART1->FCR =1|2<<6;							//RTS goes high after 8 chars, RTS low when 4 chars or less
	LPC_UART1->MCR=1<<6|1<<7;						//AUTO RTS|AUTO CTS
	baud115();
	ms();//1ms delay
}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set up UART baud rate to 921Kbaud maximum allowed by BT module RN42/RN41
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void	baud921(void)
{
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	LPC_SC->PCLKSEL0 |=  0<< 8; 					//100MHz/4= 25MHz. UART clock (CCLK/4 by RESET)OK
													//0<<8=/4,  1<<8=/1,  2<<8=/2,  3<<8=/8
	LPC_UART1->DLL =1;								//UART CLOCK.
	LPC_UART1->DLM =0;								//UART CLOCK.
	LPC_UART1->FDR =9<<0|13<<4;					//Fractional divide. 921kbaud
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set up UART baud rate to 115.2Kbaud to match BT module RN42/RN41
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////

//100MHz CCLK version.

void	baud115()
{
	///set DLAB to set up Baud rate.
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	///
	///PCLK = 25MHz= 100MHz/4	(PCLSEL1)
	///baud rate = PCLK/(16*(256*DLM+DLL)*(1+DIV/MUL))
	///
	LPC_SC->PCLKSEL0 |=  0<< 8; 					//0=100MHz/4= 25MHz. UART clock (CCLK/4 by RESET)OK
	LPC_UART1->DLL =7;								//UART CLOCK. 7
	LPC_UART1->DLM =0;								//UART CLOCK. 0
	LPC_UART1->FDR =14<<0|15<<4;					//Fractional divide. 0.93333
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}
/*
//6MHz version.
void	baud115()
{
	///set DLAB to set up Baud rate.
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	///
	///PCLK = 25MHz= 100MHz/4	(PCLSEL1)
	///baud rate = PCLK/(16*(256*DLM+DLL)*(1+DIV/MUL))
	///
	LPC_SC->PCLKSEL0 |=  1<< 8; 					//6MHz/1=6MHz
	LPC_UART1->DLL =3;								//UART CLOCK.
	LPC_UART1->DLM =0;								//UART CLOCK.
	LPC_UART1->FDR =1<<0|12<<4;					//Fractional divide.
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}
*/



