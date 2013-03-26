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



//External functions
EXTERNAL void	us(unsigned int time_us);

//public functions
PUBLIC void initUART(void);
PUBLIC void	BTbaudCPU100();
PUBLIC void	BTbaudCPU12();
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


	LPC_GPIO_BTCTS FIOCLR	=BTCTS;					//RTS(CPU) BTCTS. OUT  HIGH to stop BT sending data, LOW to enable BT sending.
	LPC_GPIO_BTCTS FIODIR	|= BTCTS;				//RTS(CPU) BTRTS UARTCTS. HIGH to stop BT sending data
	LPC_GPIO_BTCTS FIODIR	&= ~(BTRTS);			//INPUT CTS(CPU) UARTRTS. IN HIGH means stop transmitting.
	LPC_SC->PCONP |= 1 << 4;						//enable UART1 (RESET enables.) OK
	LPC_PINCON->PINSEL0 |= (1 << 30);				//UART1 TXD1 OK
	LPC_PINCON->PINSEL1 |= (1 << 0|1<<2|1<<12); 	//UART1 RXD1//|CTS1|RTS1 OK
	LPC_UART1->FCR =1|2<<6;							//RTS goes high after 8 chars, RTS low when 4 chars or less
	LPC_UART1->MCR=1<<6|1<<7;						//AUTO RTS|AUTO CTS
	BTbaudCPU12();
	us(1000);//1ms delay

}





/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set up UART baud rate to 115.2Kbaud to match BT module RN42/RN41
///@param void
///@return void
///100MHz CPU clock version
/////////////////////////////////////////////////////////////////////////////////////////////////

void	BTbaudCPU100()
{
	return;
	///set DLAB to set up Baud rate.
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	///
	///PCLK = 25MHz= 100MHz/4	(PCLSEL1)
	///baud rate = PCLK/(16*(256*DLM+DLL)*(1+DIV/MUL))
	/// For 921KBaud.
	LPC_SC->PCLKSEL0 |=  3<< 8; 					//BAUD 3=115, 0=230, 2=460, 1=921. 100MHz/4= 25MHz. UART clock (CCLK/4 by RESET)OK 3=/8=12.5mhz
	LPC_UART1->DLL =4;								//DLL must be >2. UART CLOCK. 7///DLL=1,2 does not work.
	LPC_UART1->DLM =0;								//UART CLOCK. 0
	LPC_UART1->FDR =9<<0|13<<4;					//Fractional divide. 0.93333
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set up UART baud rate to 230.8Kbaud to match BT module RN42/RN41
///@param void
///@return void
///12MHz CPU clock version
///
///note 4MHz variant does not work as outside working range.
/////////////////////////////////////////////////////////////////////////////////////////////////

void	BTbaudCPU12()
{
	///set DLAB to set up Baud rate.
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	///
	///PCLK = 25MHz= 100MHz/4	(PCLSEL1)
	///baud rate = PCLK/(16*(256*DLM+DLL)*(1+DIV/MUL))
	///
	LPC_SC->PCLKSEL0 |=  2<< 8; 					//12MHz/2=6MHz	//2=115, 1=230, 0 for 57.6, 3=28.8
	LPC_UART1->DLL =3;								//DLL must be >2. UART CLOCK.//3 for 115, 3 for 230, 1 for 460
	LPC_UART1->DLM =0;								//UART CLOCK.
	LPC_UART1->FDR =1<<0|12<<4;					//Fractional divide.	//1<<0|12<<4 for 115/230, 5<<0|8<<4 for 460
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set up UART baud rate to 115.2Kbaud to match BT module RN42/RN41
///@param void
///@return void
///12MHz CPU clock version
///
///note 4MHz variant does not work as outside working range.
/////////////////////////////////////////////////////////////////////////////////////////////////

void	BTbaud110CPU12()
{
	///set DLAB to set up Baud rate.
	LPC_UART1->LCR =3<<0|1<<7;						//8 bit word, divisor latch enable. OK
	///
	///PCLK = 25MHz= 100MHz/4	(PCLSEL1)
	///baud rate = PCLK/(16*(256*DLM+DLL)*(1+DIV/MUL))
	///
	LPC_SC->PCLKSEL0 |=  2<< 8; 					//2(/2)=115, 1(/1)=230, 0(/4) for 57.6, 3(/8)=28.8
	LPC_UART1->DLL =3;								//UART CLOCK.//3 for 115, 3 for 230, 1 for 460
	LPC_UART1->DLM =0;								//UART CLOCK.
	LPC_UART1->FDR =1<<0|12<<4;					//Fractional divide.	//1<<0|12<<4 for 115/230, 5<<0|8<<4 for 460
	///clear DLAB for comms
	LPC_UART1->LCR =3<<0|0<<7;						//8 bit word, divisor latch enable. OK
}


















