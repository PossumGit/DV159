///@name        	IR Synthesis
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

//Public variables

//Private variables

//External variables
EXTERNAL volatile word Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
EXTERNAL int BUFLEN;
//Private functions

//External functions
//EXTERNAL void sendBT(byte in[] , unsigned int );
//EXTERNAL void playIR(void);
//public functions
PUBLIC void IRsynthesis(byte IRtype, byte IRrep, int IRcode);
PUBLIC void IRDMAVIVO(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Synthesize Possum codes and place synthesised IR in buffer.
///@param byte IRtype: either H for HC1820 type or P for Rlessey type.
///@param byte IRrep: number of repeats
///@param int code: is IR code number.
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void IRsynthesis(byte IRtype, byte IRrep, int IRcode) {

	int check, i = 0;
	if (IRtype == 'P') //Plessey codes only uses bottom 5 bits, ignores any other bits.
	{
		//PLESSEY code
		Buffer[i=0,i++] = 0x8 << 28 | 0x524 << 16 | 0xa48 << 4 | 0x1 << 0;//0x08=header,200/0x524=pulse width, 0xa48=period, 2=skip 2 to start of data
		Buffer[i++] = 0x9016c010;//0x9=symbol, 0x16c=space, 0x10=Mark (16 pulses at period intervals) DATA 0
		Buffer[i++] = 0x900ed010;//0x9=symbol, 0xed =space, 0x10=Mark (16 pulses at period intervals) DATA 1 (Executes from here, no initial pulse)
		Buffer[i++] = 0x902e8010;//0x9=symbol, 0x2e8=space, 0x10=Mark (16 pulses at period intervals) INITIAL Space + PULSE =SYNC with previous symbol.
		Buffer[i++] = (((IRcode & 0x10) >> 4) + 1) | //MSB bit 4
				((((IRcode & 0x8) >> 3) + 1) << 4) | // bit 3
				((((IRcode & 0x4) >> 2) + 1) << 8) | // bit 2
				((((IRcode & 0x2) >> 1) + 1) << 16) | // bit 1
				((((IRcode & 0x1) >> 0) + 1) << 20) | //LSB bit 0
				(0x3 << 24) | //SYNC PULSE
				(0xa << 28); //0x0a=DATA structure for word.
		Buffer[i++] = 0xb << 28 | IRrep << 17 | 4 << 0; //repeat IRrep times from address 4
		Buffer[i++] = 0x00; //end of data.
		BUFLEN=i;
		for (; i < CaptureMax; i++) {
				Buffer[i] = 0;
			}
	}

	else if (IRtype == 'H') {
		//Possum HC8120 codes, only uses bottom 16 bits, ignores any other bits.
		check = (IRcode & 0xFF) ^ (IRcode >> 8 & 0xFF);
		Buffer[i=0,i++] = 0x8 << 28 | 0x524 << 16 | 0xa48 << 4 | 0x2 << 0;//0x08=header,0x524=pulsewidth, 0xa48=period, 2=skip 2 to start of data
//0x3DB is to get pulse as 1/16 of time. 50% would be 0x524, but pulse is 1/12 of time approx, so to make pulse 1/16 of time reduce pulse width.

		Buffer[i++] = 0x90010010;//0x9=symbol, 0x10=space, 0x10=Mark (16 pulses at period intervals) DATA 0
		Buffer[i++] = 0x90020010;//0x9=symbol, 0x10=space, 0x10=Mark (16 pulses at period intervals) DATA 1
		Buffer[i++] = 0x90A3D010;//0x9=symbol, 0xA3D=69ms, 0x76C=50ms space(in period intervals)( 474=30ms), 0x10=Mark (16 pulses at period intervals) INITIAL Space + PULSE =SYNC with previous symbol.
		Buffer[i++] = 1 | ((((IRcode >> 8) & 0x01) + 1) << 4)|
				((((IRcode >> 9)  & 0x01) + 1) << 8)  |
				((((IRcode >> 10) & 0x01) + 1) << 12) |
				((((IRcode >> 11) & 0x01) + 1) << 16) |
				((((IRcode >> 12) & 0x01) + 1) << 20) |
				((((IRcode >> 13) & 0x01) + 1) << 24) |
				(0xa << 28);

		Buffer[i++] = ((((IRcode >> 14) & 0x01) + 1) << 0) |
				((((IRcode >> 15)& 0x01) + 1) << 4)  |
				((((IRcode >> 0) & 0x01) + 1) << 8)	 |
				((((IRcode >> 1) & 0x01) + 1) << 12) |
				((((IRcode >> 2) & 0x01) + 1) << 16) |
				((((IRcode >> 3) & 0x01) + 1) << 20) |
				((((IRcode >> 4) & 0x01) + 1) << 24) |
				(0xa << 28);
		Buffer[i++] = ((((IRcode >> 5) & 0x01) + 1) << 0) |
				((((IRcode >> 6)& 0x01) + 1) << 4) |
				((((IRcode >> 7) & 0x01) + 1) << 8)|
				((((check >> 0) & 0x01) + 1) << 12) |
				((((check >> 1) & 0x01)	+ 1) << 16) |
				((((check >> 2) & 0x01) + 1) << 20) |
				((((check >> 3) & 0x01) + 1) << 24) |
				(0xa << 28);
		Buffer[i++] = ((((check >> 4) & 0x01) + 1) << 0) |
				((((check >> 5)	& 0x01) + 1) << 4) |
				((((check >> 6) & 0x01) + 1) << 8)|
				((((check >> 7) & 0x01) + 1) << 12) |
				(0xa << 28);

		Buffer[i++] = 0xb << 28 | IRrep << 17 | 3 << 0; //repeat rep times from address 3
		Buffer[i++] = 0x00; //end of data.
		BUFLEN=i;
		for (; i < CaptureMax; i++) {
			Buffer[i] = 0;
		}
	}
	else return;
}




void IRDMAVIVO(void)
{
//DMA experimental. Answer not fast enough for 455KHz using 4MHz clock.
//Max speed with 4MHz clock is around 100 to 125KHz.
//For 100MHz clock this translates as 5MHz.(Which is faster than current routines.
//set up to restart timer on match and update match by DMA.
// and toggle output.
//restart seems reliable
//DMA match update is every other match up until 4us
//after 4.25us it updates 3.75, 4, 4, 4.25, 4.5, 4.75 etc.(4MHz clock)

//From buffer it seems to start from 4, except 4.75 is repeated.

//From flash using const keyword it is about the same.
//putting processor to idle with __WFI is about the same.
// so DMA needs around 17-20 cycles per transfer.
//Using restart timer on match, if DMA misses it repeats the previous cycle.


	//uses TIM0 MAT0.0 DMA from memory.

	CPU100MHz();
		disableInputInterrupt();

//	const	int VIVOCODE[]={17,30,16,700,3700,30,16,30};		//const means held in flash

	int a,b,c,d,e,f,g,h;
	int i=0;

//section fill buffer
//system clock/8 10 is OK  	CLOCK/88, 22us at 4MHz.
//system clock/4 15 is OK 	CLOCK/64, 16us at 4MHz
//system clock/2 28 is OK 	CLOCK/58, 14.5us at 4MHz
//system clock/1 56 for OK	CLOCK/57, 14.25us at 4MHz

for(i=0;i<100;i++)
{
	Buffer[i]=47;		//56 is OK for 10 pulses. with flash wait at 4MHz
						//51 is good if no flash write waits at 4MHz.=(51+1)*250ns=13us
						//47 is good at 100MHz. =(47+1)*10ns = 480ns at 100MHz.

}
//Buffer[0]=100;// first pulse is low so does not give an IR output.
//Second pulse can be shorter???? if PINSEL after TCR. If PINSEL before TCR get initial glitch.




	LPC_SC->PCONP |= 1 << 29; // bit 29. GPDMA enable. TOM0 enables at power up.
	LPC_SC->PCONP |= 1 << 1; //enable TIMER0(Already enabled during power up.
	LPC_SC->PCLKSEL0 &= ~(3 << 2); //CLEAR TIMER0 PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= (1 << 2); //TIMER0 PREDIVIDE =1 CLK/1, 2=CLK/2 3= CLK/8 0=CLK/4
//	LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.


	//section sets output low, TCR enabled sets output low.
	//we need an even number of transfers to set output low at end and avoid glitch at end.
	LPC_TIM0->EMR = 1 | 1 << 4; //force output to low.		//also sets P1.28
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->TCR = 1;
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->EMR = 1 | 3 << 4; //force output to toggle.		//also sets P1.28
//end of section to avoid intial glitch



	LPC_TIM0->MCR = 2 << 0 ; //No int/timer reset/restart action on match
	LPC_TIM0->IR=1;		//reset MR0 DMA request if any.
	LPC_TIM0->MR0 = 10;		//initial period




	LPC_SC->DMAREQSEL=1;			//set timer0 match DMA instead of UART0.

	LPC_GPDMA->DMACSync=1<<8;			//sync DMA enable for MAT0.0

	LPC_GPDMA->DMACConfig=1;		//Enable DMA controller, little endian.

	LPC_GPDMA->DMACIntTCClear=1;	//clear channel 0 terminal count interrupt
	LPC_GPDMA->DMACIntErrClr=1;		//clear channel 0 errors
	//	2. Clear any pending interrupts on the channel to be used by writing to the
	//	DMACIntTCClear and DMACIntErrClear register. The previous channel operation
	//	might have left interrupt active.


	LPC_GPDMACH0->DMACCSrcAddr=(uint32_t)Buffer ;//  VIVOCODE ;
	//3 Write the source address into the DMACCxSrcAddr register.

	LPC_GPDMACH0->DMACCDestAddr=(uint32_t)&LPC_TIM0->MR0;
	//4	Write the destination address into the DMACCxDestAddr register.

	LPC_GPDMACH0->DMACCLLI=0;
	//5. Write the address of the next LLI into the DMACCxLLI register. If the transfer
	//comprises of a single packet of data then 0 must be written into this register.
	LPC_GPDMACH0->DMACCControl=
			40<<0|		//transfer size words ODD so last pulse is high, else glitch at end.
			0<<12|		//Source  size =1
			0<<15|		//destination  size =1
			2<<18|		//source width 2=word.
			2<<21|		//destination width 2=word
			1<<26|		//source increment
			1<<31;		//terminal count interrupt.

		//6 Write the control information into the DMACCxControl register.

	LPC_GPDMACH0->DMACCConfig=	1<<0|	//channel enable
								0<<1|	//source peripheral=memory
								8<<6|	//destination=TIMER0 MAT0.0
								1<<11;	//transfer type memory to peripheral


	//7. Write the channel configuration information into the DMACCxConfig register. If the
	//enable bit is set then the DMA channel is automatically enabled.



	LPC_PINCON->PINSEL3 |= (3 << 24); //first pulse starts from PINSEL.
	//pinsel makes output go high.
	//if PINSEL is first a longer first pulse, if PINSEL is after TCR a shorter first pulse.
	LPC_TIM0->TCR = 1;		//start timer 0. Output low to start, then goes high on start






	a=LPC_TIM0->TC;
	b=LPC_TIM0->MR0;
	c=LPC_GPDMACH0->DMACCDestAddr;
	while((LPC_GPDMA->DMACRawIntTCStat==0)&&(LPC_GPDMA->DMACRawIntErrStat==0));	//wait for end of transfer
	LPC_PINCON->PINSEL3= 0; (3 << 24); //set P1.28 as timer0 match 0.0

	h=LPC_TIM0->MR0;
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_PINCON->PINSEL3= 0; (3 << 24); //set P1.28 as timer0 match 0.0
us(100000);

	d=LPC_GPDMA->DMACRawIntTCStat;	//bit0 = 1 when finished.
	e=LPC_GPDMA->DMACRawIntErrStat;	//bit0 = 1 if error

	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_PINCON->PINSEL3= 0; (3 << 24); //set P1.28 as timer0 match 0.0

	while(1)
	{
		g=LPC_GPDMA->DMACRawIntTCStat;	//bit0 = 1 when finished.
		f=LPC_GPDMA->DMACRawIntErrStat;	//bit0 = 1 if error
		h=LPC_TIM0->MR0;
	}
		;				//wait for ever.
}



