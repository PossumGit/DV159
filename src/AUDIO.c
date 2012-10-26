///@name        	Audio read/write routines.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012


//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2s.h"


///Private Defines
// Max buffer length
#define BUFFER_SIZE				0x400
// I2S Buffer Source Address is AHBRAM1_BASE that used for USB RAM purpose, but
// it is not used in this example, so this memory section can be used for general purpose
// memory
//
#define I2S_BUFFER_SRC			Buffer
//LPC_AHBRAM1_BASE //0x20080000
// I2S Buffer Destination Address is (AHBRAM1_BASE + 0x100UL) that used for USB RAM purpose, but
// it is not used in this example, so this memory section can be used for general purpose
// memory
//
#define I2S_BUFFER_DST			(I2S_BUFFER_SRC+0x1000UL) //0x20081000

#define RXFIFO_EMPTY		0
#define TXFIFO_FULL			8


///Public variables

///External variables
	EXTERNAL word  Buffer[];
///Private variables
	word 	TPtr;
	PRIVATE word  BufferPtr;

	volatile byte  I2STXDone = 0;
	volatile byte  I2SRXDone = 0;

	volatile word *I2STXBuffer = (word*)(I2S_BUFFER_SRC);
	volatile word *I2SRXBuffer = (word *)(I2S_BUFFER_DST);

	volatile word I2SReadLength = 0;
	volatile word I2SWriteLength = 0;

	byte tx_depth_irq = 0;
	byte rx_depth_irq = 0;
	byte dummy=0;

///External functions

///Private functions

///local functions(code)
///
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Set up I2S comms for audio
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initAudio(void)
{

	LPC_GPIO_LNR FIODIR |= LNR; 		//L/R on ADMP441 microphone = output.
	LPC_GPIO_LNR FIOCLR = LNR; 		//L/R =1
	LPC_GPIO_MICCE FIODIR |=MICCE; 		//CHIPEN on mic =output.
	LPC_GPIO_MICCE FIOSET = MICCE; 		//CHIPEN=1=enabled.



		int	i;
	LPC_SC->PCONP |=1<<27;						// bit 27. enable I2S		//OK
	LPC_SC->PCLKSEL1 |= 0<<22;					//PCLK_I2S(bit 22,23)=CCLK/4=100MHz/4. Not reliable if /1, OK/4
	LPC_PINCON->PINSEL0 |=(1<<14 |1<<16 |1<<18);	//TX P0.7(CLK), P0.8(WS), P0.9(SDA) =I2S.	//OK
//	LPC_PINCON->PINMODE0 |=(2<<14 |2<<16 |2<<18);	//TX no pullup or pulldown.
	LPC_PINCON->PINSEL0 |=(1<<8 |1<<10 |1<<12);		//RX P0.4(CLK), P0.5(WS), P0.6(SDA) =I2S.	//OK
//	LPC_PINCON->PINMODE0 |=(2<<8 |2<<10 |2<<12);	//RX no pullup or pulldown.


	LPC_I2S->I2SDAO=
			1	//16 bits
			|1<<2			//mono tx data twice
			|0<<3			//stop
			|0<<4			//reset
			|0<<5			//master
			|0x1F<<6		//half period = 32
			|0<<15;			//mute			clear to 0 to transmit data.

	LPC_I2S->I2SDAI=
			1				// 16 bits
			|1<<2			// mono
			|0<<3			//stop
			|0<<4			//reset
			|0<<5			//master
			|0x1F<<6;		//half period=32

	LPC_I2S->I2SIRQ=
			1				//enable RX interrupt
			|2<<8			//interrupt when buffer has 1 words of 32 bit data.
			|2<<16;			//interrupt when buffer has 1 words of 32 bits.

	i=LPC_I2S->I2SSTATE;		//status feedback.

			//clock.
			//PCLK_I2S=CCLK=100MHz.
			//32KHz sampling requires 2.048MHz
			//MCLK=PCLK_I2S*X/(2Y)
			//X=1, Y=24 gives: 2.0833MHz, a little high, but minimises jitter.
	//ADMP441 gives 32 bit +32 bit at up to 48KHz.
	//TFA9882 can use 8+8 to 32+32bit at 32KHz to 48KHz
	//32KHz= 2.048MHz MCLK
	//44.1KHz=2.8824MHz MCLK
	//48KHz=3.072MHz MCLK.

	LPC_I2S->I2SRXRATE=3|1<<8;		//RXCLK=100MH/48=2.0833MHz
	LPC_I2S->I2STXRATE=3|1<<8;		//TXCLK=100MH/48=2.0833MHz
	LPC_I2S->I2SRXBITRATE=1;		//final /2
	LPC_I2S->I2STXBITRATE=1;		//final/2
	LPC_I2S->I2SRXMODE=0|0<<2|1<<3;	//do not output RX_MCLK
	LPC_I2S->I2STXMODE=0|0<<2|1<<3;	//do not output TX_MCLK


		BufferPtr=0;
		TPtr=0;




}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Record audio, save in flash memory
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void recordAudio(void)

