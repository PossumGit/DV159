///@name        	IR Capture and Replay
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK
///@date			23 April 2012


//defines
#define CaptureStart  1000000			///< Delay before replay IR 10ms delay
#define CaptureFirst	0x10			///< First IR data in this position, allow 16 control words before IR data
#define Second	100000000				///< 1 seconds worth of cycles at 100MHz.
#define WaitForIR	10*Second			///< Nominal 10s
#define WaitEndIR	3*Second			///< Nominal 3s
#define IRPulseWidth Buffer[1]		///< Width of IR pulse, stored in Buffer[] control area
#define IRPulses Buffer[2]			///<Number of IR pulses, stored in Buffer[] control area
#define IRtime_ms Buffer[3]			///<Total length of IR signal in ms, stored in Buffer[] control area

//includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"

//public variables

//local variables.
PRIVATE int Pulses;				///< Count IR pulses during capture/replay
PRIVATE int PulseWidth = 100; 	///< 100 system clocks=1uS.
PRIVATE int EndPlay = 1;		///< Used to communicate end from interrupt routines.

//external variables
EXTERNAL int Buffer[];		///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture

//Local functions
PRIVATE void startCaptureIR(void);
PRIVATE void endCaptureIR(void);
PRIVATE void startPlayIR(void);
PRIVATE void endPlayIR(void);
PRIVATE void correctIR(void);

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Captures an IR signal and stores in Buffer[].
///@param void
///@return void
///@par Modifies
/// Buffer[]: Save IR data to Buffer[]
///@par Time
/// WaitEndIR(3s) wait for repeat IR. WaitForIr(10s) wait for first IR
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void captureIR(void) {

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
///@brief Replay IR signal stored in Buffer[]
///@param void
///@return void
///@par Modifies
/// Buffer[]: Save IR data to Buffer[]
///@par Notes
/// CaptureBuffer[x]>=CaptureStart(100000=1ms) is replay IR data.
/// CaptureBuffer[x]==0 is end of IR data.
///@par Time
/// WaitEndIR(3s) wait for repeat IR. WaitForIr(10s) wait for first IR
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void playIR(void) {
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
///@brief TIMER1 interrupt. Only TIMER1.MR0 can generate this interrupt
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
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
///@brief TIMER0 interrupt. Only TIMER0.CR0 can generate this interrupt
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
 void TIMER0_IRQHandler(void) {

	if (Pulses < CaptureMax) {
		Buffer[Pulses++] = LPC_TIM0->CR0;
	} else {
		endCaptureIR();
	}
	LPC_TIM0->IR = (1 << 4); //reset TIMER0.CR0 interrupt (enable next interrupt.)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Initialise for IR capture.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void startCaptureIR(void) {
	Pulses = CaptureFirst;
	LPC_SC->PCLKSEL0 &= ~(3 << 2); //CLEAR PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= 1 << 2; //TIMER0 PREDIVIDE =1 (system clock)
	LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.
	LPC_PINCON->PINSEL3 |= (3 << 20); //set P1.26 as capture 0.0
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt.
	LPC_TIM0->CCR |= (1 << 1) | (1 << 2); //capture  falling with interrupt
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->TCR = 1 | 0 << 1; //enable timer0 (start timer0)

	NVIC->ISER[0]=1<<1;			//enable TIMER0 interrupt.

	//_EnableIRQ(TIMER0_IRQn); // enable interrupt
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Initialise for IR replay.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void startPlayIR(void) {

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

	NVIC->ISER[0]=1<<2;
//	NVIC_EnableIRQ(TIMER1_IRQn);
	LPC_TIM0->MR0 = PulseWidth; //generate match on MR0 after PulseWidth system clocks.(3uS)
	LPC_TIM0->MCR = 1 << 1 | 1 << 2; //stop and reset timer on match.
	LPC_TIM1->MR0 = Buffer[Pulses++];
	LPC_TIM1->MCR = 1; //enable MR0
	LPC_TIM1->TCR = 0 | 1 << 1; //enable timer 1 (reset timer1)
	LPC_TIM1->TCR = 1 | 0 << 1; //enable timer 1 (start timer1)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Finalise for IR replay.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void endPlayIR(void) {
	LPC_PINCON->PINSEL3 &= ~(3 << 24); //set P1.28 as GPIO, so force to 0 as output set to 0.
	LPC_TIM0->MCR &= ~0x07; //disable match interrupt
	LPC_TIM1->MCR &= ~0x07; //disable match interrupt
	NVIC->ICER[0]=1<<1|1<<2;			//disable TIMER0 and TIMER1 interrupts.
	while ((LPC_TIM0->MR0 > LPC_TIM0->TC)) // wait until last pulse has finished.
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Post process IR capture.
///@param void
///@return void
///@par Modifies
/// Buffer[]: Process data within buffer.
/// Save data within Buffer[].
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void correctIR(void) {
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
	Buffer[0]='I'|'R'<<8|' '<<16|' '<<24;

	IRPulses = Pulses; //number of Pulses for IR.
	IRPulseWidth = PulseWidth; //Half minimum period for IR
	IRtime_ms = b / 100000; //ms time for IR
	Buffer[4]=1;
	Buffer[5]=1;
	Buffer[6]=1;
	Buffer[7]=1;
	Buffer[8]=1;
	Buffer[9]=1;
	Buffer[10]=1;
	Buffer[11]=1;
	Buffer[12]=1;
	Buffer[13]=1;
	Buffer[14]=1;
	Buffer[15]=1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn InfraRed LED off.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initIR(void) {
	LPC_GPIO1->FIODIR |= 1 << 28; //IR defined as an output.
	LPC_GPIO1->FIOCLR |= 1 << 28; //clear IR output (IR off).
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Finalise for IR capture.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void endCaptureIR(void) {
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt
	NVIC->ICER[0]=1<<1|1<<2;			//disable TIMER0 and TIMER1 interrupts.
}


