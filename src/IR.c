///@name        	IR Capture and Replay
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK
///@date			23 April 2012
//defines
#define CaptureStart  100000			///< Delay before replay IR 1ms delay
#define CaptureFirst	0x01			///< First IR data in this position, allow 16 control words before IR data
#define Second	100000000				///< 1 seconds worth of cycles at 100MHz.
#define Sec3		3*Second				///< 3 seconds worth of cycles at 100MHz.
#define WaitForIR	10*Second			///< Nominal 10s
#define WaitEndIR	3*Second			///< Nominal 3s
//includes

#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
//public variables
//local variables.

PRIVATE volatile word IRAddress = 0; ///< Count IR pulses during capture/replay
PRIVATE volatile word PulseWidth = 100; ///< 100 system clocks=1uS.
PRIVATE volatile word IRData = 1; ///< Used to communicate end from interrupt routines.
PRIVATE volatile word IRTimeMatch = 1;///< Last time set into match register.
PRIVATE volatile word Period = 2632; ///<38KHz default.
PRIVATE volatile int Mark = 0x10000; ///<Mark burst in IR compression processing. Initial is bigger than max mark.
PRIVATE volatile word Space = 0; ///<Space gap between bursts in IR compression.
PRIVATE volatile word SymbolBank = 0; ///<location of first symbol (skip) in SYMBOL BANK
PRIVATE volatile word SymbolWord = 0;	///< a word out of the symbol bank.
PRIVATE volatile word DataStep = 0; ///<used in compression timer1 ISR
PRIVATE volatile word Pulse = 1; ///<1=pulse, 0=no pulse.
PRIVATE volatile word Delay = 0;///<delay in compress routine.
PRIVATE volatile int COUNT[] = { 0, 0, 0, 0, 0, 0, 0, 0 };///< used for loops in compress.
PRIVATE volatile word plast;
PRIVATE volatile word rlast;
PRIVATE volatile Tick=0;
PRIVATE int EndIR;
PRIVATE volatile int MarkCount;

//external variables
EXTERNAL volatile int Buffer[]; ///< Whole of RAM2 is Buffer, reused for NEAT, Bluetooth, audio and IR replay and capture
PUBLIC	int BUFLEN;
//Local functions
PRIVATE void startCaptureIR(void);
PRIVATE void endCaptureIR(void);
PRIVATE void startPlayIR(void);
PRIVATE void endPlayIR(void);
PRIVATE void correctIR(void);
PRIVATE void compress(void);

//External functions
EXTERNAL void	LED1YELLOW(void);
EXTERNAL void	LED1GREEN(void);
EXTERNAL void	LED1OFF(void);
EXTERNAL void CPU100MHz (void);
EXTERNAL void CPU12MHz(void);
EXTERNAL int repeatInput(void);
EXTERNAL void txBT(void);
EXTERNAL byte	inputTest(void);
EXTERNAL void disableInputInterrupt(void);

EXTERNAL int	PCBiss;		//=3 for PCHB issue 3, =4 for PCB issue 4.
EXTERNAL void receiveBTbuffer(int,int);
//EXTERNAL word	inputTime(void);
// PUBLIC functions

