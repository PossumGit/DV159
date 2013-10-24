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
PUBLIC  volatile byte I2CSlaveBuffer[32];
//Private variables
PRIVATE volatile byte I2CSlaveTempBuffer[32];
PRIVATE volatile word I2CMasterState = I2CSTATE_IDLE;
PRIVATE volatile word I2CSlaveState = I2CSTATE_IDLE;

PRIVATE  byte I2CMasterBuffer[32];


PRIVATE volatile word I2CReadLength;
PRIVATE volatile word I2CWriteLength;

PRIVATE volatile word RdIndex = 0;
PRIVATE volatile word WrIndex = 0;
PRIVATE volatile word COUNT=0;
PRIVATE volatile word lastcharge=0x8000;
PRIVATE volatile int Batt=800;
PRIVATE volatile int LCharge=0x8000;

//External variables
EXTERNAL int storedcharge;
EXTERNAL int charge;
EXTERNAL int ChargeConfidence;
EXTERNAL int NIMH;
EXTERNAL int LITHIUM;
EXTERNAL char STATE;
//Private functions
//public functions
PUBLIC int I2CBATTERY(void);
PUBLIC char QUERYnewbattery(void);
PUBLIC void I2CChargeWR(int);
PUBLIC char I2CSTATUS(void);
PUBLIC void I2CNewBattery(void);
//PRIVATE void I2CSTART(void);

PRIVATE void I2CGO(void);



//External functions
EXTERNAL void	us(unsigned int);
//public functions
PUBLIC void I2CINIT(void);
PUBLIC void I2CREAD(void);
PUBLIC void I2CSHUTDOWN(void);
PUBLIC char READPIO(void);
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
	LPC_SC->PCONP |=1<<19;						// bit 19. enable I2C1
	LPC_SC->PCLKSEL1 |= 0<<6;					//PCLK_I2C(bit 6,7)=CCLK/4=100MHz/4. Not reliable if /1.
	LPC_PINCON->PINMODE1&=~(3<<6|3<<8);						//0<<6 and 0<<8 //enable pull ups.
	LPC_PINCON->PINMODE1|=(0<<6|0<<8);						//0<<6 and 0<<8
	LPC_I2C1->I2SCLH=10;						//duty cycle High
	LPC_I2C1->I2SCLL=10;						//Duty cycle low.
//Enable I2C
	LPC_I2C1->I2CONCLR=AA|SI|STO|STA;					//Clear SI|STA
//	NVIC->ISER[0]=1<<11;							//I2C1 interrupt set.
	LPC_GPIO2->FIODIR&=~1<<20;						//SCL1
	LPC_GPIO2->FIODIR&=~1<<19;						//SDA1
	LPC_GPIO2->FIOSET=1<<20;
	LPC_GPIO2->FIOCLR=1<<19;						//SDA
	us(10);										//10us+1.25us C overhead. at 4MHz
	LPC_GPIO2->FIOSET=1<<19;						//SDA	==STOP BIT.
	LPC_PINCON->PINSEL1 |=(3<<6 |3<<8);			//I2C1 P0.19(SDA1), P0.20(SCL1)ENABLE I2C pin function.
	us(10);									//10us+1.25us C overhead at 4MHz.
}







