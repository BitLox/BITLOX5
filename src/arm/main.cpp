#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h> 
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_TinyUSB.h>
#include "main.h"
#include "ST7789.h"
#include "keypad_MPR121.h"
#include "../storage_common.h"
#include "../hwinterface.h"
#include "BLE.h"

// #include "../stream_comm.h"

/** PBKDF2 is used to derive encryption keys. In order to make brute-force
  * attacks more expensive, this should return a number which is as large
  * as possible, without being so large that key derivation requires an
  * excessive amount of time (> 1 s). This is a platform-dependent function
  * because key derivation speed is platform-dependent.
  *
  * In order to permit key recovery when the number of iterations is unknown,
  * this should be a power of 2. That way, an implementation can use
  * successively greater powers of 2 until the correct number of iterations is
  * found.
  * \return Number of iterations to use in PBKDF2 algorithm.
  */
uint32_t getPBKDF2Iterations(void)
{
	return 2048;
//	return 128;
}

void useWhatComms(void)
{
	int result;
	int s;
	uint8_t temp1[1];

  	writeUSB_BLE_Screen();


	result = getAndReturnInput();
	switch (result){
	case 0:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 1:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 4:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 5:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 8:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 9:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;

	case 2:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 3:
		temp1[0] = 7;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 6:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 7:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 10:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 11:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;

	default:
		s = 0;
		temp1[0] = s;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		break;
	}
}

// void useWhatCommsStealth(void)
// {
// 	char rChar;
// 	int r;
// 	int s;
// 	uint8_t temp1[1];

// 	r = rChar - '0';
// 	r = 1;
// 	rChar = '1';
// 	switch (rChar){
// 	case '1':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;
// 	case '2':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;
// 	case '3':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;
// 	case 'Y':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;
// 	case '7':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;
// 	case '8':
// 		temp1[0] = 0;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		deactivateBLE();
// 		break;


// 	case '4':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;
// 	case '5':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;
// 	case '6':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;
// 	case '9':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;
// 	case '0':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;
// 	case 'N':
// 		temp1[0] = 1;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// 		activateBLE();
// 		break;

// 	default:
// 		s = 0;
// 		temp1[0] = s;
// 		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		break;
// 	}
// }

// void useWhatCommsDuress(void)
// {
// 	int result;
// 	int s;
// 	uint8_t temp1[1];

//   writeUSB_BLE_Screen();

// 	result = getAndReturnInput();
// 	switch (result){
// 	case 1:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;
// 	case 2:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;
// 	case 3:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;
// 	case 0:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;
// 	case 7:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;
// 	case 8:
// //		temp1[0] = 0;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// //		deactivateBLE();
// 		break;


// 	case 4:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;
// 	case 5:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;
// 	case 6:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;
// 	case 9:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;
// 	case 10:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;
// 	case 11:
// //		temp1[0] = 1;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeBLE_Screen();
// //		activateBLE();
// 		break;

// 	default:
// //		s = 0;
// //		temp1[0] = s;
// //		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
// 		writeUSB_Screen();
// 		break;
// 	}
// showReady();
// }

void showReady(void)
{
	// writeSplashScreen();
  tftBlackScreen();
  char p[] = "BITLOX5";
  drawtext(p, ST77XX_WHITE, 4, 0, 0);
}

void writeUSB_BLE_Screen(void){
  tftBlackScreen();
  char p[] = "USB / BLE";
  drawtext(p, ST77XX_RED, 3, 0, 0);
}

void writeUSB_Screen(void){
  tftBlackScreen();
  char p[] = "USB";
  drawtext(p, ST77XX_RED, 3, 0, 0);
}

void writeBLE_Screen(void){
  tftBlackScreen();
  char p[] = "BLE";
  drawtext(p, ST77XX_RED, 3, 0, 0);
}


void setup() {
  Serial.begin(9600);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize ST7789
  initDisplay();
  // large block of text
  tftBlackScreen();
  char q[] = "BITLOX5";
  drawtext(q, ST77XX_RED, 3, 0, 0);
  delay(3000);

  tftBlackScreen();
  char p[] = "INPUT PIN:";
  drawtext(p, ST77XX_RED, 3, 0, 0);
  delay(1000);

  initKeypad();

  useWhatComms();

//   uint8_t comms[1];
//   nonVolatileRead(comms, DEVICE_COMMS_SET_ADDRESS, 1);
//   Serial.println(comms[0]);

// routine for checking PIN, exits if PIN correct
  
//if PIN correct, initialize BLE

// Show ready screen

}

void loop() {
	// processPacket();

  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(100);                       // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  // delay(100);                       // wait for a second
}