PUBLIC void PCLKSEL (void);
PUBLIC int captureIR(void);
PUBLIC void playIR(void);
PUBLIC void streamIR(int);
PUBLIC int streamcaptureIR(void);
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
PUBLIC int captureIR(void) {

	int a;
	for (a=0;a<8;a++)
	{
	COUNT[a]=0;
	}
		IRAddress = 0; ///< Count IR pulses during capture/replay
		PulseWidth = 100; ///< 100 system clocks=1uS.
		IRData = 1; ///< Used to communicate end from interrupt routines.
		IRTimeMatch = 1;///< Last time set into match register.
		Period = 2632; ///<38KHz default.
		Mark = 0x10000; ///<Mark burst in IR compression processing. Initial is bigger than max mark.
		Space = 0; ///<Space gap between bursts in IR compression.
		SymbolBank = 0; ///<location of first symbol (skip) in SYMBOL BANK
		SymbolWord = 0;	///< a word out of the symbol bank.
		DataStep = 0; ///<used in compression timer1 ISR
		Pulse = 1; ///<1=pulse, 0=no pulse.
		Delay = 0;



	IRAddress = CaptureFirst;
	LED1GREEN();

//	PCLKSEL();
	CPU100MHz();		//also disables ext interrupts.
#if release==1
	LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
if (PCBiss==3||PCBiss==4)
{

	LPC_GPIO1-> FIOSET |=1<<29			;//IR capture on.
}

	//CPU100MHz disables interrupts except TIMER 0 and TIMER 1

	startCaptureIR(); //initiate IR capture
	while ((IRAddress != CaptureFirst) ? (LPC_TIM0->TC < WaitEndIR
			+ LPC_TIM0->CR0) : (LPC_TIM0->TC < WaitForIR)) // IR timeouts, WaitEndIR(3s), WaitForIR(10s).Wait here during capture IR.
	{
		if (1 & LPC_UART1->LSR) //if character comes in from bluetooth, read char and abort if its not an A.
		if('A'!=LPC_UART1->RBR)	break;
		repeatInput();	//change of input?
		txBT();		//send any available data from change of input to BT.
		if (IRAddress >= CaptureMax) break;
	//	if(LPC_TIM0->CR0 > 0x7e000000)break;

		if((IRAddress!=CaptureFirst)&&(LPC_TIM0->TC > WaitEndIR + LPC_TIM0->CR0))break;
		if ((IRAddress == CaptureFirst) && (LPC_TIM0->TC > WaitForIR))break;
	}
	BUFLEN=IRAddress+1;
	endCaptureIR();

	correctIR();

	CPU12MHz();

	LED1OFF();
if (PCBiss==3||PCBiss==4)
{

	LPC_GPIO1->FIOCLR |=1<<29			;//IR capture off.
}

#if release==1
	LPC_WDT->WDTC = 10000000;	//set timeout 40s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
	if (IRAddress > CaptureFirst)
		return 1;
	return 0;

}





/*
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
PUBLIC int streamcaptureIR(void) {


	IRAddress = CaptureFirst;
	LED1GREEN();
	CPU100MHz();		//also disables ext interrupts.
#if release==1
	LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
if (PCBiss==3||PCBiss==4)
{

	LPC_GPIO1-> FIOSET |=1<<29			;//IR capture on.
}

	//CPU100MHz disables interrupts except TIMER 0 and TIMER 1

	startCaptureIR(); //initiate IR capture
	while ((IRAddress != CaptureFirst) ? (LPC_TIM0->TC < WaitEndIR
			+ LPC_TIM0->CR0) : (LPC_TIM0->TC < WaitForIR)) // IR timeouts, WaitEndIR(3s), WaitForIR(10s).Wait here during capture IR.
	{
	//	if (1 & LPC_UART1->LSR) //if character comes in from bluetooth, read char and abort if its not an A.
	//	if('A'!=LPC_UART1->RBR)	break;
	//	repeatInput();	//change of input?
	//	txBT();		//send any available data from change of input to BT.






		if (IRAddress >= CaptureMax) break;
	//	if(LPC_TIM0->CR0 > 0x7e000000)break;
	}
	BUFLEN=IRAddress+1;
	endCaptureIR();

	correctIR();

	CPU12MHz();

	LED1OFF();
if (PCBiss==3||PCBiss==4)
{

	LPC_GPIO1->FIOCLR |=1<<29			;//IR capture off.
}

#if release==1
	LPC_WDT->WDTC = 10000000;	//set timeout 40s watchdog timer
	LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
	LPC_WDT->WDFEED=0x55;			//watchdog feed
#endif
	if (IRAddress > CaptureFirst)
		return 1;
	return 0;

}







*/




















