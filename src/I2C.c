///@name        	I2C communications.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2s.h"

//Public variables

//Private variables

//External variables

//Private functions

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


PUBLIC void initI2C(void)
{
	int	i,j,k,l,q;
	char a;
	LPC_SC->PCONP |=1<<19;						// bit 19. enable I2C1
	LPC_SC->PCLKSEL1 |= 0<<6;					//PCLK_I2C(bit 6,7)=CCLK/4=100MHz/4. Not reliable if /1.
	LPC_PINCON->PINSEL1 |=(3<<6 |3<<8);			//I2C1 P0.19(SDA1), P0.20(SCL1)


	LPC_I2C1->I2SCLH=0x8000;						//duty cycle High
	LPC_I2C1->I2SCLL=0x8000;						//Duty cycle low.
	LPC_I2C1->I2CONSET=1<<6;					//bit 6=enable I2C

	i=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status


	LPC_I2C1->I2CONSET=1<<6|1<<5;					//bit 6=enable I2C

	while(LPC_I2C1->I2STAT!=0x08);
	a=0x55;
	LPC_I2C1->I2DAT=a;
	LPC_I2C1->I2CONCLR=1<<3;					//Clear SI

	for(q=0;q<10000;q++)
		{
		q++;
		}
	i=	LPC_I2C1->I2STAT;						//status
	j=	LPC_I2C1->I2STAT;						//status
	k=	LPC_I2C1->I2STAT;						//status
	l=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status
	j=	LPC_I2C1->I2STAT;						//status
	k=	LPC_I2C1->I2STAT;						//status
	l=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status
	j=	LPC_I2C1->I2STAT;						//status
	k=	LPC_I2C1->I2STAT;						//status
	l=	LPC_I2C1->I2STAT;						//status
	i=	LPC_I2C1->I2STAT;						//status
	j=	LPC_I2C1->I2STAT;						//status
	k=	LPC_I2C1->I2STAT;						//status
	l=	LPC_I2C1->I2STAT;						//status
}
