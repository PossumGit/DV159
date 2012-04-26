/*
 ===============================================================================
 Name        : IR.c
 Author     : Duncan Irvine
 Version     : test
 Copyright   : Copyright (C)
 Description : IR routines.
 ===============================================================================
 */
//
/////////////////////////////////////////////////////////////////////////////////////////////////
////PUBLIC FUNCTIONS
//captureIR()
//playIR()
//initIT()
//
////PUBLIC VARIABLES
//because of limited RAM, these will be a UNION with other uses.
//Buffer[0] IRPulseWidth
//Buffer[1] IRPulses
//buffer[2] IRtime_ms
//BUFFER[0x10-0x1FFF] NEAT, Bluetooth, Audio and IR buffer.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


//includes
#include <cr_section_macros.h>
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "HUB.h"

//constants


#define CaptureTot	CaptureMax+CaptureExt
#define CaptureStart  1000000			//delay before replay IR 10ms delay
#define CaptureFirst	0x10			//first IR data in this position
//allows room for 16 words of data.
#define Second	100000000				//1 seconds worth of cycles at 100MHz.
#define WaitForIR	10*Second			//nominal 10s
#define WaitEndIR	3*Second			//nominal 3s

//name variables within Buffer. (Was union with struct but gave errors when big.
#define IRPulseWidth Buffer[0]
#define IRPulses Buffer[1]
#define IRtime_ms Buffer[2]

//Local functions
PRIVATE void startCaptureIR(void);
PRIVATE void endCaptureIR(void);
PRIVATE void startPlayIR(void);
PRIVATE void endPlayIR(void);
PRIVATE void correctIR(void);

//Public functions
//defined in HUB.h


//local variables.
PRIVATE int Pulses;
PRIVATE int PulseWidth = 100; //300 system clocks=3uS.
PRIVATE int EndPlay = 1;

//problem debugging big unions. Errors from target unless size is smaller.
//structure is preferable if debugging can be solved.
/*struct Capt{
 int IRPulses;			//count of pulses, includes block of data about IR (16 words.)
 int IRPulseWidth;		//in clock cycles
 int IRtime_ms;			// in ms
 };		// note sizeOf (Capt) must be < CaptureFirst.

 __DATA(RAM1)static int CaptureBuffer2[CaptureExt];

 __DATA(RAM2)

 static union{
 struct Capt CaptureData;
 int Buffer[CaptureMax];
 }store;

 */
//RAM region specific definitions.
__DATA(RAM1)int Buffer2[CaptureExt];

__DATA(RAM2)int Buffer[CaptureMax];

/////////////////////////////////////////////////////////////////////////////////////////////////
//captureIR
//returns time of capture.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void captureIR() {

	Pulses = CaptureFirst;
	LED3GREEN();
	startCaptureIR(); //initiate IR capture
	while ((Pulses != CaptureFirst) ? (LPC_TIM0->TC < WaitEndIR + LPC_TIM0->CR0)
			: (LPC_TIM0->TC < WaitForIR)) // IR timeouts, WaitEndIR(3s), WaitForIR(10s).Wait here during capture IR.
	{
	}
	endCaptureIR();
	LED3YELLOW();
	correctIR();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Replay IR
