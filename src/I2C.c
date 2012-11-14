///@name        	I2C communications.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines I2C status
#define AA 1<<2
#define SI 1<<3
#define STO 1<<4
#define STA 1<<5
#define I2EN 1<<6
#define I2CSTATE_IDLE 0
#define I2CSTATE_PENDING 1
#define I2CSTATE_SLA_NACK 2
#define I2CSTATE_ACK 3
#define I2CSTATE_NACK 4
#define I2CSTATE_ARB_LOSS 5
#define I2CSTATE_END 6




//Includes
#include "HUB.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2s.h"

//Public variables
PUBLIC  byte I2CSlaveBuffer[32];
//Private variables
PRIVATE volatile byte I2CSlaveTempBuffer[32];
PRIVATE volatile word I2CMasterState = I2CSTATE_IDLE;
PRIVATE volatile word I2CSlaveState = I2CSTATE_IDLE;

PRIVATE volatile byte I2CMasterBuffer[32];


PRIVATE volatile word I2CReadLength;
PRIVATE volatile word I2CWriteLength;

PRIVATE volatile word RdIndex = 0;
PRIVATE volatile word WrIndex = 0;
PRIVATE word COUNT=0;

//External variables

//Private functions
//public functions
PUBLIC int I2CBATTERY(void);

//PRIVATE void I2CSTART(void);

PRIVATE void I2CGO(void);



//External functions

