///@name        	Flash read/write routines.
///@author     		Duncan Irvine
///@version     	test
///@copyright  		Possum UK 23 April 2012

//Defines

//Includes
#include "HUB.h"
#include "lpc17xx_pinsel.h"


//Public variables
PUBLIC int FlashAddress; ///< The address used for flash read and flash write.

//Local variables
EXTERNAL int Buffer[]; ///< Used for Audio and IR store and replay.

//External variables

//Local functions
PRIVATE int readStatus(void);
PRIVATE void writeBlock(int);

//External functions

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Erases the whole of the flash memory chip.
///@param void
///@return void
///@par Time
/// Chip Erase takes typically 3s, maximum 10s.
/////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void eraseFlash(void)
{
	int a;
	while (1 & readStatus()); //wait for not busy.
	//first command is write enable
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeSSP0Byte(0x06); //write enable for erase command.
	a = readSSP0Byte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	//	a = readStatus();
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeSSP0Byte(0x60); //chip erase
	a = readSSP0Byte();

	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable

	while (1 & readStatus());//wait for not busy.
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Write first half (4096 words) of Buffer[] to flash memory little endian 16384 bytes.
///@param void
///@return void
///
///@par Modifies
/// FlashAddress=FlashAddress+4096
/// Buffer[]: writes data to flash using FlashAddress from first half of Buffer[].
///@par Time
/// 55ms for this. Worst case is around 210ms (64ms*3(write)+16ms(data comms))
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void writeFlashLow(void)
{
	int i = 0;
	for (i = 0; i < 64; i++) //64 blocks of 64 words from 0 to 4095 words.
		writeBlock(i); //write 64 words
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Write last half (4096 words) of Buffer[] to flash memory little endian 16384 bytes.
///@param void
///@return void
///
///@par Modifies
/// FlashAddress=FlashAddress+4096.
/// Buffer[]: writes data to flash using FlashAddress from last half of Buffer[].
///@par Time
/// 55ms for this. Worst case is around 210ms (64ms*3(write)+16ms(data comms))
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void writeFlashHigh(void) {
	int i = 0;
	for (i = 64; i < 128; i++) //64 blocks of 64 words from 4096 to 8191 words.
		writeBlock(i); //write 64 words

}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read 16384 bytes from Flash from address FlashAddress, little endian, to last 4096 words of Buffer[].
///@param void
///@return void
///
///@par Modifies
/// FlashAddress=FlashAddress+4096.
/// Buffer[] top half is written from flash memory pointed to by FlashAddress
///@par Time
/// 16ms(data comms) to read 16KB or 4096 words.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void readFlashHigh(void) {
	while (1 & readStatus())
		; //wait for not busy
	//set up SPI for flash.
	char a, b, c, d;
	int i;
	char address0, address8, address16;
	address0 = 0xFF & FlashAddress;
	address8 = 0xFF & (FlashAddress >> 8);
	address16 = 0xFF & (FlashAddress >> 16);
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeSSP0Byte(0x03); //read command
	a = readSSP0Byte();
	writeSSP0Byte(address16); //address 16-23
	a = readSSP0Byte();
	writeSSP0Byte(address8); //address 8-15
	a = readSSP0Byte();
	writeSSP0Byte(address0); //address 0-7
	a = readSSP0Byte();
	for (i = 4096; i < 8192; i++) {
		writeSSP0Byte(0); //data   little-endian
		a = readSSP0Byte();
		writeSSP0Byte(0); //data
		b = readSSP0Byte();
		writeSSP0Byte(0); //data
		c = readSSP0Byte();
		writeSSP0Byte(0); //data
		d = readSSP0Byte();
		Buffer[i] = a + (b << 8) + (c << 16) + (d << 24); //merge 4 bytes into word.
	}
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	FlashAddress = FlashAddress + 4096;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read 16384 bytes from Flash at address FlashAddress, little endian, to first 4096 words of Buffer[].
///@param void
///@return void
///
///@par Modifies
/// FlashAddress=FlashAddress+4096.
/// Buffer[] first half is written from flash memory pointed to by FlashAddress
///@par Time
/// 16ms(data comms) to read 16KB or 4096 words.
////////////////////////////////////////////////////////////////////////////////////////////////
PUBLIC void readFlashLow(void) {
	while (1 & readStatus())
		; //wait for not busy
	//set up SPI for flash.
	char a, b, c, d;
	int i;
	char address0, address8, address16;
	address0 = 0xFF & FlashAddress;
	address8 = 0xFF & (FlashAddress >> 8);
	address16 = 0xFF & (FlashAddress >> 16);
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeSSP0Byte(0x03); //read command
	a = readSSP0Byte();
	writeSSP0Byte(address16); //address 16-23
	a = readSSP0Byte();
	writeSSP0Byte(address8); //address 8-15
	a = readSSP0Byte();
	writeSSP0Byte(address0); //address 0-7
	a = readSSP0Byte();
	for (i = 0; i < 4096; i++) {
		writeSSP0Byte(0); //data   little-endian
		a = readSSP0Byte();
		writeSSP0Byte(0); //data
		b = readSSP0Byte();
		writeSSP0Byte(0); //data
		c = readSSP0Byte();
		writeSSP0Byte(0); //data
		d = readSSP0Byte();
		Buffer[i] = a + (b << 8) + (c << 16) + (d << 24); //merge 4 bytes into word.
	}
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	FlashAddress = FlashAddress + 4096;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//PRIVATE functions:
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Write 64 words of Buffer[] little endian to 256 bytes to flash memory at address FlashAddress.
///@param int i (0-63) addresses a 64 word block within Buffer[] to write to flash using page mode (256 bytes)
///@return void
///
///@par Modifies
/// FlashAddress=FlashAddress+256.
/// Buffer[] first half is written from flash memory pointed to by FlashAddress
///@par Time
/// 0.25ms(data comms) to read 256B or 64 words, 0.7ms typical to program page, maximum 3ms.
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE void writeBlock(int i) {
	char a;
	char b, c, d, e;
	int j, k, p;
	k = i * 64;
	char address0, address8, address16;
	while (1 & readStatus())
		; //wait for not busy (ready).
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash CS enable
	writeSSP0Byte(0x06); //write enable.
	a = readSSP0Byte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash CS disable
	address0 = 0xFF & FlashAddress;
	address8 = 0xFF & (FlashAddress >> 8);
	address16 = 0xFF & (FlashAddress >> 16);
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash  CS enable
	writeSSP0Byte(0x02); //page write
	a = readSSP0Byte();
	writeSSP0Byte(address16); //address 16-23
	a = readSSP0Byte();
	writeSSP0Byte(address8); //address 8-15
	a = readSSP0Byte();
	writeSSP0Byte(address0); //address 0-7
	a = readSSP0Byte();

	for (j = 0; j < 64; j++) {
		p = Buffer[j + k]; //64i+j
		b = 0xFF & p;
		c = 0xFF & (p >> 8);
		d = 0xFF & (p >> 16);
		e = 0xFF & (p >> 24);
		writeSSP0Byte(b); //data little-endian
		a = readSSP0Byte();
		writeSSP0Byte(c); //data
		a = readSSP0Byte();
		writeSSP0Byte(d); //data
		a = readSSP0Byte();
		writeSSP0Byte(e); //data
		a = readSSP0Byte();
	}
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	FlashAddress = FlashAddress + 256; //4*64
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///@brief Read Flash memory status.
///@param void
///@return int. If bit 1=1 flash memory is busy, else ready
/////////////////////////////////////////////////////////////////////////////////////////////////
PRIVATE int readStatus(void) {
	int a, b;
	LPC_GPIO0->FIOCLR = 1 << 23; //Flash enable
	writeSSP0Byte(0x05); //read
	writeSSP0Byte(0x00); //read address
	a = readSSP0Byte();
	b = readSSP0Byte();
	LPC_GPIO0->FIOSET |= 1 << 23; //Flash disable
	return b;

}