/*

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief stream IR starting from address 0, stream BT starting from address start
///@param int start
///@return void
///@par Modifies
/// Buffer[]: Save IR data to Buffer[]
///@par Notes
/// CaptureBuffer[x]>=CaptureStart(100000=1ms) is replay IR data.
/// CaptureBuffer[x]==0 is end of IR data.
///@par Time
/// WaitEndIR(3s) wait for repeat IR. WaitForIr(10s) wait for first IR
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void streamIR(start) {

int a;

//	LED2GREEN();
//	if (Buffer[2] != 0) {
		LED1GREEN();
		CPU100MHz();	//CPU100MHz disables interrupts except TIMER 0 and TIMER 1
#if release==1
		LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif

		receiveBTbuffer(0,start);
		startPlayIR();
		receiveBTbuffer(start,0x2000);
		while (IRData > 0)//wait here during play IR
		{
	//		if (IRData<LPC_TIM1->TC)
//			{
//				a=1;
//				break;
//			}

		//	if (1 & LPC_UART1->LSR) //if character comes in from bluetooth, read char and abort if its not an A.
		//	if('A'!=LPC_UART1->RBR)	break;
		//	repeatInput();	//change of input?
		//	txBT();		//send any available data from change of input to BT.
		}
		LED1OFF();


		endPlayIR();

#if release==1
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif
	//}
		CPU12MHz();
}
*/



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

int a;
for (a=0;a<8;a++)
{
COUNT[a]=0;
}

//	PulseWidth = 100; ///< 100 system clocks=1uS.
//	IRData = 1; ///< Used to communicate end from interrupt routines.
	IRTimeMatch = 1;///< Last time set into match register.
//	Period = 2632; ///<38KHz default.
	Mark = 0x10000; ///<Mark burst in IR compression processing. Initial is bigger than max mark.
	Space = 0; ///<Space gap between bursts in IR compression.
	SymbolBank = 0; ///<location of first symbol (skip) in SYMBOL BANK
	SymbolWord = 0;	///< a word out of the symbol bank.
	DataStep = 0; ///<used in compression timer1 ISR
	Pulse = 1; ///<1=pulse, 0=no pulse.
	Delay = 0;