//public functions
PUBLIC void I2CINIT(void);
PUBLIC void I2CREAD(void);
PUBLIC void I2CSHUTDOWN(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief initialise I2C
///@param void
///@return void
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CINIT(void)
{
//	word	i,j,k,l,q;
//	char a,b;
	LPC_SC->PCONP |=1<<19;						// bit 19. enable I2C1
	LPC_SC->PCLKSEL1 |= 0<<6;					//PCLK_I2C(bit 6,7)=CCLK/4=100MHz/4. Not reliable if /1.
	LPC_PINCON->PINSEL1 |=(3<<6 |3<<8);			//I2C1 P0.19(SDA1), P0.20(SCL1)
	LPC_PINCON->PINMODE1&=~(3<<6|3<<8);						//0<<6 and 0<<8 //enable pull ups.
	LPC_PINCON->PINMODE1|=(0<<6|0<<8);						//0<<6 and 0<<8




	LPC_I2C1->I2SCLH=0x40;						//duty cycle High
	LPC_I2C1->I2SCLL=0x40;						//Duty cycle low.


//Enable I2C
	LPC_I2C1->I2CONCLR=AA|SI|STO|STA;					//Clear SI|STA
	NVIC->ISER[0]=1<<11;							//I2C1 interrupt set.
//	NVIC_EnableIRQ(I2C_IRQn);

////////////////////////////////////////

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief get regs 0x0A-0x11 I2C
///@param void
///@return void
///
/// Descriptions:	The routine to complete a I2C transaction
///					from start to stop. All the intermediate
///					steps are handled in the interrupt handler.
///
///					The read length, write length and I2C master buffer
///					need to be filled.
///
/// battery monitor:

///I2CSlaveBuffer[0]	Temperature MSB
///I2CSlaveBuffer[1]	Temperature LSB
///I2CSlaveBuffer[2]	Voltage MSB
///I2CSlaveBuffer[3]	Voltage LSB
///I2CSlaveBuffer[4]	Current MSB
///I2CSlaveBuffer[5]	Current LSB
///I2CSlaveBuffer[6]	Acc current MSB
///I2CSlaveBuffer[7]	Acc current LSB
///I2CSlaveBuffer[7]	Status.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CREAD(void)
{
 //char a;
 //word i;


	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0A;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[0]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0B;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[1]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0C;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[2]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0D;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[3]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0E;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[4]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0F;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[5]=I2CSlaveTempBuffer[0];
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x10;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[6]=I2CSlaveTempBuffer[0];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x11;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[7]=I2CSlaveTempBuffer[0];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[8]=I2CSlaveTempBuffer[0];
//	I2CSlaveBuffer[0]=I2CSlaveBuffer[9];




	/*
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0A;
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=8;
	I2CWriteLength=2;
	I2CGO();
//	if ((I2CSlaveBuffer[4]==0) && (I2CSlaveBuffer[5]==0))
//	{I2CSlaveBuffer[4]=L4;
//	 I2CSlaveBuffer[5]=L5;
//	}
//	else
//	{
//		L4=I2CSlaveBuffer[4];
//		L5=I2CSlaveBuffer[5];
//	}


	I2CSlaveBuffer[4]=~I2CSlaveBuffer[4];
	I2CSlaveBuffer[5]=~I2CSlaveBuffer[5];
	I2CSlaveBuffer[6]=~I2CSlaveBuffer[6];
	I2CSlaveBuffer[7]=~I2CSlaveBuffer[7];
	//a=I2CSlaveBuffer[0];
	 */
}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read battery voltage I2C DS2745
///@param void
///@return int battery voltage in mV
///
/// Descriptions:	The routine to complete a I2C transaction
///					from start to stop. All the intermediate
///					steps are handled in the interrupt handler.

/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int I2CBATTERY(void)
{
 char a;
// word i;

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0C;
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();

	a=I2CSlaveBuffer[0];
	return a;
}






/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set full charge state
///@param void
///@return void
///
///Descriptions:	The charge accumulate is set to 0xFFFF
///					It is capped at 0xFFFF, so any further charge does not accumulate.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CFullCharge(void)
{



	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;
	I2CMasterBuffer[2]=0x0;
	I2CReadLength=0;
	I2CWriteLength=3;
	I2CGO();



	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x11;
	I2CMasterBuffer[2]=0x66;
	I2CReadLength=0;
	I2CWriteLength=3;
	I2CGO();



	//////////////////////////////////////
	// writing to address 0x11 seems to cause current to read 0 for a few seconds.
	//unclear why.
	 I2CMasterBuffer[0]=0x90;
		I2CMasterBuffer[1]=0x10;
		I2CMasterBuffer[2]=0x11;
		I2CReadLength=0;
		I2CWriteLength=3;
		I2CGO();



	}



/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set shutdown, also set PIO to high (open drain.)
///@param void
///@return void
///
///Descriptions:	The charge accumulate is set to 0xFFFF
///					It is capped at 0xFFFF, so any further charge does not accumulate.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CSHUTDOWN(void)
{



	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;
	I2CMasterBuffer[2]=0x38;
	I2CReadLength=0;
	I2CWriteLength=3;
	I2CGO();




	}








/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief execute an I2C transfer
///@param void
///@return void
///
///
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void I2CGO(void)
{

	 LPC_I2C1->I2CONSET=I2EN;					//bit 6=enable I2C
	  I2CMasterState = I2CSTATE_IDLE;
	  RdIndex = 0;
	  WrIndex = 0;

		LPC_I2C1->I2CONCLR=SI;					//Clear SI
		LPC_I2C1->I2CONSET=STA;					//STA = START




		  // wait until the state is a terminal state //
	  while ((I2CMasterState !=I2CSTATE_END)&&(I2CMasterState !=I2CSTATE_SLA_NACK))

	  {

	  }
		  ;

}









///////////////////////////////////////////////////////////////////////////////
///@brief I2C_IRQHandler
///Descriptions:		I2C interrupt handler, deal with master mode only.
///@param void
///@return void
///////////////////////////////////////////////////////////////////////////////////
void I2C1_IRQHandler(void)
{
	byte StatValue;
	char a;

	// this handler deals with master read and master write only //
	StatValue = LPC_I2C1->I2STAT;

	switch (COUNT)
	{
	case 0:
		a=0;
		break;
	case 1:
		a=1;
		break;
	case 2:
		a=2;
		break;
	case 3:
		a=3;
		break;
	case 4:
		a=4;
		break;
	case 5:
		a=5;
		break;
	case 6:
		a=6;
		break;
	case 7:
		a=7;

	break;
	}

	COUNT++;
	switch ( StatValue )
	{
	case 0x08:
		//
		// A START condition has been transmitted.
		// We now send the slave address and initialize
		// the write buffer
		// (we always start with a write after START+SLA)
		 //
		WrIndex = 0;
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		LPC_I2C1->I2CONCLR = (SI | STA);
		I2CMasterState = I2CSTATE_PENDING;
		break;

	case 0x10:
		//
		// A repeated START condition has been transmitted.
		// Now a second, read, transaction follows so we
		// initialize the read buffer.
		 //
		RdIndex = 0;
		// Send SLA with R bit set, */
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		LPC_I2C1->I2CONCLR = SI | STA;
	break;

	case 0x18:
		//
		// SLA+W has been transmitted; ACK has been received.
		// We now start writing bytes.
		 //
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		LPC_I2C1->I2CONCLR = SI;
		break;

	case 0x20:
		//
		// SLA+W has been transmitted; NOT ACK has been received.
		// Send a stop condition to terminate the transaction
		// and signal I2CEngine the transaction is aborted.
		//
		LPC_I2C1->I2CONSET = STO;
		LPC_I2C1->I2CONCLR = SI;
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x28:
		//
		// Data in I2DAT has been transmitted; ACK has been received.
		// Continue sending more bytes as long as there are bytes to send
		// and after this check if a read transaction should follow.
		//
		if ( WrIndex < I2CWriteLength )
		{
			// Keep writing as long as bytes avail
			LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		}

		else
		{
			if ( I2CReadLength != 0 )
			{
				// Send a Repeated START to initialize a read transaction /
				// (handled in state 0x10)
				LPC_I2C1->I2CONSET = STA;	// Set Repeated-start flag
			}
			else
			{
				I2CMasterState = I2CSTATE_END; //was ACK.
				LPC_I2C1->I2CONSET = STO;      // Set Stop flag
			}
		}
		LPC_I2C1->I2CONCLR = SI;
		break;

	case 0x30:
		//
		// Data byte in I2DAT has been transmitted; NOT ACK has been received
		// Send a STOP condition to terminate the transaction and inform the
		// I2CEngine that the transaction failed.
		 //
		LPC_I2C1->I2CONSET =STO;
		LPC_I2C1->I2CONCLR = SI;
		I2CMasterState = I2CSTATE_NACK;
		break;

	case 0x38:
		//
		// Arbitration loss in SLA+R/W or Data bytes.
		// This is a fatal condition, the transaction did not complete due
		// to external reasons (e.g. hardware system failure).
		// Inform the I2CEngine of this and cancel the transaction
		// (this is automatically done by the I2C hardware)
		 //
		I2CMasterState = I2CSTATE_ARB_LOSS;
		LPC_I2C1->I2CONCLR = SI;
		break;

	case 0x40:
		//
		// SLA+R has been transmitted; ACK has been received.
		// Initialize a read.
		// Since a NOT ACK is sent after reading the last byte,
		// we need to prepare a NOT ACK in case we only read 1 byte.
		 //
		if ( I2CReadLength == 1 )
		{
			// last (and only) byte: send a NACK after data is received */
			LPC_I2C1->I2CONCLR = AA;
		}
		else
		{
			// more bytes to follow: send an ACK after data is received */
			LPC_I2C1->I2CONSET = AA;
		}
		LPC_I2C1->I2CONCLR = SI;
		break;

	case 0x48:
		//
		// SLA+R has been transmitted; NOT ACK has been received.
		// Send a stop condition to terminate the transaction
		// and signal I2CEngine the transaction is aborted.
		 //
		LPC_I2C1->I2CONSET = STO;
		LPC_I2C1->I2CONCLR = SI;
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x50:
		//
		// Data byte has been received; ACK has been returned.
		// Read the byte and check for more bytes to read.
		// Send a NOT ACK after the last byte is received
		 //
		I2CSlaveTempBuffer[RdIndex++] = LPC_I2C1->I2DAT;
		if ( RdIndex < (I2CReadLength-1) )
		{
			// lmore bytes to follow: send an ACK after data is received */
			LPC_I2C1->I2CONSET = AA;
		}
		else
		{
			// last byte: send a NACK after data is received */
			LPC_I2C1->I2CONCLR = AA;


		}
		LPC_I2C1->I2CONCLR = SI;
		break;

	case 0x58:
		//
		 // Data byte has been received; NOT ACK has been returned.
		 // This is the last byte to read.
		 //Generate a STOP condition and flag the I2CEngine that the
		 //transaction is finished.
		 //
		I2CSlaveTempBuffer[RdIndex++] = LPC_I2C1->I2DAT;
		I2CMasterState = I2CSTATE_END;
		LPC_I2C1->I2CONSET = STO;	// Set Stop flag
		LPC_I2C1->I2CONCLR = SI;	// Clear SI flag
		break;


	default:
		LPC_I2C1->I2CONCLR = SI;
	break;
  }
  return;
}
