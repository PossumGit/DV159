/*
 ===============================================================================
 Name        : IR.c
 Author     : Duncan Irvine
 Version     : test
 Copyright   : Copyright (C)
 Description : IR routines.
 ===============================================================================
 */

#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "HUB.h"

void startCaptureIR(void);
void endCaptureIR(void);
void startPlayIR(void);
void endPlayIR(void);

static int Pulses = 0;
static int PulseWidth = 100; //300 system clocks=3uS.
static int EndPlay=1;
const int CaptureMax = 8000;
static int CaptureBuffer[8000];

/////////////////////////////////////////////////////////////////////////////////////////////////
//captureIR
//returns time of capture.
//
////////////////////////////////////////////////////////////////////////////////////////////////
int captureIR() {
	int a, b, c, i;
	startCaptureIR(); //initiate IR capture

	while (Pulses ? (LPC_TIM0->TC < 200000000 + LPC_TIM0->CR0) : (LPC_TIM0->TC
			< 1000000000))// IR timeouts, edge?2s:10s  //Wait here during capture IR.
	{

	}

	endCaptureIR();
	a=CaptureBuffer[0]-1000;
	for (i=0;(i<CaptureMax)&&(CaptureBuffer[i]!=0);i++)
	{
	CaptureBuffer[i]=CaptureBuffer[i]-a; //reduce start delay to 10uS.
	b=CaptureBuffer[i];
	}
	return b; //period of capture.
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//Replay IR
//returns number of IR Pulses captured.
//
////////////////////////////////////////////////////////////////////////////////////////////////
int playIR() {
	startPlayIR();
	while (EndPlay>=1000)//wait here during play IR
		{
		__WFI();		//sleep, wait for interrupt.
	}
	endPlayIR();
	if (Pulses>1)Pulses--;//because it increments after it finds a 0 in CaptureBuffer.
	return Pulses;
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


//read next word out of buffer.
//less than 1000 is end of buffer.
	if (Pulses < CaptureMax) {
		EndPlay = CaptureBuffer[Pulses++];
		LPC_TIM1->MR0 = EndPlay;		//set up next match.
	} else {
		EndPlay = 0;
	}
	LPC_TIM1->IR = (1 << 0);// reset MR0 interrupt (enable next interrupt)
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//TIMER0 interrupt. Only TIMER0.CR0 can generate interrupt
//Used to capture IR.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void TIMER0_IRQHandler(void) {

		if (Pulses < CaptureMax) {
			CaptureBuffer[Pulses++] = LPC_TIM0->CR0;
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
		LPC_SC->PCLKSEL0 &= ~(3 << 2); //CLEAR PREDIVIDE bits.
		LPC_SC->PCLKSEL0 |= 1 << 2; //TIMER0 PREDIVIDE =1 (system clock)
		LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.
		LPC_PINCON->PINSEL3 |= (3 << 20); //set P1.26 as capture 0.0
		LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt.
		LPC_TIM0->CCR |= (1 << 1) | (1 << 2); //capture  falling with interrupt
		Pulses = 0; 				//for initial pulse.
		LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
		LPC_TIM0->TCR = 1 | 0 << 1; //enable timer0 (start timer0)
		NVIC_EnableIRQ(TIMER0_IRQn);// enable interrupt
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
		EndPlay=1000;			//set to 0 when end of buffer or no more captured data.
		LPC_TIM0->EMR = 1 | 1 << 4; //force output to 0.		//also sets P1.28
		LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
		LPC_TIM0->MR0 = 1;
		LPC_TIM0->MR0 = 0;	//set and clear MR0 to clear P1.28
		LPC_PINCON->PINSEL3 |= (3 << 24); //set P1.28 as timer0 match 0.0
		Pulses = 0; //for initial pulse.
		LPC_SC->PCLKSEL0 &= ~(3 << 4); //CLEAR PREDIVIDE bits.
		LPC_SC->PCLKSEL0 |= (1 << 4); //TIMER1 PREDIVIDE =1 (system clock)
		LPC_TIM1->PR = 0; //set IR sample clock
		NVIC_EnableIRQ(TIMER1_IRQn);
		LPC_TIM0->MR0 = PulseWidth; //generate match on MR0 after PulseWidth system clocks.(3uS)
		LPC_TIM0->MCR =1<<1|1<<2;		//stop and reset timer on match.
		LPC_TIM1->MR0 = CaptureBuffer[Pulses++];
		LPC_TIM1->MCR = 1; //enable MR0
		LPC_TIM1->TCR = 1 | 0 << 1; //enable timer 1 (start timer1)
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Finalise IR replay.
	//
	//
	////////////////////////////////////////////////////////////////////////////////////////////////
	void endPlayIR()
	{
		LPC_PINCON->PINSEL3 &= ~(3 << 24); //set P1.28 as GPIO, so force to 0 as output set to 0.
		LPC_TIM1->MCR &= ~0x01; //disable match interrupt
	}