//	LED2GREEN();
	if (Buffer[2] != 0) {
		LED1GREEN();

//		PCLKSEL();
		CPU100MHz();	//CPU100MHz disables interrupts except TIMER 0 and TIMER 1
#if release==1
		LPC_WDT->WDTC = 40000000;	//set timeout 40s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif



		startPlayIR();
		while (IRData > 0)//wait here during play IR
		{
	//		if (IRTimeMatch+100000<LPC_TIM1->TC)
	//		{
	//			break;	//abort if interrupt missed.
	//		}
	//		if(0x24000000<LPC_TIM1->TC)break;//break if time>$000
			if (1 & LPC_UART1->LSR) //if character comes in from bluetooth, read char and abort if its not an A.
			if('A'!=LPC_UART1->RBR)
				{
				break;
				}
			repeatInput();	//change of input?
			txBT();		//send any available data from change of input to BT.
		}
		LED1OFF();

		repeatInput();	//change of input?
		txBT();		//send any available data from change of input to BT.
		endPlayIR();

#if release==1
		LPC_WDT->WDTC = 10000000;	//set timeout 5s watchdog timer
		LPC_WDT->WDFEED=0xAA;			//watchdog feed, no interrupt in this sequence.
		LPC_WDT->WDFEED=0x55;			//watchdog feed

#endif
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief TIMER1 interrupt. Only TIMER1.MR0 can generate this interrupt
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////



///
///
///change,
///interrupt 20us before pulse is due
void TIMER1_IRQHandler(void) {
	//generate positive pulse on p1.28, length PulseWidth.
	//pulse width is held in timer0.
//	int t1,t2,t3,t4;

//	if (Pulse)//if Pulse=0 then no IR pulse, else IR pulse.
//	{
//		LPC_TIM0->EMR = 1 | 1 << 4; //Set P1.28, clear P1.28 MR0 on match
//		LPC_TIM0->TCR = 1 | 0 << 1; //reset timer0 and start timer0// bit 1 has to be cleared for counting.
//	}


	//end of pulse section.
	//
	//
	//read next word out of buffer.
	//0 is end of buffer.
//	LED2GREEN();
	if (IRAddress < CaptureMax) {


//		IRData = Buffer[IRAddress];
//		if (IRData & (1 << 31))
			compress();

//		else {
			//Raw format data.
/*
			 if(Tick)
					 {
					carrier(1, 0, IRTimeMatch);	//generate pulses on IRTimeMatch.
					Tick=0;
					 }


			IRTimeMatch = IRData; //save value of EndPlay for next interrupt.
			LPC_TIM1->MR0 = IRTimeMatch; //set up next match.
			IRAddress++;
			Pulse=1;
		}

	} else {
		IRData = 0; //end of replay.
	//	return;
*/
	}

	LPC_TIM1->IR = (1 << 0); // reset MR0 interrupt (enable next interrupt)
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief compress acts to decompress IR data on the fly.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void compress(void) {
	//coded scheme if most significant bit is 1.
	int a=0, b=0,t1,t2,t3,t4;
	word IRAddresssave;
	word IRspace=0, t=0;


	while (1) {
		if (IRAddress >= CaptureMax)
		{
		IRData = 0;
		return;}

		IRData = Buffer[IRAddress];
		if (0 == IRData)
		return; //Zero, end of IR.

		switch ((IRData >> 28) & 0xF) {

		case 0b1001://SYMBOL
		{


			 if(Tick)
			 {
			carrier(MarkCount, Period, IRTimeMatch);	//generate Markcount pulses.
			IRTimeMatch = IRTimeMatch+Period*MarkCount;
			Tick=0;
			IRAddress++;// go to next word.
				break;
			 }
			 else
			 {
			Space = (IRData >> 12) & 0xFFFF;
			IRspace = Space * Period;
			MarkCount = IRData & 0xFFF;
					if (IRspace > Sec3)
					IRspace = Sec3; //maximum space=3s.
				IRTimeMatch = IRTimeMatch + IRspace;
				if ((IRTimeMatch - LPC_TIM1->TC) > IRreturn)// If time to next pulse>50us, exit interrupt.
				{
					Tick = 1;

					LPC_TIM1->MR0 = IRTimeMatch - IRinterrupt; //return 40us before pulse.
					return;
				}
				IRTimeMatch = IRTimeMatch + IRspace;
				carrier(MarkCount, Period, IRTimeMatch); //generate Markcount pulses.
				IRTimeMatch = IRTimeMatch + Period;
				IRAddress++;// go to next word.

			}


		}
		case 0b1010://DATA

		 {
			//first symbol in buffer is dummy.
			//space is always first, followed by mark.
			//code is structured to speed up mark at expense of space. Space is longer so less time critical.
			//mark can have 0 in it. Then you get 1 pulse.
	//		if (Mark < (SymbolWord & 0xFFF)) {
	//			Mark++;
	//			IRTimeMatch = IRTimeMatch + Period;//save value of IRTimeMatch for next interrupt.
	//			LPC_TIM1->MR0 = IRTimeMatch; //set up next match, pulse after period.
	//			return; //exit compress and finish  interrupt.
	//		}  //get next symbol.



			 while(1)
			 {
				 if(Tick)
				 {
				carrier(MarkCount, Period, IRTimeMatch);	//generate Markcount pulses.
				IRTimeMatch = IRTimeMatch+Period;
				Tick=0;
				 }
				do {
					a = (IRData >> DataStep) & 0xF;
					DataStep += 4;
				} while (0 == a && DataStep < 32); //skip all 0 nibbles.

				if (DataStep >= 32) {		//get next data word
					IRAddress++;
					DataStep = 0;
					break;
				}
				else						//do space then mark
				{
					SymbolWord = Buffer[SymbolBank + a]; //load SWord with correct SYMBOL from SYMBOL BANK.
					Space = (SymbolWord >> 12) & 0xFFFF;
					IRspace = Space * Period;
					MarkCount = SymbolWord & 0xFFF;
					if (IRspace > Sec3)	IRspace = Sec3; //maximum space=3s.
					IRTimeMatch = IRTimeMatch + IRspace;
					if ((IRTimeMatch-LPC_TIM1->TC)> IRreturn)// If time to next pulse>IRreturn, exit interrupt.
					{
						Tick=1;

						LPC_TIM1->MR0 = IRTimeMatch-IRinterrupt;	//return IRinterrupt before pulse.
						return;
					}
					IRTimeMatch = IRTimeMatch + IRspace;
					carrier(MarkCount, Period, IRTimeMatch);	//generate Markcount pulses.
					IRTimeMatch = IRTimeMatch+Period;
				}


				}

			break;
		}

		case 0b1011://REPEAT
		{
			a = (IRData >> 25) & 0x7;
			b = ((IRData >> 17) & 0xFF) - 1;//COUNT=0,1 gives 1 rep, 2=2 reps, 3=3 reps
			if (COUNT[a] < b) {
				COUNT[a]++;
				IRAddress = IRData & 0x7FFF; //limit is 4 times available memory
				IRData = Buffer[IRAddress];
				break;
			} else {
				COUNT[a] = 0;
				IRAddress++;//ignore , go to next word.
				if (IRAddress >= CaptureMax)
					IRData = 0;
				else
					IRData = Buffer[IRAddress];
				break;
			}

			IRAddress++; //ignore , go to next word.
			if (IRAddress >= CaptureMax)
				IRData = 0;
			else
				IRData = Buffer[IRAddress];
			break;
		}



		case 0b1100://INPUT
			{
				a = 0x30 & inputTest();
//				t = inputTime();
				switch (0x7 & (IRData >> 25)) {
				case 0: //DELAY, ABORT if input released
				{
					if (0x30!=a) //input pressed. 0x0=both pressed, 0x30 both released 0x10 int pressed 0x20 ext pressed.
					{
						if (Delay  >= (IRData & 0xfff)) {
							Delay = 0;
							Pulse=1;
							IRAddress++; // go to next word.
							if (IRAddress >= CaptureMax)
								IRData = 0;
							else
								IRData = Buffer[IRAddress];

						}else//add 50ms delay and try again.
						{

						//generate interrupts at 50ms intervals.
						Delay++;
						Pulse = 0; //interrupt but no pulse.
						IRTimeMatch = IRTimeMatch + 1000000;//10ms, save value of IRTimeMatch for next interrupt.
						LPC_TIM1->MR0 = IRTimeMatch; //set up next match, pulse after period.
						return; //exit compress and finish  interrupt.
						}
					}else
					{		//input released so abort.
									Delay=0;
									Pulse=1;
									IRData = 0;
					} //delay finshed, then continue.
					break;
				}
				case 1://DELAY, ABORT if input pressed.
				{
							if (0x30!=a) //input pressed so ABORT.
							{
								Delay=0;
								Pulse=1;
								IRData = 0;
							} else //delay finshed, then continue.
							{

								if (Delay  >= (IRData & 0xfff)) {
									Delay = 0;
									Pulse=1;
									IRAddress++; // go to next word.
									if (IRAddress >= CaptureMax)
										IRData = 0;
									else
										IRData = Buffer[IRAddress];

								}else//add 10ms delay and try again.
								{

								//generate interrupts at 10ms intervals.
								Delay++;
								Pulse = 0; //interrupt but no pulse.
								IRTimeMatch = IRTimeMatch + 1000000;//10ms, save value of IRTimeMatch for next interrupt.
								LPC_TIM1->MR0 = IRTimeMatch; //set up next match, pulse after period.
								return; //exit compress and finish  interrupt.
								}
							}
							break;
						}
				case 2://SUBSIDIARY MODE
				{

					if (0x30!=a) //input pressed. 0= not pressed,1=internal, 2=external, 3=both.
					{
						plast=IRTimeMatch;
						t=IRTimeMatch-rlast;		//time since last release in 10ns.
						if (t > ((IRData & 0xff) * 100000000))//greater than xx seconds pressed.
						{
							Pulse = 1;
							IRData = 0; //abort
						} else//less than xx seconds pressed.
						{
							Pulse = 1;
							IRAddress++; // go to next word.
							if (IRAddress >= CaptureMax)
								IRData = 0;
							else
								IRData = Buffer[IRAddress];
						}

					}

					else //input released
					{
						rlast=IRTimeMatch;
						t=IRTimeMatch-plast;		//time since last press in 10ns.
						if (t > (((IRData >> 8) & 0xff) * 100000000)) //seconds released
						{
							IRData = 0; //abort
							Pulse = 1;
						} else//waiting for released timeout
						{
							//generate interrupts at 50ms intervals.
							Pulse = 0; //interrupt but no pulse.
							IRTimeMatch = IRTimeMatch + 5000000;//50ms, save value of IRTimeMatch for next interrupt.
							LPC_TIM1->MR0 = IRTimeMatch; //set up next match, pulse after period.
							return; //exit compress and finish  interrupt.
						}
					}

					break;
				}
				case 3:		//abort if time>xx, abort if released.
				{
					if (0x30!=a) //input pressed. 0= not pressed,1=internal, 2=external, 3=both.
							{

								if (IRTimeMatch > ((IRData & 0xff) * 100000000))//greater than xx seconds pressed.
								{
									Pulse = 1;
									IRData = 0; //abort
								} else//less than xx seconds pressed.
								{
									Pulse = 1;
									IRAddress++; // go to next word.
									if (IRAddress >= CaptureMax)
										IRData = 0;
									else
										IRData = Buffer[IRAddress];
								}

							}

							else //input released abort.
							{
								Delay=0;
								Pulse=1;
								IRData = 0;
							}
					break;
				}

						default: {
					IRAddress++;//ignore , go to next word.
					break;
				}
				}

				break;
			}

		case 0b1000: //HEADER
		{




			PulseWidth = (IRData >> 16) & 0x7FF;//SET Pulse Width from 50(0.5us) to 2047(20.47us)

			Period = (IRData >> 4) & 0xFFF; //SET Period maximum 4095 (40.95us, 24.42KHz) 30KHz-48KHz,455KHz,typical 38KHz(26.32us).
			if((Period>100) && (PulseWidth*2>Period))PulseWidth=Period/2;
			SymbolBank = IRAddress; //new header has new symbol bank.
			IRAddress = IRAddress + 1 + (IRData & 0xF); //next address, skip any symbol bank.
			IRData=Buffer[IRAddress];

			if (IRAddress>=CaptureMax)
				{

					IRData=0;
				}
				else
				{
				IRData=Buffer[IRAddress];

				}

				if(IRData<0x80000000)
				{if ((IRData - LPC_TIM1->TC) > IRreturn)
				{
				LPC_TIM1->MR0 = IRData-IRinterrupt;		//interrupt with IRinterrupt*10ns left
				return;
				}
				}

	//		IRAddress=raw(IRAddress, CaptureMax-IRAddress,IRreturn);	//(IRAddress, MaxCount, Maxtime)

			break;
			}



		case 0b1101://RESERVED
		{
			IRAddress++;//ignore , go to next word.
			if (IRAddress >= CaptureMax)
				IRData = 0;
			else
				IRData = Buffer[IRAddress];
			break;
		}
		case 0b1110://RESERVED
		{
			IRAddress++;//ignore , go to next word.
			if (IRAddress >= CaptureMax)
				IRData = 0;
			else
				IRData = Buffer[IRAddress];
			break;
		}
		case 0b1111://RESERVED
		{
			IRAddress++;//ignore , go to next word.
			if (IRAddress >= CaptureMax)
				IRData = 0;
			else
				IRData = Buffer[IRAddress];
			break;
		}

		default://RAW data.
			LPC_TIM0->TCR=0;		//required to make MR0 stick
			LPC_TIM0->TCR=1;		//required to make MR0 stick, else delayed by 1 pulse.
			LPC_TIM0->MR0 = PulseWidth;

			IRAddresssave=IRAddress;
			IRAddress=raw(IRAddress, CaptureMax-IRAddress,IRreturn);	//(IRAddress, MaxCount, Maxtime)
//raw is asm which keeps doing pulses until a longer gap arises or until end of buffer.
			if ((IRAddresssave-IRAddress)>=1)
			{
				IRTimeMatch=Buffer[IRAddress-1];
			}



			if (IRAddress>=CaptureMax)
			{

				IRData=0;
			}
			else
			{
			IRData=Buffer[IRAddress];
			}



			if(IRData<0x80000000)
			{
	//		IRTimeMatch = IRData; //save value of IRTimeMatch for next interrupt.
			LPC_TIM1->MR0 = IRData-IRinterrupt;		//interrupt with IRinterrupt*10ns left
			return;
			}
			break;
		}

	}
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief TIMER0 interrupt. Only TIMER0.CR0 can generate this interrupt
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
void TIMER0_IRQHandler(void) {



	if ((IRAddress < CaptureMax)) {
		Buffer[IRAddress++] = LPC_TIM0->CR0;
	} else {
		endCaptureIR();
	}
	LPC_TIM0->IR = (1 << 4); //reset TIMER0.CR0 interrupt (enable next interrupt.)
}


void PCLKSEL (void)
{
	//capture
	LPC_SC->PCLKSEL0 =0;
	LPC_SC->PCLKSEL0 =((1 << 2)|(1<<4)|3<< 8);	//TIMER0, TIMER1, 115.2KBAUD
	LPC_SC->PCLKSEL1 =0;
	LPC_SC->PCLKSEL1 |= 1 << 10;	// SSP0


//	LPC_SC->PCLKSEL0 &= ~((3 << 2)|(3<<4)); //CLEAR PREDIVIDE bits.
//	LPC_SC->PCLKSEL0 |= ((1 << 2)|(1<<4)); //TIMER0 PREDIVIDE =1 (system clock)|TIMER1 PREDIVIDE =1 (system clock)
	//replay audio
//	LPC_SC->PCLKSEL0 &= ~((3 << 2)|(3<<4)); //CLEAR PREDIVIDE bits.
//	LPC_SC->PCLKSEL0 |= ((1 << 2)|(1<<4)); //TIMER0 PREDIVIDE =1 (system clock)|TIMER1 PREDIVIDE =1 (system clock)
	//audio I2S
//	LPC_SC->PCLKSEL1 |= 0<<22;
	//I2C	(battery monitor
//	LPC_SC->PCLKSEL1 |= 0<<6;					//PCLK_I2C(bit 6,7)=CCLK/4=100MHz/4. Not reliable if /1.
	//SSP
//	LPC_SC->PCLKSEL1 |= 1 << 10; //100MHz/2= 50MHz. SSP0 clock (CCLK/4 by RESET)
	//UART
//	LPC_SC->PCLKSEL0 |=  3<< 8; 					//BAUD 3=115, 0=230, 2=460, 1=921. 1=/1, 0=/4, 2=/2, 3=/8
}
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Initialise for IR capture.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void startCaptureIR(void) {
	IRAddress = CaptureFirst;
	LPC_SC->PCLKSEL0 &= ~((3 << 2)|(3<<4)); //CLEAR PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= ((1 << 2)|(1<<4)); //TIMER0 PREDIVIDE =1 (system clock)|TIMER1 PREDIVIDE =1 (system clock)

//	LPC_SC->PCLKSEL0 |= 1 << 2; //TIMER0 PREDIVIDE =1 (system clock)
	LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.
	LPC_PINCON->PINSEL3 |= (3 << 20); //set P1.26 as capture 0.0
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt.
	LPC_TIM0->CCR |= (1 << 1) | (1 << 2); //capture  falling with interrupt
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->TCR = 1 | 0 << 1; //enable timer0 (start timer0)

	NVIC->ISER[0] = 1 << 1; //enable TIMER0 interrupt.

	//_EnableIRQ(TIMER0_IRQn); // enable interrupt
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Initialise for IR replay.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void startPlayIR(void) {


//	compress(); //look at IR DATA, HEADER first, set up first match

//process header


	IRAddress = 0; ///< Count IR pulses during capture/replay
	IRData = Buffer[IRAddress];
	//if(0==IRData)return;
	PulseWidth = (IRData >> 16) & 0x7FF;//SET Pulse Width from 50(0.5us) to 2047(20.47us)

	Period = (IRData >> 4) & 0xFFF; //SET Period maximum 4095 (40.95us, 24.42KHz) 30KHz-48KHz,455KHz,typical 38KHz(26.32us).
	if((Period!=0)&& (PulseWidth*2>Period))PulseWidth=Period/2;


	SymbolBank = IRAddress; //new header has new symbol bank.
	IRAddress = IRAddress + 1 + (IRData & 0xF); //next address, skip any symbol bank.
	Tick=0;		// used for interrupt part of space.
	MarkCount=5;

	disableInputInterrupt();
//	IRAddress = 0;
	IRTimeMatch = 10000;
	Mark = 0x10000;//must be greater than maximum mark, currently 12 bits.
	DataStep = 0;
	if(PulseWidth<20)PulseWidth=20;		//min 200ns
	if(PulseWidth>10000)PulseWidth=10000;  //max 100us

	LPC_SC->PCLKSEL0 &= ~((3 << 2)|(3<<4)); //CLEAR PREDIVIDE bits.
	LPC_SC->PCLKSEL0 |= ((1 << 2)|(1<<4)); //TIMER0 PREDIVIDE =1 (system clock)|TIMER1 PREDIVIDE =1 (system clock)

	LPC_TIM0->PR = 0; //PRESCALE reset when count=1 IR CLOCK=system clock.


	IRData = CaptureStart; //set to 0 when end of buffer or no more captured data.
	LPC_TIM0->EMR = 1 | 1 << 4; //force output to 0.		//also sets P1.28
	LPC_TIM0->TCR = 0 | 1 << 1; //disable timer0, reset timer0
	LPC_TIM0->MR0 = 1;
	LPC_TIM0->MR0 = 0; //set and clear MR0 to clear P1.28
	LPC_PINCON->PINSEL3 |= (3 << 24); //set P1.28 as timer0 match 0.0


	LPC_TIM1->PR = 0; //set IR sample clock

	NVIC->ISER[0] = 1 << 2;
	//	NVIC_EnableIRQ(TIMER1_IRQn);
	LPC_TIM0->MR0 = PulseWidth; //generate match on MR0 after PulseWidth system clocks.(3uS)
	LPC_TIM0->MCR = 1 << 1 | 1 << 2; //stop and reset timer counter (TC) and prescale counter (PC) on match.
	//	LPC_TIM1->MR0 = 1;//compress sets up first match.


	Pulse=0;
	IRTimeMatch = 1000;//save value of IRTimeMatch for next interrupt.
	LPC_TIM1->MCR = 1; //enable MR0
	LPC_TIM1->TCR = 0 | 1 << 1; //enable timer 1 (reset timer1)
	LPC_TIM1->TCR = 1 | 0 << 1; //enable timer 1 (start timer1)
	LPC_TIM1->MR0 = IRTimeMatch; 		//first interrupt = +100us, no pulse.


}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Finalise for IR replay.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void endPlayIR(void) {
	//us(1000);
	word NEXTIRMATCH;

	NEXTIRMATCH=IRTimeMatch+PulseWidth;
	if((NEXTIRMATCH-LPC_TIM1->TC)>20000000)us(10000);
	else
	{
		while(NEXTIRMATCH>LPC_TIM1->TC);	//wait for any post space added.
	}

	LPC_PINCON->PINSEL3 &= ~(3 << 24); //set P1.28 as GPIO, so force to 0 as output set to 0.
	LPC_TIM0->MCR &= ~0x07; //disable match interrupt
	LPC_TIM1->MCR &= ~0x07; //disable match interrupt
	NVIC->ICER[0] = 1 << 1 | 1 << 2; //disable TIMER0 and TIMER1 interrupts.
//	while ((LPC_TIM0->MR0 > LPC_TIM0->TC)) // wait until last pulse has finished.
//	{
//	}
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
	int minwidth = 2000; //10us is max pulse width to consider
	int b, a, i, width;

	a = Buffer[CaptureFirst] - CaptureStart;
	Buffer[CaptureFirst] = CaptureStart;

	for (i = CaptureFirst + 1; (i < IRAddress); i++) {
		Buffer[i] = Buffer[i] - a; //reduce start delay to CaptureStart/10^7.
		b = Buffer[i]; //length of capture in clock cycles.
		// now find minimum pulse width.
		width = Buffer[i] - Buffer[i - 1];
		if ((width > 0) && (width < minwidth))
			minwidth = width;
	}

	PulseWidth = minwidth / 2;
	if (PulseWidth < 50)
		PulseWidth = 50;
	if (PulseWidth > 1023)
		PulseWidth = 1023;

	Buffer[0] = ((PulseWidth & 0x7FF) << 16) | 0b1000 << 28; //HEADER Period is minimum period.


	for (i = IRAddress; i < CaptureMax; i++) {
		Buffer[i] = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Turn InfraRed LED off.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void initIR(void) {
	LPC_GPIO_IROUT FIODIR |= IROUT; //IR defined as an output.
	LPC_GPIO_IROUT FIOCLR = IROUT; //clear IR output (IR off).
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Finalise for IR capture.
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void endCaptureIR(void) {
	LPC_TIM0->CCR &= ~0x07; //disable any capture interrupt
	NVIC->ICER[0] = 1 << 1 | 1 << 2; //disable TIMER0 and TIMER1 interrupts.

}

