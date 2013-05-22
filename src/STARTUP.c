//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2009-10 Code Red Technologies Ltd.
//
// Microcontroller Startup code for use with Red Suite
//
// Version : 101130
//
// Software License Agreement
//
// The software is owned by Code Red Technologies and/or its suppliers, and is
// protected under applicable copyright laws.  All rights are reserved.  Any
// use in violation of the foregoing restrictions may subject the user to criminal
// sanctions under applicable laws, as well as to civil liability for the breach
// of the terms and conditions of this license.
//
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// USE OF THIS SOFTWARE FOR COMMERCIAL DEVELOPMENT AND/OR EDUCATION IS SUBJECT
// TO A CURRENT END USER LICENSE AGREEMENT (COMMERCIAL OR EDUCATIONAL) WITH
// CODE RED TECHNOLOGIES LTD.
//
//*****************************************************************************






#if defined (__cplusplus)
#ifdef __REDLIB__
#error Redlib does not support C++
#else
//*****************************************************************************
//
// The entry point for the C++ library startup
//
//*****************************************************************************
extern "C" {
	extern void __libc_init_array(void);
}
#endif
#endif

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

// Code Red - if CMSIS is being used, then SystemInit() routine
// will be called by startup code rather than in application's main()
#if defined (__USE_CMSIS)
#include "system_LPC17xx.h"
#include "lpc17xx.h"

#include "HUB.h"
#endif
EXTERNAL char STATE;
//*****************************************************************************
#if defined (__cplusplus)
extern "C" {
#endif

//*****************************************************************************
//
// Forward declaration of the default handlers. These are aliased.
// When the application defines a handler (with the same name), this will
// automatically take precedence over these weak definitions
//
//*****************************************************************************
     void ResetISR(void);
WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void MemManage_Handler(void);
WEAK void BusFault_Handler(void);
WEAK void UsageFault_Handler(void);
WEAK void SVCall_Handler(void);
WEAK void DebugMon_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);
WEAK void IntDefaultHandler(void);

//*****************************************************************************
//
// Forward declaration of the specific IRQ handlers. These are aliased
// to the IntDefaultHandler, which is a 'forever' loop. When the application
// defines a handler (with the same name), this will automatically take
// precedence over these weak definitions
//
//*****************************************************************************
void WDT_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER1_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER2_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER3_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART0_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART1_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART2_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART3_IRQHandler(void) ALIAS(IntDefaultHandler);
void PWM1_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C0_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C1_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C2_IRQHandler(void) ALIAS(IntDefaultHandler);
void SPI_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP1_IRQHandler(void) ALIAS(IntDefaultHandler);
void PLL0_IRQHandler(void) ALIAS(IntDefaultHandler);
void RTC_IRQHandler(void) ALIAS(IntDefaultHandler);
void EINT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void EINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void EINT2_IRQHandler(void) ALIAS(IntDefaultHandler);
void EINT3_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADC_IRQHandler(void) ALIAS(IntDefaultHandler);
void BOD_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB_IRQHandler(void) ALIAS(IntDefaultHandler);
void CAN_IRQHandler(void) ALIAS(IntDefaultHandler);
void DMA_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2S_IRQHandler(void) ALIAS(IntDefaultHandler);
void ENET_IRQHandler(void) ALIAS(IntDefaultHandler);
void RIT_IRQHandler(void) ALIAS(IntDefaultHandler);
void MCPWM_IRQHandler(void) ALIAS(IntDefaultHandler);
void QEI_IRQHandler(void) ALIAS(IntDefaultHandler);
void PLL1_IRQHandler(void) ALIAS(IntDefaultHandler);
void USBActivity_IRQHandler(void) ALIAS(IntDefaultHandler);
void CANActivity_IRQHandler(void) ALIAS(IntDefaultHandler);

//*****************************************************************************
//
// The entry point for the application.
// __main() is the entry point for Redlib based applications
// main() is the entry point for Newlib based applications
//
//*****************************************************************************
#if defined (__REDLIB__)
extern void __main(void);
#endif
extern int main(void);
//*****************************************************************************
//
// External declaration for the pointer to the stack top from the Linker Script
//
//*****************************************************************************
extern void _vStackTop(void);