//returns number of IR Pulses captured.
//CaptureBuffer[x]>=CaptureStart(100000=1ms) is replay IR data
//CaptureBuffer[x]==0 is end of IR data.
////////////////////////////////////////////////////////////////////////////////////////////////
void playIR() {
	if (Pulses > 17) {
		startPlayIR();

		while (EndPlay >= CaptureStart)//wait here during play IR
		{
			__WFI(); //sleep, wait for interrupt.
		}
		endPlayIR();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//TIMER1 interrupt. Only TIMER1.MR0 can generate interrupt
//Used to play IR.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void TIMER1_IRQHandler(void) {
	//generate positive pulse on p1.28, length PulseWidth.
	//pulse width is held in timer0, pulse width is held in timer1.
	LPC_TIM0->EMR = 1 | 1 << 4; //Set P1.28, clear P1.28 MR0 on match
	LPC_TIM0->TCR = 1 | 0 << 1; //reset timer0 and start timer0// bit 1 has to be cleared for counting.
	//end of pulse section.
	//
	//read next word out of buffer.
	//less than 1000 is end of buffer.
	if (Pulses < CaptureMax) {
		EndPlay = Buffer[Pulses++];
		LPC_TIM1->MR0 = EndPlay; //set up next match.
	} else {
		EndPlay = 0;
	}
	LPC_TIM1->IR = (1 << 0); // reset MR0 interrupt (enable next interrupt)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//TIMER0 interrupt. Only TIMER0.CR0 can generate interrupt
//Used to capture IR.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void TIMER0_IRQHandler(void) {

	if (Pulses < CaptureMax) {
		Buffer[Pulses++] = LPC_TIM0->CR0;
	} else {
		endCaptureIR();
	}
	LPC_TIM0->IR = (1 << 4); //reset TIMER0.CR0 interrupt (enable next interrupt.)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Initialise IR CAPTURE.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
void startCaptureIR() {
	Pulses = CaptureFirst;
	LPC_SC->PCLKSEL0 &= ~(3 << 2); //CLEAR PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= 1 << 2; //TIMER0 PREDIVIDE =1 (system clock)
	LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.
	LPC_PINCON->PINSEL3 |= (3 << 20); //set P1.26 as capture 0.0
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt.
	LPC_TIM0->CCR |= (1 << 1) | (1 << 2); //capture  falling with interrupt
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->TCR = 1 | 0 << 1; //enable timer0 (start timer0)
	NVIC_EnableIRQ(TIMER0_IRQn); // enable interrupt
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//finalise IR CAPTURE.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
void endCaptureIR() {
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//initialise IR replay.
//IR replay uses timer1 to define length of IR periods,
//Timer0 is used to define pulse widths.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void startPlayIR() {

	EndPlay = CaptureStart; //set to 0 when end of buffer or no more captured data.
	LPC_TIM0->EMR = 1 | 1 << 4; //force output to 0.		//also sets P1.28
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->MR0 = 1;
	LPC_TIM0->MR0 = 0; //set and clear MR0 to clear P1.28
	LPC_PINCON->PINSEL3 |= (3 << 24); //set P1.28 as timer0 match 0.0
	Pulses = CaptureFirst; //for initial pulse.
	LPC_SC->PCLKSEL0 &= ~(3 << 4); //CLEAR PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= (1 << 4); //TIMER1 PREDIVIDE =1 (system clock)
	LPC_TIM1->PR = 0; //set IR sample clock
	NVIC_EnableIRQ(TIMER1_IRQn);
	LPC_TIM0->MR0 = PulseWidth; //generate match on MR0 after PulseWidth system clocks.(3uS)
	LPC_TIM0->MCR = 1 << 1 | 1 << 2; //stop and reset timer on match.
	LPC_TIM1->MR0 = Buffer[Pulses++];
	LPC_TIM1->MCR = 1; //enable MR0
	LPC_TIM1->TCR = 0 | 1 << 1; //enable timer 1 (reset timer1)
	LPC_TIM1->TCR = 1 | 0 << 1; //enable timer 1 (start timer1)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Finalise IR replay.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
void endPlayIR() {
	LPC_PINCON->PINSEL3 &= ~(3 << 24); //set P1.28 as GPIO, so force to 0 as output set to 0.
	LPC_TIM0->MCR &= ~0x07; //disable match interrupt
	LPC_TIM1->MCR &= ~0x07; //disable match interrupt
	while ((LPC_TIM0->MR0 > LPC_TIM0->TC)) // wait until last pulse has finished.
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Estimate pulse width from minimum distance between captures.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
void correctIR() {
	int minwidth = 2000; //100us is max pulse width to consider
	int b, a, i, width;

	a = Buffer[CaptureFirst] - CaptureStart;
	Buffer[CaptureFirst] = CaptureStart;
	for (i = CaptureFirst + 1; (i < Pulses); i++) {
		Buffer[i] = Buffer[i] - a; //reduce start delay to CaptureStart/10^7.
		b = Buffer[i]; //length of capture in clock cycles.
		// now find minimum pulse width.
		width = Buffer[i] - Buffer[i - 1];
		if ((width > 0) && (width < minwidth))
			minwidth = width;
	}
	minwidth = minwidth / 2;
	if ((minwidth > 50) && (minwidth <= 1000))
		PulseWidth = minwidth;
	else
		PulseWidth = 300; // may be glitch
	for (i = Pulses; i < CaptureMax; i++) {
		Buffer[i] = 0;
	}
	IRPulses = Pulses; //number of Pulses for IR.
	IRPulseWidth = PulseWidth; //Half minimum period for IR
	IRtime_ms = b / 100000; //ms time for IR
}

void initIR() {
	LPC_GPIO1->FIODIR |= 1 << 28; //IR defined as an output.
	LPC_GPIO1->FIOCLR |= 1 << 28; //clear IR output (IR off).
}
