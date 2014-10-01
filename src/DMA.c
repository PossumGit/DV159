///@name        	IR Capture and Replay
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK
///@date			26 March 2013
//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx.h"
//Public variables

//Private global variables

//External variables



//Public functions
//void DMAinit(void);
//External functions



/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
DMAinit()
{
	LPC_PCONP |=1<<29;		//Enable DMA POWER
	LPC_DMAreqSel=0;		//All select UARTS. (1 for TIMERS)
	LPC_DMACBREQ=1<<11;		//DMA burst mode on UART1 RX.

	LPC_DMACConfig=1<<0|0<<1;	//enabled|littleendian
	LPC_DMACSync=0;				//enable DMA sync
	LPC_DMACC0SrcAddr=		//Chan 0, address of UART1 read register
	LPC_DMACCxDextAddr=Buffer;	//DMA destination.
	LPC_DMACC0Control=length<<0|1<<12|1<<17|0<<18\2<<21;
	////transfer size to memory|Source burst|dest burst|source width|Destination width
}
	*/




