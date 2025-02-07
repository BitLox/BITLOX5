/*
 * flash.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: thesquid
 */




/** \file eeprom.c
  *
  * \brief Reads and writes to the AVR's EEPROM.
  *
  * This contains functions which implement non-volatile storage using the
  * AVR's EEPROM. Compared to contemporary mass storage devices, the size of
  * the storage space is not much (only 1024 bytes on the ATmega328), but it's
  * enough to fit a couple of wallets.
  *
  * This file is licensed as described by the file LICENCE.
  */

//#include "flash.h"

//#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)|| defined(NRF52840_XXAA)
//
//
//#endif

#include <Arduino.h>

#include "../common.h"
#include "../hwinterface.h"
#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
#include "DueFlashStorage_lib/DueFlashStorage.h"
#endif

#if defined(NRF52840_XXAA)
// #include <extEEPROM.h>

//extEEPROM myEEPROM(kbits_256, 1, 64, 0x50);

//#include <Bluefruit_FileIO.h>
//#define FILENAME    "/bitlox.txt"
//#define CONTENTS    "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF"
//File file(InternalFS);
#endif

#if defined(NRF52840_XXAA)
	// extEEPROM myEEPROM(kbits_256, 1, 64, 0x50);
	// byte i2cStat = myEEPROM.begin(myEEPROM.twiClock100kHz);

#endif

//#include "eink.h"
#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
DueFlashStorage dueFlashStorage;
#endif



struct Configuration {
  uint8_t b;
};

/** Size of EEPROM, in number of bytes. */
//moved to hwinterface.h

/** Write to non-volatile storage.
  * \param data A pointer to the data to be written.
  * \param address Byte offset specifying where in non-volatile storage to
  *                start writing to.
  * \param length The number of bytes to write.
  * \return See #NonVolatileReturnEnum for return values.
  * \warning Writes may be buffered; use nonVolatileFlush() to be sure that
  *          data is actually written to non-volatile storage.
  */
NonVolatileReturn nonVolatileWrite(uint8_t *data, uint32_t address, uint32_t length)
{
	Serial.begin(9600);
	Serial.println("---nonVolatileWrite-------");
//	extEEPROM myEEPROM(kbits_256, 1, 64, 0x50);
//	byte i2cStat = myEEPROM.begin(myEEPROM.twiClock100kHz);



	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}
//	eeprom_busy_wait();
	// The (void *)(int) is there because pointers on AVR are 16 bit, so
	// just doing (void *) would result in a "cast to pointer from integer
	// of different size" warning.
//	cli();

	#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
	dueFlashStorage.write(address, data, (size_t)length);
	#endif

//#if defined(NRF52840_XXAA)
//file.open(FILENAME, FILE_READ);
//;
//#endif

	#if defined(NRF52840_XXAA)
	// myEEPROM.write(address, data, (size_t)length);

	;
	#endif


//	eeprom_write_block(data, (void *)(int)address, (size_t)length);
//	sei();

	return NV_NO_ERROR;

}

/** Read from non-volatile storage.
  * \param data A pointer to the buffer which will receive the data.
  * \param address Byte offset specifying where in non-volatile storage to
  *                start reading from.
  * \param length The number of bytes to read.
  * \return See #NonVolatileReturnEnum for return values.
  */
NonVolatileReturn nonVolatileReadNoPtr(uint8_t data, uint32_t address, uint32_t length)
{
#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + (uint32_t)length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}
	data = dueFlashStorage.read(address);
#endif
#if defined(NRF52840_XXAA)
	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + (uint32_t)length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}
	// data = myEEPROM.read(address);
#endif
	return NV_NO_ERROR;

}

NonVolatileReturn nonVolatileRead(uint8_t *data, uint32_t address, uint32_t length)
{
	Serial.println("---nonVolatileRead--top-----");
	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + (uint32_t)length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}
//	uint8_t beingRead[length];
	int i;

	for(i=0; i<length; i++){
		#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
		data[i] = dueFlashStorage.read(address+i);
		#endif

		#if defined(NRF52840_XXAA)
		// data[i] = myEEPROM.read(address+i);
		#endif
		Serial.print(data[i]);
		Serial.println("  ---nonVolatileRead--loop-----");
	}

//	data = beingRead;

//	writeEink(, false, 10, 10);
	Serial.println("---nonVolatileRead--out-----");

	return NV_NO_ERROR;

}

/** Ensure that all buffered writes are committed to non-volatile storage.
  * \return See #NonVolatileReturnEnum for return values.
  */
NonVolatileReturn nonVolatileFlush(void)
{
	// Nothing to do; writes are never buffered.
	return NV_NO_ERROR;
}