{



	NVIC->ISER[0]=1<<27;		//I2S interrupt enable
	for(;;);					//wait here.
}

///play audio from flash memory.

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Play audio from flash memory.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void playAudio(void)
{

}




/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief I2S audio comms interrupt routine
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////

 void I2S_IRQHandler()
{
//	word RXLevel = 0;





	int	a,x;

		a=LPC_I2S->I2SSTATE;

		x=LPC_I2S->I2SRXFIFO;







		if (BufferPtr<CaptureMax)
		{

		Buffer[BufferPtr++] = x;

		}
		else
		{


			a=LPC_I2S->I2SSTATE;
			a=LPC_I2S->I2SSTATE;
			a=LPC_I2S->I2SSTATE;
			a=LPC_I2S->I2SSTATE;
			LPC_I2S->I2SIRQ=
					2				//enable tX interrupt, disable rx interrupt.
					|0<<8			//interrupt when buffer has 0 words of 32 bit data.
					|0<<16;			//interrupt when buffer has 0 words of 32 bits.

			x=Buffer[TPtr++];

			//I2S is twos complement signed. 0=mute, 0x7FFF is max positive, 0x8000 is max negative.
			//At 32KHz, maximum amplitude square wave of 2KHz.
			//FIFO has 8 words of 32 bits or 16 words of 16 bits as shown.
			//could manage 32 words of 8 bits giving 1KHz simply.
			LPC_I2S->I2STXFIFO=0x7FFF7FFF;
			LPC_I2S->I2STXFIFO=0x7FFF7FFF;
			LPC_I2S->I2STXFIFO=0x7FFF7FFF;
			LPC_I2S->I2STXFIFO=0x7FFF7FFF;
			LPC_I2S->I2STXFIFO=0x80008000;
			LPC_I2S->I2STXFIFO=0x80008000;
			LPC_I2S->I2STXFIFO=0x80008000;
			LPC_I2S->I2STXFIFO=0x80008000;
			if (TPtr>=CaptureMax)
			{
			TPtr=1000;
		}
		}




/*	//Check RX interrupt
	if(I2S_GetIRQStatus(LPC_I2S, I2S_RX_MODE))
	{
		RXLevel = I2S_GetLevel(LPC_I2S, I2S_RX_MODE);
		if ( (RXLevel != RXFIFO_EMPTY) && !I2SRXDone )
		{
			while ( RXLevel > 0 )
			{
				if ( I2SReadLength == BUFFER_SIZE )
				{
					//Stop RX
					I2S_Stop(LPC_I2S, I2S_RX_MODE);
					// Disable RX
					I2S_IRQCmd(LPC_I2S, I2S_RX_MODE, DISABLE);
					I2SRXDone = 1;
					break;
				}
				else
				{
					I2SRXBuffer[I2SReadLength++] = LPC_I2S->I2SRXFIFO;
				}
				RXLevel--;
			}
		}
	}
*/
	return;
}