//*****************************************************************************
#if defined (__cplusplus)
} // extern "C"
#endif
//*****************************************************************************
//
// The vector table.
// This relies on the linker script to place at correct location in memory.
//
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
	// Core Level - CM3
	&_vStackTop, // The initial stack pointer	//00
	ResetISR,								// 0x04 The reset handler
	NMI_Handler,							// 0x08 The NMI handler
	HardFault_Handler,						// 0x0C The hard fault handler
	MemManage_Handler,						// 0x10 The MPU fault handler
	BusFault_Handler,						// 0x14 The bus fault handler
	UsageFault_Handler,						// 0x18 The usage fault handler
	0,										// 0x1C Reserved
	0,										// 0x20 Reserved
	0,										// 0x24 Reserved
	0,										// 0x28 Reserved
	SVCall_Handler,							// 0x2C SVCall handler
	DebugMon_Handler,						// 0x30 Debug monitor handler
	0,										// 0x34 Reserved
	PendSV_Handler,							// 0x38 The PendSV handler
	SysTick_Handler,						// 0x3C The SysTick handler

	// Chip Level - LPC17
	WDT_IRQHandler,							// 16, 0x40 - WDT
	TIMER0_IRQHandler,						// 17, 0x44 - TIMER0
	TIMER1_IRQHandler,						// 18, 0x48 - TIMER1
	TIMER2_IRQHandler,						// 19, 0x4c - TIMER2
	TIMER3_IRQHandler,						// 20, 0x50 - TIMER3
	UART0_IRQHandler,						// 21, 0x54 - UART0
	UART1_IRQHandler,						// 22, 0x58 - UART1
	UART2_IRQHandler,						// 23, 0x5c - UART2
	UART3_IRQHandler,						// 24, 0x60 - UART3
	PWM1_IRQHandler,						// 25, 0x64 - PWM1
	I2C0_IRQHandler,						// 26, 0x68 - I2C0
	I2C1_IRQHandler,						// 27, 0x6c - I2C1
	I2C2_IRQHandler,						// 28, 0x70 - I2C2
	SPI_IRQHandler,							// 29, 0x74 - SPI
	SSP0_IRQHandler,						// 30, 0x78 - SSP0
	SSP1_IRQHandler,						// 31, 0x7c - SSP1
	PLL0_IRQHandler,						// 32, 0x80 - PLL0 (Main PLL)
	RTC_IRQHandler,							// 33, 0x84 - RTC
	EINT0_IRQHandler,						// 34, 0x88 - EINT0
	EINT1_IRQHandler,						// 35, 0x8c - EINT1
	EINT2_IRQHandler,						// 36, 0x90 - EINT2
	EINT3_IRQHandler,						// 37, 0x94 - EINT3
	ADC_IRQHandler,							// 38, 0x98 - ADC
	BOD_IRQHandler,							// 39, 0x9c - BOD
	USB_IRQHandler,							// 40, 0xA0 - USB
	CAN_IRQHandler,							// 41, 0xa4 - CAN
	DMA_IRQHandler,							// 42, 0xa8 - GP DMA
	I2S_IRQHandler,							// 43, 0xac - I2S
	ENET_IRQHandler,						// 44, 0xb0 - Ethernet
	RIT_IRQHandler,							// 45, 0xb4 - RITINT
	MCPWM_IRQHandler,						// 46, 0xb8 - Motor Control PWM
	QEI_IRQHandler,							// 47, 0xbc - Quadrature Encoder
	PLL1_IRQHandler,						// 48, 0xc0 - PLL1 (USB PLL)
	USBActivity_IRQHandler,					// 49, 0xc4 - USB Activity interrupt to wakeup
	CANActivity_IRQHandler, 				// 50, 0xc8 - CAN Activity interrupt to wakeup
	0x59415751,								// YAWQ file name reverse order first 4 bytes.(8.3 format)
	0x2020204F,								// 1  O
	Version,								//    1_E 5F=_, 45=E		//gives QWAYO  1_E

};




