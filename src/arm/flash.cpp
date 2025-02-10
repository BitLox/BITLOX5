/*
 * flash.cpp
 *
 *  Created on: Oct 11, 2014
 * Appended on: Feb 8, 2025
 *      Author: thesquid
 */
#include <Arduino.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include "flash.h"
#include "../common.h"
#include "../hwinterface.h"

using namespace Adafruit_LittleFS_Namespace;

File file(InternalFS);

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

// #define FILENAME    "1488"
#define CONTENTS "NIGGATRON 76 "

const char filename[1] = {(char)address};


// uint8_t* Addr = &address;
// char buf[16];
// buf[0] = Addr[0];
// twitter.post(buf);



	// char *FileName;

	// FileName = *(char *)(&(address));
	// FileName = &address;
	// if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE) || ((address + length) > EEPROM_SIZE))
	// {
	// 	return NV_INVALID_ADDRESS;
	// }

	// Initialize Internal File System
	InternalFS.begin();

	Serial.print("Open file to write ... ");
	Serial.print(filename[0]);

	if (file.open(filename, FILE_O_WRITE))
	{
		file.truncate(0);
		file.close();
		file.open(filename, FILE_O_WRITE);
		file.write(CONTENTS, strlen(CONTENTS));
		file.close();
	}
	else
	{
		Serial.println("Failed!");
	};

	Serial.println("Done writing");

	return NV_NO_ERROR;
}

/** Read from non-volatile storage.
  * \param data A pointer to the buffer which will receive the data.
  * \param address Byte offset specifying where in non-volatile storage to
  *                start reading from.
  * \param length The number of bytes to read.
  * \return See #NonVolatileReturnEnum for return values.
  */
NonVolatileReturn nonVolatileRead(uint8_t *data, uint32_t address, uint32_t length)
{
	// Serial.begin(9600);
	// Serial.println("---nonVolatileRead-------");


	// uint8_t buffer[length];
	// for (int i=0; i<length; i++)
	// 	buffer[i] = data[i];


	// char *FileName;

	// *FileName = *(char *)(&(address));
	// if ((address > EEPROM_SIZE) || (length > EEPROM_SIZE) || ((address + length) > EEPROM_SIZE))
	// {
	// 	return NV_INVALID_ADDRESS;
	// }

	// // Initialize Internal File System
	// InternalFS.begin();

	// Serial.print("Open file to read ... ");

	// if (file.open(FileName, FILE_O_READ))
	// {
	// 	Serial.println("OK");
	// 	file.read(data, length);
	// 	file.close();
	// }
	// else
	// {
	// 	Serial.println("Failed!");
	// };

	// Serial.println("Done reading");

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


