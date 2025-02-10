/*
 * flash.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: thesquid
 */

/*
  *  Use the I2C bus with EEPROM 24LC64
  *  Sketch:    eeprom.pde
  *
  *  Author: hkhijhe
  *  Date: 01/10/2010
  *
  *
  */
#include <Arduino.h>
#include <Wire.h> //I2C library
#include "Adafruit_EEPROM_I2C.h"
#include "flash.h"

#include "../common.h"
#include "../hwinterface.h"

Adafruit_EEPROM_I2C i2ceeprom;


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
	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}
    
	if (i2ceeprom.begin(EEPROM_ADDR)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
		Serial.println("Found I2C EEPROM");
	  } else {
		Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
		while (1) delay(10);
	  };

	i2ceeprom.write(address, data, length);

	return NV_NO_ERROR;
}

NonVolatileReturn nonVolatileRead(uint8_t *data, uint32_t address, uint32_t length)
{
	Serial.begin(9600);
	Serial.println("---nonVolatileRead--top-----");

	if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE)
		|| ((address + (uint32_t)length) > EEPROM_SIZE))
	{
		return NV_INVALID_ADDRESS;
	}




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