//*****************************************************************************
// Functions to carry out the initialization of RW and BSS data sections. These
// are written as separate functions rather than being inlined within the
// ResetISR() function in order to cope with MCUs with multiple banks of
// memory.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))

void data_init(unsigned int romstart, unsigned int start, unsigned int len) {
	unsigned int *pulDest = (unsigned int*) start;
	unsigned int *pulSrc = (unsigned int*) romstart;
	unsigned int loop;
	for (loop = 0; loop < len; loop = loop + 4)
		*pulDest++ = *pulSrc++;
}

__attribute__ ((section(".after_vectors")))
void bss_init(unsigned int start, unsigned int len) {
	unsigned int *pulDest = (unsigned int*) start;
	unsigned int loop;
	for (loop = 0; loop < len; loop = loop + 4)
		*pulDest++ = 0;
}

#ifndef USE_OLD_STYLE_DATA_BSS_INIT
//*****************************************************************************
// The following symbols are constructs generated by the linker, indicating
// the location of various points in the "Global Section Table". This table is
// created by the linker via the Code Red managed linker script mechanism. It
// contains the load address, execution address and length of each RW data
// section and the execution and length of each BSS (zero initialized) section.
//*****************************************************************************
extern unsigned int __data_section_table;
extern unsigned int __data_section_table_end;
extern unsigned int __bss_section_table;
extern unsigned int __bss_section_table_end;
#else
//*****************************************************************************
// The following symbols are constructs generated by the linker, indicating
// the load address, execution address and length of the RW data section and
// the execution and length of the BSS (zero initialized) section.
// Note that these symbols are not normally used by the managed linker script
// mechanism in Red Suite/LPCXpresso 3.6 (Windows) and LPCXpresso 3.8 (Linux).
// They are provide here simply so this startup code can be used with earlier
// versions of Red Suite which do not support the more advanced managed linker
// script mechanism introduced in the above version. To enable their use,
// define "USE_OLD_STYLE_DATA_BSS_INIT".
//*****************************************************************************
extern unsigned int _etext;
extern unsigned int _data;
extern unsigned int _edata;
extern unsigned int _bss;
extern unsigned int _ebss;
#endif