/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief get regs 0x01 I2C
///@param void
///@return reg 1=status
///
///bit 6 is 1 if power on reset, 0 if it has been reset.
///bit 3 is 0 if Li/ 1 if NiMH
///
///
///
///
///
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC char QUERYnewbattery(void)
{

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
	I2CGO();

	if(I2CSlaveTempBuffer[0]&0x40)		//battery power on, new battery.
	{
		//clear status.6
		//set battery charge state to 0x8000
		I2CMasterBuffer[0]=0x90;			//reset bit 6 POWER on RESET for battery.
		I2CMasterBuffer[1]=0x1;
		I2CMasterBuffer[2]=0x18;
		I2CReadLength=0;
		I2CWriteLength=3;
		I2CGO();
		I2CChargeWR(0x8000);		//set initial battery state to 0x8000
		STATE='R';
	}




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

///I2CSlaveBuffer[9]	Status.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CREAD(void)
{

 //word i;


	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0A;		//
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[0]=I2CSlaveTempBuffer[0];
	I2CSlaveBuffer[1]=I2CSlaveTempBuffer[1];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0C;		//
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[2]=I2CSlaveTempBuffer[0];
	I2CSlaveBuffer[3]=I2CSlaveTempBuffer[1];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x0E;		//
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[4]=I2CSlaveTempBuffer[0];
	I2CSlaveBuffer[5]=I2CSlaveTempBuffer[1];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x10;		//charge
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	I2CSlaveBuffer[6]=I2CSlaveTempBuffer[0];
	I2CSlaveBuffer[7]=I2CSlaveTempBuffer[1];

	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;		//1 for status.
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=1;
	I2CWriteLength=2;
		I2CGO();

	I2CSlaveBuffer[8]=I2CSlaveTempBuffer[0];		//status byte.

	I2CSlaveBuffer[9]=0x01 ^ ((LPC_GPIO2->FIOPIN &(1<<8))>>8);//=1 if AC powered;
	I2CSlaveBuffer[10]=0xAA;
	I2CSlaveBuffer[11]=0xAA;
	I2CSlaveBuffer[12]=0xAA;
	I2CSlaveBuffer[13]=0xAA;
	I2CSlaveBuffer[14]=0xAA;
	I2CSlaveBuffer[15]=0xAA;

	if (!(LPC_GPIO2->FIOPIN&1<<8))			//AC connected
		{
			if(0x08& I2CSlaveBuffer[8])			//1=NIMH, 0=LITHIUM
			{
					NIMH=1;
					LITHIUM=0;
					}
					else
					{NIMH=0;
					LITHIUM=1;
					}
		}

//rest of this goes wrong some of the time, therefore remove unless sorted.
	//problem is faulty data read from DC2745 after datawrite.
	if (lastcharge<0x7400)lastcharge=0x7400;
	if (lastcharge>0x9000)lastcharge=0x9000;


	charge=((I2CSlaveBuffer[6])<<8)+I2CSlaveBuffer[7];


	if (charge<0x7400)
		{charge=lastcharge;
	//	I2CChargeWR(charge);
		}
	else if (charge>0x9000)
		{charge=lastcharge;
	//	I2CChargeWR(charge);
		}
	else
	{
		lastcharge=charge;
	}






	LPC_GPIO2->FIODIR&=~(1<<8);			//ACOK


	if (LPC_GPIO2->FIOPIN &(1<<8))		//ACOK=1 means on battery mode.
		{
//This is battery mode.
	if (charge>storedcharge)		//was charging.
		//discharge use rate in monitor.
		//charge, add efficiency factor in.
	{


				//On battery and charge>stored charge
				//so has just come off charge.


				if(NIMH) charge=storedcharge+0.70*(charge-storedcharge);		//NIMH
						else
							charge=storedcharge+1.05*(charge-storedcharge);		//LI ION to force fully charged state as 0x8000
				if (charge>0x8000){
					charge=0x8000;
					I2CChargeWR(charge);
					ChargeConfidence=5;
				}
			//	I2CChargeWR(charge);
				if(0x7FF0>charge)	ChargeConfidence=3;

				storedcharge=charge;

//				I2CSlaveBuffer[6]=0xFF&(charge<<8);			//write new charge state.
//				I2CSlaveBuffer[7]=0xFF&charge;

	//			I2CChargeWR(charge);

				}

	else 	//discharging.
	{
		storedcharge=charge;
	}
		}
}






PUBLIC char I2CBAT(void)//poweruphex codes D and E
{
	I2CMasterBuffer[0]=0x90;
			I2CMasterBuffer[1]=0x0C;
			I2CMasterBuffer[2]=0x91;
			I2CReadLength=1;
			I2CWriteLength=2;
			I2CGO();
	return (I2CSlaveTempBuffer[0]);

}







/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief read battery voltage I2C DS2745
///@param void
///@return int battery voltage in mV
///
/// Descriptions:	The routine to complete a I2C transaction
///					from start to stop. All the intermediate
///					steps are handled in the interrupt handler.
///
//
//
//if new battery CHARGE=0x8000.
//
//if ACON and charge> 0x8000 and charge <0x9000 then charge=0x8000
//
//
//if ACON: GREEN, last=GREEN
//
//else if NIMH and V<3.0 || V>4.8V: LED= last;
//else if NIMH and V<3.5V YELLOW store last
//else if NIMH and V>4.0V GREEN store last
//
//
//
//if charge <0x7900 then YELLOW store last
//
//else GREEN.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC int I2CBATTERY(void) //batterystate and main during power up.
{
			//return 1=GREEN=good battery, 0=yellow=low battery.
 int c,v;
// word i;
 //
 //
	if(!(LPC_GPIO2->FIOPIN &(1<<8)))
	{	//AC on.
	Batt=1;		//ACON: GREEN, last=GREEN (GREEN=good battery)
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x10;
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	c=(I2CSlaveTempBuffer[0]<<8)| I2CSlaveTempBuffer[1];
	if (c>0x8000)		// ACON and charge> 0x8000 and charge <0x9000 then charge=0x8000
		{
		I2CChargeWR(0x8000);		//write 0x8000.
		}
		return 1;		//AC on.

	}
//measure volts.
		I2CMasterBuffer[0]=0x90;
		I2CMasterBuffer[1]=0x0C;
		I2CMasterBuffer[2]=0x91;
		I2CReadLength=2;
		I2CWriteLength=2;
		I2CGO();

		v=I2CSlaveTempBuffer[0]<<3 | I2CSlaveTempBuffer[1]>>5;
		if (v<615||v>984) v=Batt;
		else Batt=v;					//b=volts.




	if(	NIMH)
	{

//v*4.88mV(/32)
//v=615=3.0V
//v=635=3.1V
//v=656=3.2V
//v=676=3.3V
//v=697=3.4V
//v=717=3.5V
//v=738=3.6V
//v=758=3.7V
//v=789=3.8V
//v=800=3.9V
//v=820=4.0V
//v=840=4.1V
//v=861=4.2V
//v=881=4.3V
//v=902=4.4V
//v=922=4.5V
//v=984=4.8V


	if (v<717) return 0;			//<3.5V low battery
	if (v>800) return 1;			//>3.9V good battery
//check charge state:
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x10;
	I2CMasterBuffer[2]=0x91;
	I2CReadLength=2;
	I2CWriteLength=2;
	I2CGO();
	c=(I2CSlaveTempBuffer[0]<<8)| I2CSlaveTempBuffer[1];
	if (c<0x7400||c>0xA000) c=LCharge;
		else LCharge=c;
	if (c<0x79AE) return 0;
	else return 1;

	}
	else
	if (LITHIUM)
	{
		//v*4.88mV
		//v=615=3.0V
		//v=717=3.5V
		//v=758=3.7V
		//v=800=3.9V
		//v=820=4.0V
		//v=984=4.8V
	if (v<758) return 0;		//lithium less than 3.7V
	if (v>800) return 1;		//lithium > 3.9V
//check charge state:
	I2CMasterBuffer[0]=0x90;
		I2CMasterBuffer[1]=0x10;
		I2CMasterBuffer[2]=0x91;
		I2CReadLength=2;
		I2CWriteLength=2;
		I2CGO();
		c=(I2CSlaveTempBuffer[0]<<8)| I2CSlaveTempBuffer[1];
		if (c<0x7400||c>0xA000) c=LCharge;
			else LCharge=c;
		if (c<0x79AE) return 0;
		else return 1;

	}

	else
	return 1;
}














/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set full charge state
///@param void
///@return void
///
///Descriptions:	The charge accumulate is set to 0xFFFF
///					It is capped at 0xFFFF, so any further charge does not accumulate.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CChargeWR(int charge)
{


	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x10;
	I2CMasterBuffer[2]=0xFF&(charge>>8);
	I2CMasterBuffer[3]=0xFF&(charge);
	I2CReadLength=0;
	I2CWriteLength=4;

	I2CGO();


	}









/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief set shutdown, also set PIO to high (open drain.)
///@param void
///@return void
///
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void I2CNewBattery(void)
{
	I2CMasterBuffer[0]=0x90;
	I2CMasterBuffer[1]=0x1;
	I2CMasterBuffer[2]=0x18;
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



PUBLIC char I2CSTATUS(void)

{
	 char a;
	// word i;

		I2CMasterBuffer[0]=0x90;
		I2CMasterBuffer[1]=0x01;
		I2CMasterBuffer[2]=0x91;
		I2CReadLength=1;
		I2CWriteLength=2;
		I2CGO();

		a=I2CSlaveTempBuffer[0];
		return a;
}





PRIVATE void I2CGO(void)

{
	int status;
	 	 LPC_I2C1->I2CONSET=I2EN;					//bit 6=enable I2C
	 	 I2CMasterState = I2CSTATE_IDLE;
	 	 RdIndex = 0;
	 	 WrIndex = 0;
	 	 LPC_I2C1->I2CONCLR=SI;					//Clear SI
	 	 LPC_I2C1->I2CONSET=STA;					//STA = START transmit



	 	while ((I2CMasterState !=I2CSTATE_END)&&(I2CMasterState !=I2CSTATE_SLA_NACK))
	 	 {

	 		us(100);
	 		status=LPC_I2C1->I2STAT;
	 	 switch (status)
	 	 {
	 	case 0x08:			//Master transmit		Master Receive
	 			//
	 			// A START condition has been transmitted.
	 			// We now send the slave address and initialise
	 			// the write buffer
	 			// (we always start with a write after START+SLA)
	 			 //
	 			WrIndex = 0;
	 			LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];		//hit
	 			LPC_I2C1->I2CONCLR = (SI | STA);
	 			I2CMasterState = I2CSTATE_PENDING;
	 			break;

	 		case 0x10:		//Master transmit		Master Receive
	 			//
	 			// A repeated START condition has been transmitted.
	 			// Now a second, read, transaction follows so we
	 			// initialize the read buffer.
	 			 //
	 			RdIndex = 0;
	 			// Send SLA with R bit set, */
	 			LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];			//hit
	 			LPC_I2C1->I2CONCLR = SI | STA;
	 		break;

	 		case 0x18:			//Master transmit
	 			//
	 			// SLA+W has been transmitted; ACK has been received.
	 			// We now start writing bytes.
	 			 //
	 			LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];			//hit
	 			LPC_I2C1->I2CONCLR = SI;
	 			break;

	 		case 0x20:			//Master transmit
	 			//
	 			// SLA+W has been transmitted; NOT ACK has been received.
	 			// Send a stop condition to terminate the transaction
	 			// and signal I2CEngine the transaction is aborted.
	 			//
	 			LPC_I2C1->I2CONSET = STO;
	 			LPC_I2C1->I2CONCLR = SI;
	 			I2CMasterState = I2CSTATE_SLA_NACK;		//not hit.
	 			break;

	 		case 0x28:			//Master transmit
	 			//
	 			// Data in I2DAT has been transmitted; ACK has been received.
	 			// Continue sending more bytes as long as there are bytes to send
	 			// and after this check if a read transaction should follow.
	 			//
	 			if ( WrIndex < I2CWriteLength )							//hit
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

	 		case 0x30:			//Master transmit
	 			//
	 			// Data byte in I2DAT has been transmitted; NOT ACK has been received
	 			// Send a STOP condition to terminate the transaction and inform the
	 			// I2CEngine that the transaction failed.
	 			 //
	 			LPC_I2C1->I2CONSET =STO;					//not hit
	 			LPC_I2C1->I2CONCLR = SI;
	 			I2CMasterState = I2CSTATE_NACK;
	 			break;

	 		case 0x38:			//Master transmit		Master Receive
	 			//
	 			// Arbitration loss in SLA+R/W or Data bytes.
	 			// This is a fatal condition, the transaction did not complete due
	 			// to external reasons (e.g. hardware system failure).
	 			// Inform the I2CEngine of this and cancel the transaction
	 			// (this is automatically done by the I2C hardware)
	 			 //
	 			I2CMasterState = I2CSTATE_ARB_LOSS;			//not hit
	 			LPC_I2C1->I2CONCLR = SI;
	 			break;

	 		case 0x40:		//Master Receive
	 			//
	 			// SLA+R has been transmitted; ACK has been received.
	 			// Initialize a read.
	 			// Since a NOT ACK is sent after reading the last byte,
	 			// we need to prepare a NOT ACK in case we only read 1 byte.
	 			 //
	 			if ( I2CReadLength == 1 )						//hit
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

	 		case 0x48:		//Master Receive
	 			//
	 			// SLA+R has been transmitted; NOT ACK has been received.
	 			// Send a stop condition to terminate the transaction
	 			// and signal I2CEngine the transaction is aborted.
	 			 //
	 			LPC_I2C1->I2CONSET = STO;				//not hit
	 			LPC_I2C1->I2CONCLR = SI;
	 			I2CMasterState = I2CSTATE_SLA_NACK;
	 			break;

	 		case 0x50:		//Master Receive
	 			//
	 			// Data byte has been received; ACK has been returned.
	 			// Read the byte and check for more bytes to read.
	 			// Send a NOT ACK after the last byte is received
	 			 //
	 			I2CSlaveTempBuffer[RdIndex++] = LPC_I2C1->I2DAT;			//hit
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

	 		case 0x58:		//Master Receive
	 			//
	 			 // Data byte has been received; NOT ACK has been returned.
	 			 // This is the last byte to read.
	 			 //Generate a STOP condition and flag the I2CEngine that the
	 			 //transaction is finished.
	 			 //
	 			I2CSlaveTempBuffer[RdIndex++] = LPC_I2C1->I2DAT;				//hit
	 			I2CMasterState = I2CSTATE_END;
	 			LPC_I2C1->I2CONSET = STO;	// Set Stop flag
	 			LPC_I2C1->I2CONCLR = SI;	// Clear SI flag
	 			break;

	 		case	0x60:
	 		case	0x68:
	 		case	0x70:
	 		case	0x78:
	 		case	0x80:
	 		case	0x88:
	 		case	0x90:
	 		case	0x98:
	 		case	0xA0:
	 		case	0xA8:
	 		case	0xB0:
	 		case	0xB8:
	 		case	0xC0:
	 		case	0xC8:

	 			//Slave modes should never see these.
	 			LPC_I2C1->I2CONCLR = SI;
	 			LPC_I2C1->I2CONSET = STO;	// Set Stop flag
	 			return;
	 			break;





	 		case	0x00:			//bus error
	 			LPC_I2C1->I2CONCLR = SI;
	 			LPC_I2C1->I2CONSET = STO;	// Set Stop flag
	 			return;
	 			break;
	 		case	0xF8:
	 			break;
	 		default:
	 			LPC_I2C1->I2CONCLR = SI;
	 			LPC_I2C1->I2CONSET = STO;	// Set Stop flag
	 			return;
	 		break;

	 	 }


	 	 }

	 	us(1000);
		LPC_I2C1->I2CONSET = STO;	// Set Stop flag





}





/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief execute an I2C transfer
///@param void
///@return void
///
///
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void I2CGOI(void)
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