//*****************************************************************************
// Reset entry point for your code.
// Sets up a simple runtime environment and initializes the C/C++
// library.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void
ResetISR(void) {
//About 0.8ms to get here from power up at 3.3V
//About 2.6ms from power on to here.
	NVIC->ICER[0]=0xFFFFFFFF;		//disable all interrupts.
	NVIC->ICER[1]=0xFFFFFFFF;

	LPC_GPIO_OFF FIOCLR = OFF; 			//SD(shutdown) =OFF button set low to keep on, set high to turn off.
	LPC_GPIO_OFF FIODIR |= OFF; 		//SD(shutdown) =OFF. //1K pull-down prevents turning on during power up. (3.3mA is OK)
										//set GPIO0_11 to turn off device.

	LPC_GPIO_IROUT FIOCLR = IROUT; 		//clear IR output (IR off).
	LPC_GPIO_IROUT FIODIR |= IROUT; 	//IR defined as an output.


	LPC_GPIO_BTRESET FIOSET = BTRESET;		//reset=high=run.
	LPC_GPIO_BTRESET FIODIR |= BTRESET;	//

	 if(0x1&LPC_SC->RSID)
		{STATE='P';	//Power up.
#if release==1
	LPC_GPIO_BTRESET FIOCLR = BTRESET;		//on release, clear reset for reset pulse.
#endif
		}
	else if(0x2&LPC_SC->RSID) STATE='I';		//RESET	BUTTON or external voltage monitor reset.
	else if(0x4&LPC_SC->RSID) STATE='J';		//WATCHDOG reset
	else if(0x8&LPC_SC->RSID) STATE='K';		//BROWNOUT reset (should never see this.)

	LPC_SC->RSID=0;								//Clear reset state.

    LPC_GPIO_BTFACTORY FIOSET = BTFACTORY;			//set factory default to high, to start factory default sequence.
    LPC_GPIO_BTFACTORY FIODIR |= BTFACTORY;

#ifndef USE_OLD_STYLE_DATA_BSS_INIT
    //
    // Copy the data sections from flash to SRAM.
    //
	unsigned int LoadAddr, ExeAddr, SectionLen;
	unsigned int *SectionTableAddr;

	// Load base address of Global Section Table
	SectionTableAddr = &__data_section_table;

    // Copy the data sections from flash to SRAM.
//while loop takes 17.9ms
	while (SectionTableAddr < &__data_section_table_end) {
		LoadAddr = *SectionTableAddr++;
		ExeAddr = *SectionTableAddr++;
		SectionLen = *SectionTableAddr++;
		data_init(LoadAddr, ExeAddr, SectionLen);		//copy data for each block.
	}

	// At this point, SectionTableAddr = &__bss_section_table;
	// Zero fill the bss segment
//while loop takes 1.9ms
	while (SectionTableAddr < &__bss_section_table_end) {
		ExeAddr = *SectionTableAddr++;
		SectionLen = *SectionTableAddr++;
		bss_init(ExeAddr, SectionLen);		//zero data for each block.
	}

#else
	// Use Old Style Data and BSS section initialization.
	// This will only initialize a single RAM bank.
	unsigned int * LoadAddr, *ExeAddr, *EndAddr, SectionLen;

    // Copy the data segment from flash to SRAM.
	LoadAddr = &_etext;
	ExeAddr = &_data;
	EndAddr = &_edata;
	SectionLen = (void*)EndAddr - (void*)ExeAddr;
	data_init((unsigned int)LoadAddr, (unsigned int)ExeAddr, SectionLen);
	// Zero fill the bss segment
	ExeAddr = &_bss;
	EndAddr = &_ebss;
	SectionLen = (void*)EndAddr - (void*)ExeAddr;
	bss_init ((unsigned int)ExeAddr, SectionLen);
#endif

#ifdef __USE_CMSIS
//	SystemInit();
#endif

#if defined (__cplusplus)
	//
	// Call C++ library initialisation
	//
	__libc_init_array();
#endif

#if defined (__REDLIB__)
	// Call the Redlib library, which in turn calls main()
	__main() ;
#else
	main();
#endif

	//
	// main() shouldn't return, but if it does, we'll just enter an infinite loop
	//
	while (1) {
		 NVIC_SystemReset();
		;
	}
}

//*****************************************************************************
// Default exception handlers. Override the ones here by defining your own
// handler routines in your application code.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void NMI_Handler(void)
{
    while(1)
   {
  //  	LED2YELLOW();
    	NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void HardFault_Handler(void)
{
    while(1)
   {
	LED2YELLOW();
   	 NVIC_SystemReset();
    }

}
__attribute__ ((section(".after_vectors")))
void MemManage_Handler(void)
{
    while(1)
   {
//	LED2YELLOW();
    	 NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void BusFault_Handler(void)
{
   while(1)
   {
	//LED2YELLOW();
    	 NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void UsageFault_Handler(void)
{
   while(1)
   {
//	LED2YELLOW();
    	 NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void SVCall_Handler(void)
{
   while(1)
   {
//	LED2YELLOW();
    	 NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void DebugMon_Handler(void)
{
   while(1)
   {
//		LED2YELLOW();
	      	 NVIC_SystemReset();
    }
}
__attribute__ ((section(".after_vectors")))
void PendSV_Handler(void)
{
    while(1)
    {
  //  	LED2YELLOW();
     	 NVIC_SystemReset();
   }
}
__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void)
{
    while(1)
    {
   // 	LED2YELLOW();
    	   	 NVIC_SystemReset();
   }
}

//*****************************************************************************
//
// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.
//
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void IntDefaultHandler(void)
{
    while(1)
    {
  //  	LED2YELLOW();

    	 NVIC_SystemReset();
   }
}
