/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface
n
Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/
/*
 * keypad_alpha.cpp
 *
 *  Created on: Feb 5, 2015
 *      Author: thesquid
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <string.h>
#include "main.h"
#include "keypad.h"
#include "eink.h"
#include "nRF52_ePaper/due_ePaper.h"
#include "usart.h"
#include "BLE.h"
#include "../storage_common.h"
#include "../hwinterface.h"
#include "../hash.h"
#include "../sha256.h"
#include "../prandom.h"
#include "lcd_and_input.h"
#include "../stream_comm.h"
#include "../common.h"
#include "../baseconv.h"
#include "ST7789.h"

// Define capsLock
bool capsLock = false; // False = lowercase, True = uppercase

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// Character mappings for each pad (0-11) - Lowercase
const char* keyMap[12] = {
    "0^",     // Pad 0
    "1abc",   // Pad 1
    "2def",   // Pad 2
    "3ghi",   // Pad 3
    "4jkl",   // Pad 4
    "5mno",   // Pad 5
    "6pqr",   // Pad 6
    "7stu",   // Pad 7
    "8vwx",   // Pad 8
    "9yz!#$%&*()_+=/?",   // Pad 9
    "~~~~",   // Pad 10
    "@@@@"    // Pad 11
};

// Capital character mappings for each pad (0-11)
const char* keyMapCaps[12] = {
    "0^",     // Pad 0
    "1ABC",   // Pad 1
    "2DEF",   // Pad 2
    "3GHI",   // Pad 3
    "4JKL",   // Pad 4
    "5MNO",   // Pad 5
    "6PQR",   // Pad 6
    "7STU",   // Pad 7
    "8VWX",   // Pad 8
    "9YZ!#$%&*()_+=/?",   // Pad 9
    "~~~~",   // Pad 10
    "@@@@"    // Pad 11
};

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


void initKeypad(void){
    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
    if (!cap.begin(0x5A))
    {
        #ifdef DEBUG_MODE
        Serial.println("MPR121 not found, check wiring?");
        #endif
    }
    else
    {
        #ifdef DEBUG_MODE
        Serial.println("MPR121 found!");
        #endif
    }
}

void cleanI2C(){
        //  https://forums.adafruit.com/viewtopic.php?t=157070
        // Clean the I2C Bus
        // SCL is 23 on feather nRF82540
        pinMode(23, OUTPUT);
        for (byte i = 0; i < 8; i++)
        {
            // Toggle the SCL pin eight times to reset any errant commands received by the slave boards.
            digitalWrite(23, HIGH);
            delayMicroseconds(3);
            digitalWrite(23, LOW);
            delayMicroseconds(3);
        }
        pinMode(23, INPUT);
}

void getInput121()
{
    do
    {
        cleanI2C();
        // Get the currently touched pads
        currtouched = cap.touched();
        for (uint8_t i = 0; i < 12; i++)
        {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
            {
                Serial.print(i);
                Serial.println(" touched");
            }
            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
            {
                Serial.print(i);
                Serial.println(" released");
            }
        }

        // reset our state
        lasttouched = currtouched;

        // put a delay so it isn't overwhelming
        delay(100);
    } while (1);
};


char getAndReturnInputDisplay(int x, int y) {
    unsigned long pressStartTime = 0; // Time when key was first pressed
    int pressedPad = -1;             // Index of the pressed pad
    int charIndex = 0;               // Index of the current character in keyMap
    bool isPressed = false;          // Tracks if a key is currently pressed
    unsigned long lastCycleTime = 0; // Time of last character cycle
    unsigned long lastTouchTime = 0; // Time of last touch event for debouncing

    while (true) {
        cleanI2C();
        currtouched = cap.touched();

        // Check for new touches or releases
        for (uint8_t i = 0; i < 12; i++) {
            // New touch detected
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) && !isPressed) {
                unsigned long currentTime = millis();
                if (currentTime - lastTouchTime >= 50) { // 50ms debounce
                    isPressed = true;
                    pressedPad = i;
                    pressStartTime = currentTime;
                    lastTouchTime = currentTime;
                    charIndex = 0;
                    // Select the active key map
                    const char* activeMap = capsLock ? keyMapCaps[pressedPad] : keyMap[pressedPad];
                    // Display the character at x, y
                    displayChar(activeMap[charIndex], x, y, false);
                    Serial.print("Current char: ");
                    Serial.println(activeMap[charIndex]);
                }
            }
            // Release detected
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) && isPressed && pressedPad == i) {
                isPressed = false;
                lasttouched = currtouched;
                // Select the active key map
                const char* activeMap = capsLock ? keyMapCaps[pressedPad] : keyMap[pressedPad];
                // Handle pad 0 ("0^") separately
                if (pressedPad == 0 && activeMap[charIndex] == '^') {
                    capsLock = !capsLock; // Toggle caps lock
                    displayChar(' ', x, y, true); // Clear character, show caps status
                    Serial.println(capsLock ? "Caps Lock ON" : "Caps Lock OFF");
                    return '\0'; // No character output
                }
                // Return the selected character ('@' for NO, '~' for YES)
                return activeMap[charIndex];
            }
        }

        // Update lasttouched
        lasttouched = currtouched;

        // If a key is pressed, check for long press and cycle characters
        if (isPressed) {
            unsigned long currentTime = millis();
            if (currentTime - pressStartTime >= 800) {
                if (currentTime - lastCycleTime >= 800) {
                    const char* activeMap = capsLock ? keyMapCaps[pressedPad] : keyMap[pressedPad];
                    charIndex = (charIndex + 1) % strlen(activeMap);
                    lastCycleTime = currentTime;
                    // Display the character at x, y
                    displayChar(activeMap[charIndex], x, y, false);
                    Serial.print("Current char: ");
                    Serial.println(activeMap[charIndex]);
                }
            }
        }

        delay(10); // Short delay for responsiveness
    }
}


int getAndReturnInput()
{
    int result;
    int state = 1;
    do
    {
        cleanI2C();
        // Get the currently touched pads
        currtouched = cap.touched();
        for (uint8_t i = 0; i < 12; i++)
        {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" touched");
            }
            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" released");
                state = 0;
                result = i;
            }
        }

        // reset our state
        lasttouched = currtouched;

        delay(100);
    }while(state);
    return result;
}

int getAndReturnInputPlusDisplay(int x, int y)
{
    int result;
    int state = 1;
    do
    {
        cleanI2C();
        // Get the currently touched pads
        currtouched = cap.touched();
        for (uint8_t i = 0; i < 12; i++)
        {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" touched");
            }
            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" released");
                state = 0;
                result = i;
            }
        }

        // reset our state
        lasttouched = currtouched;

        delay(100);
    }while(state);
    return result;
}

// const byte DEPTH = 4;
// const byte COLS = 3; //three columns
// const byte ROWS = 4; //four rows


// #define ROW_PIN_1      22
// #define ROW_PIN_2      23
// #define ROW_PIN_3      24
// #define COL_PIN_1      18
// #define COL_PIN_2      19
// #define COL_PIN_3      20
// #define COL_PIN_4      21

//define the symbols on the buttons of the keypads
//char hexaKeys_plastic[DEPTH][COLS][ROWS] = {
//  {
//    {'~', '@', '9', '8'},
//    {'7', '6', '5', '4'},
//    {'3', '2', '1', '0'}
//  },
//  {
//    {'~', '@', 'y', 'v'},
//    {'s', 'p', 'm', 'j'},
//    {'g', 'd', 'a', '^'}
//  },
//  {
//    {'~', '@', 'z', 'w'},
//    {'t', 'q', 'n', 'k'},
//    {'h', 'e', 'b', '^'}
//  },
//  {
//    {'~', '@', 'z', 'x'},
//    {'u', 'r', 'p', 'l'},
//    {'i', 'f', 'c', '^'}
//  }
//};
//

// const char hexaKeys[DEPTH][COLS][ROWS] = {
//   {
//     {'@', '6', '0', '9'},
//     {'8', '7', '5', '4'},
//     {'3', '2', '1', '~'}
//   },
//   {
//     {'@', 'p', '^', 'y'},
//     {'v', 's', 'm', 'j'},
//     {'g', 'd', 'a', '~'}
//   },
//   {
//     {'@', 'q', '^', 'z'},
//     {'w', 't', 'n', 'k'},
//     {'h', 'e', 'b', '~'}
//   },
//   {
//     {'@', 'r', '^', 'z'},
//     {'x', 'u', 'o', 'l'},
//     {'i', 'f', 'c', '~'}
//   }
// };

// const char hexaKeysCAPS[DEPTH][COLS][ROWS] = {
//   {
//     {'@', '6', '0', '9'},
//     {'8', '7', '5', '4'},
//     {'3', '2', '1', '~'}
//   },
//   {
//     {'@', 'P', '^', 'Y'},
//     {'V', 'S', 'M', 'J'},
//     {'G', 'D', 'A', '~'}
//   },
//   {
//     {'@', 'Q', '^', 'Z'},
//     {'W', 'T', 'N', 'K'},
//     {'H', 'E', 'B', '~'}
//   },
//   {
//     {'@', 'R', '^', 'Z'},
//     {'X', 'U', 'O', 'L'},
//     {'I', 'F', 'C', '~'}
//   }
// };

// const char hexaKeysPlusSpace[DEPTH][COLS][ROWS] = {
//   {
//     {'@', '6', '0', '9'},
//     {'8', '7', '5', '4'},
//     {'3', '2', '1', '~'}
//   },
//   {
//     {'@', 'p', '^', 'y'},
//     {'v', 's', 'm', 'j'},
//     {'g', 'd', 'a', '~'}
//   },
//   {
//     {'@', 'q', '_', 'z'},
//     {'w', 't', 'n', 'k'},
//     {'h', 'e', 'b', '~'}
//   },
//   {
//     {'@', 'r', '^', 'z'},
//     {'x', 'u', 'o', 'l'},
//     {'i', 'f', 'c', '~'}
//   }
// };

// const char hexaKeysCAPSPlusSpace[DEPTH][COLS][ROWS] = {
//   {
//     {'@', '6', '0', '9'},
//     {'8', '7', '5', '4'},
//     {'3', '2', '1', '~'}
//   },
//   {
//     {'@', 'P', '^', 'Y'},
//     {'V', 'S', 'M', 'J'},
//     {'G', 'D', 'A', '~'}
//   },
//   {
//     {'@', 'Q', '_', 'Z'},
//     {'W', 'T', 'N', 'K'},
//     {'H', 'E', 'B', '~'}
//   },
//   {
//     {'@', 'R', '^', 'Z'},
//     {'X', 'U', 'O', 'L'},
//     {'I', 'F', 'C', '~'}
//   }
// };

// const int keyDelay = 50;


int signum (int x) {
  if (x < 0) return -1;
  if (x > 0) return 1;
  return 0;
}

int add (int x, int y) {
  for (int i = 0; i < abs(y); ++i) {
    if (y > 0) ++x;
    else --x;
  }
  return x;
}

int mult (int x, int y) {
  int sign = signum(x) * signum(y);
  x = abs(x);
  y = abs(y);
  int res = 0;
  for (int i = 0; i < y; ++i) {
    res = add(res, x);
  }
  return sign * res;
}

int pow (int x, int y) {
  if (y < 0) return 0;
  int res = 1;
  for (int i = 0; i < y; ++i) {
    res = mult(res, x);
  }
  return res;
}


bool doAEMValidate(bool displayAlpha)
{

	char encryption_phrase_char[17]={};
	char *encryption_phrase_char_ptr;

	uint8_t tempLang[1];
	nonVolatileRead(tempLang, DEVICE_LANG_ADDRESS, 1);

	int lang;
	lang = (int)tempLang[0];

	int zhSizer = 1;

	if (lang == 3)
	{
		zhSizer = 2;
	}


	encryption_phrase_char_ptr = &encryption_phrase_char[0];

	uint8_t encryption_phrase_array[16]={};
	uint8_t *encryption_phrase;

	encryption_phrase = &encryption_phrase_array[0];

//	memset(encryption_phrase, 0, 15);
//	strcpy(encryption_phrase, "");

	if(displayAlpha){
		buttonInterjectionNoAckSetup(ASKUSER_AEM_ENTRY_ALPHA);
	}else{
		buttonInterjectionNoAckSetup(ASKUSER_AEM_ENTRY);
	}



	encryption_phrase_char_ptr = getInputAEM(displayAlpha, false);
//	getInput(displayAlpha, false);
	int i;
	for (i = 0; i<sizeof(encryption_phrase_char)-1;i++)
	{
		encryption_phrase_array[i] = encryption_phrase_char_ptr[i];
	}

	clearDisplay();

//	writeEinkDisplay("decrypt with", false, COL_1_X, LINE_1_Y, encryption_phrase_char_ptr,false,COL_1_X, LINE_2_Y, "",false,5,50, "",false,5,70, "",false,0,0);
//	displayHexStream(encryption_phrase_array, 16);




	uint8_t ciphertext[64];
	nonVolatileRead(ciphertext, AEM_PHRASE_ADDRESS, 64);

//	writeEinkDisplay("to be decrypted", false, COL_1_X, LINE_1_Y, "",false,COL_1_X, LINE_2_Y, "",false,5,50, "",false,5,70, "",false,0,0);
//	displayHexStream(ciphertext, 64);
//

	decryptStreamSized(ciphertext, encryption_phrase_array, 64);
	int j;
	bool goodPhrase = true;
	for(j=0; j<64; j++)
	{
		if((ciphertext[j] > 0x0000 && ciphertext[j] < 0x0030) ||(ciphertext[j] > 0x0039 && ciphertext[j] < 0x0041)||(ciphertext[j] > 0x005A && ciphertext[j] < 0x0061)||ciphertext[j] > 0x007A)
		{
			goodPhrase = false;
		}
	}

	initDisplay();
	if(goodPhrase)
	{
		writeEinkNoDisplay("RECOGNITION PHRASE:",  COL_1_X, LINE_0_Y, (char *)ciphertext,COL_1_X, LINE_2_Y, "",5,50, "",5,70, "",0,0);
	}else
	{
		writeEinkNoDisplay("RECOGNITION PHRASE:",  COL_1_X, LINE_0_Y, "bad decryption phrase",COL_1_X, LINE_2_Y, "",5,50, "",5,70, "",0,0);
	}

	drawCheck(draw_check_X,draw_check_Y);

	display();

	waitForButtonPress();
//	displayHexStream(ciphertext, 64);

	return false;

}



void doAEMSet(void)
{

	bool approved;
	bool approved2;
	bool approved3;
	bool approved4;
	bool approved5;
	bool permission_denied;

	approved = false;
	approved2 = false;
	approved3 = false;
	approved4 = false;
	approved5 = false;

	if (!approved)
	{
		// Need to explicitly get permission from user.
		// The call to parseTransaction() should have logged all the outputs
		// to the user interface.
		permission_denied = buttonInterjectionNoAckSetup(ASKUSER_USE_AEM);
		if (!permission_denied)
		{
			// User approved action.
			approved = true;
		}
	} // if (!approved)
	if (approved)
	{
		uint8_t display_phrase_array[64]={};
		char display_phrase_char[65] = {};
		char *display_phrase;

		display_phrase = &display_phrase_char[0];

		memset(display_phrase, 0, 64);
		strcpy(display_phrase, "");

		permission_denied = buttonInterjectionNoAckSetup(ASKUSER_AEM_DISPLAYPHRASE);
		if (!permission_denied)
		{
			// User approved action.
			approved2 = true;
		}

		if(approved2)
		{


			buttonInterjectionNoAckSetup(ASKUSER_ALPHA_INPUT_PREFACE);


			display_phrase = getInput(true, false);
			int j;
			for (j = 0; j<sizeof(display_phrase_char)-1;j++)
			{
				display_phrase_array[j] = display_phrase[j];
			}

			clearDisplay();


			permission_denied = buttonInterjectionNoAckPlusData(ASKUSER_SHOW_DISPLAYPHRASE, display_phrase, 0);
			if (!permission_denied)
			{
				// User approved action.
				approved3 = true;
			}
			if(approved3)
			{
				char encryption_phrase_char[17]={};
				char *encryption_phrase_char_ptr;

				encryption_phrase_char_ptr = &encryption_phrase_char[0];

				uint8_t encryption_phrase_array[16]={};
				uint8_t *encryption_phrase;

				encryption_phrase = &encryption_phrase_array[0];


				permission_denied = buttonInterjectionNoAckSetup(ASKUSER_AEM_PASSPHRASE);
				if (!permission_denied)
				{
					// User approved action.
					approved4 = true;
				}
				if(approved4)
				{

					buttonInterjectionNoAckSetup(ASKUSER_ALPHA_INPUT_PREFACE);


					encryption_phrase_char_ptr = getInput(true, false);
					int i;
					for (i = 0; i<sizeof(encryption_phrase_char)-1;i++)
					{
						encryption_phrase_array[i] = encryption_phrase_char_ptr[i];
					}

					clearDisplay();
					permission_denied = buttonInterjectionNoAckPlusData(ASKUSER_SHOW_UNLOCKPHRASE, encryption_phrase_char_ptr, 0);
					if (!permission_denied)
					{
						// User approved action.
						approved5 = true;
					}
					if(approved5)
					{

						encryptStreamSized(display_phrase_array, encryption_phrase_array, 64);

						nonVolatileWrite(display_phrase_array, AEM_PHRASE_ADDRESS, 64);
						toggleAEM(true);
					}
				}
			}
		}
	}
}

int fetchDevicePINWrongCount(void)
{
	int isForm;
	uint8_t tempComms[1];
	nonVolatileRead(tempComms, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);

	isForm = (int)tempComms[0];

	return isForm;
}

int fetchTransactionPINWrongCount(void)
{
	int isForm;
	uint8_t tempComms[1];
	nonVolatileRead(tempComms, WRONG_TRANSACTION_PIN_COUNT_ADDRESS, 1);

	isForm = (int)tempComms[0];

	return isForm;
}


const int keyDelayNoDisplay = 200;

int entryDelayNoDisplay = 0;

uint8_t *fetchPIN(void)
{
	uint8_t tempPIN[32];
	nonVolatileRead(tempPIN, PIN_ADDRESS, 32);
	return tempPIN;
}

void pinStatusCheckandPremadePIN()
{
	#ifdef DEBUG_MODE
	Serial.println(" ---------pinStatusCheckandPremadePIN----------");
	#endif
	uint8_t temp[32];
	uint8_t ref_compare_hash[32];
	uint8_t j;
	HashState *sig_hash_hs_ptr;
	HashState sig_hash_hs;
	HashState *sig_hash_hs_ptr2;
	HashState sig_hash_hs2;

	sig_hash_hs_ptr = &sig_hash_hs;
	sig_hash_hs_ptr2 = &sig_hash_hs2;

	uint8_t *hash_ptr;
	uint8_t hash[32] = {};
	uint8_t *hash2_ptr;
	uint8_t hash2[32] = {};

	//  char *p;
	//  p=&theStringArray[0];

	char bufferPIN1array[21]={};
	char bufferPIN2array[21]={};
	char bufferPIN3array[21]={};

	char *bufferPIN1;
	char *bufferPIN2;
	char *bufferPIN3;

	bufferPIN1 = &bufferPIN1array[0];
	bufferPIN2 = &bufferPIN2array[0];
	bufferPIN3 = &bufferPIN3array[0];


	int pinStatus;
	pinStatus = checkHasPIN();
	#ifdef DEBUG_MODE
	Serial.print("---------pinStatus------");
	Serial.println(pinStatus);
	#endif

	if(pinStatus != 127)
		{
			char rChar;
			int r;
			bool yesOrNo;


			buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_STANDARD_SETUP_2);

			r = waitForNumberButtonPress4to8();
			tftBlackScreen();
			// r = rChar - '0';

			if(r == 10)
			{
				useWhatSetup();
			}else
			{
				generateInsecurePIN(bufferPIN1, r+1);

				sha256Begin(sig_hash_hs_ptr);

				for (j=0;j<20;j++)
				{
					sha256WriteByte(sig_hash_hs_ptr, bufferPIN1[j]);
				}
				sha256FinishDouble(sig_hash_hs_ptr);
				writeHashToByteArray(hash, sig_hash_hs_ptr, false);

				if(1)
				{
					int s = 0;
					uint8_t temp1[1];
					temp1[0] = s;
					nonVolatileWrite(temp1, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);

					buttonInterjectionNoAckPlusData(ASKUSER_SET_DEVICE_PIN_BIG, bufferPIN1, r);
					yesOrNo = waitForButtonPress();
					tftBlackScreen();
					if(!yesOrNo)
					{
						nonVolatileWrite(hash, PIN_ADDRESS, 32);

						int s = 127;
						uint8_t temp1[1];
						temp1[0] = s;
						nonVolatileWrite(temp1, HAS_PIN_ADDRESS, 1);

						int type = 127;
						uint8_t temp2[1];
						temp2[0] = type;
						nonVolatileWrite(temp2, SETUP_TYPE_ADDRESS, 1);

						#ifdef DEBUG_MODE
						Serial.println("----nonVolatileWrite PIN----");
						#endif
					}else if(yesOrNo)
					{
						pinStatusCheckandPremadePIN();
					}
				}
				else
				{
//					writeNotEqual_Screen();
//	//				writeEink("PIN NOT EQUAL", false, STATUS_X, STATUS_Y);
//					pinStatusCheck();
				}
			}
		}
		else if (pinStatus == 127)
		{
			checkDevicePIN(false);
/*
	  		memset(bufferPIN3, 0, 20);
	  		strcpy(bufferPIN3, "");
			writeEink("PIN:", false, STATUS_X, STATUS_Y);
			bufferPIN3 = getInput(false);


			char *duress = "911" ;
			if(*bufferPIN3 == *duress)
			{
//				writeEink("STOMP", false, STATUS_X, STATUS_Y);
				duressFormat();
				while(1){;;};
			}
			checkHashesNoDisplay(bufferPIN3);
*/
		}
		else
		{
			char textDisplay[] = {"PIN ERROR - REFLASH"};
			writeEink(textDisplay, false, STATUS_X, STATUS_Y);
			while(1){;;};
		}
	return;
}


void checkDevicePIN(bool displayAlpha)
{
	char bufferPIN3array[21]={};
	char *bufferPIN3;

	bufferPIN3 = &bufferPIN3array[0];

//	memset(&bufferPIN3array[0], 0, sizeof(bufferPIN3array));
	memset(bufferPIN3, 0, 21);
	strcpy(bufferPIN3, "");

	if(displayAlpha){
		buttonInterjectionNoAck(ASKUSER_ENTER_PIN_ALPHA);
	}else{
		buttonInterjectionNoAck(ASKUSER_ENTER_PIN);
	}

	#ifdef DEBUG_MODE
	Serial.println("-----before getInput----------");	
	#endif
	// bufferPIN3 = waitForNumberButtonPress();
	bufferPIN3 = getInput(displayAlpha, false);
	#ifdef DEBUG_MODE
	Serial.println("-----after getInput----------");	
	#endif
	tftBlackScreen();
	#ifdef DEBUG_MODE
	Serial.println("-----before checkhashes----------");
	#endif
	checkHashes(bufferPIN3, displayAlpha);
	#ifdef DEBUG_MODE
	Serial.println("-----after checkhashes----------");
	#endif
}

char *getTransactionPINfromUser(void)
{
	char bufferPIN3array[21]={};
	char *bufferPIN3;

	bufferPIN3 = &bufferPIN3array[0];

	memset(bufferPIN3, 0, 21);
	strcpy(bufferPIN3, "");

	buttonInterjectionNoAck(ASKUSER_ENTER_TRANSACTION_PIN);


	bufferPIN3 = getInput(false, false);


	return bufferPIN3;
}

void duressFormat()
{
//	writeEink("STOMP2", false, STATUS_X, STATUS_Y);
	writeCheck_Screen();
//	writeEink("STOMP3", false, STATUS_X, STATUS_Y);
	initialFormatAutoDuress();
//	writeEink("STOMP4", false, STATUS_X, STATUS_Y);
	useWhatCommsDuress();
}

void pinStatusCheck()
{
	uint8_t temp[32];
	uint8_t ref_compare_hash[32];
	uint8_t j;
	HashState *sig_hash_hs_ptr;
	HashState sig_hash_hs;
	HashState *sig_hash_hs_ptr2;
	HashState sig_hash_hs2;

	sig_hash_hs_ptr = &sig_hash_hs;
	sig_hash_hs_ptr2 = &sig_hash_hs2;

	uint8_t *hash_ptr;
	uint8_t hash[32] = {};
	uint8_t *hash2_ptr;
	uint8_t hash2[32] = {};


	char bufferPIN1array[21]={};
	char bufferPIN2array[21]={};
	char bufferPIN3array[21]={};

	char *bufferPIN1;
	char *bufferPIN2;
	char *bufferPIN3;

	bufferPIN1 = &bufferPIN1array[0];
	bufferPIN2 = &bufferPIN2array[0];
	bufferPIN3 = &bufferPIN3array[0];


	int pinStatus;
	pinStatus = checkHasPIN();

	if(pinStatus != 127)
	{
		char rChar;
		int r;
		bool yesOrNo;

		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_ADVANCED_SETUP_2);
//		initDisplay();
//
//		overlayBatteryStatus(BATT_VALUE_DISPLAY);
//		writeEinkNoDisplay("ADVANCED SETUP",  COL_1_X, LINE_1_Y, "TO SET DEVICE PIN",COL_1_X,LINE_2_Y, "-PRESS AND HOLD TO CYCLE",COL_1_X,LINE_3_Y, "-RELEASE TO SELECT",COL_1_X,LINE_4_Y, "",25,80);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "GO",22,80, "BACK",148,80);
//		drawCheck(3,85);
//		drawX(180,87);
//
//		display();

		yesOrNo = waitForButtonPress();
		clearDisplay();
		if(!yesOrNo)
		{
			buttonInterjectionNoAckSetup(ASKUSER_ALPHA_INPUT_PREFACE);

//			initDisplay();
//			writeEinkNoDisplaySingle("MINIMUM 4 CHARACTERS",  COL_1_X, LINE_1_Y);
//			writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//			display();
//			initDisplay();
//			writeEinkNoDisplaySingle("PRESS/HOLD/RELEASE",  COL_1_X, LINE_1_Y);
//			writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//			display();
			bufferPIN1 = getInput(true, true);
			clearDisplay();
		}else if(yesOrNo)
		{
			useWhatSetup();
		}

		sha256Begin(sig_hash_hs_ptr);

		for (j=0;j<20;j++)
		{
			sha256WriteByte(sig_hash_hs_ptr, bufferPIN1[j]);
		}
		sha256FinishDouble(sig_hash_hs_ptr);
		writeHashToByteArray(hash, sig_hash_hs_ptr, false);


		if(1)
		{
			int s = 0;
			uint8_t temp1[1];
			temp1[0] = s;
			nonVolatileWrite(temp1, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);

			buttonInterjectionNoAckPlusData(ASKUSER_SET_DEVICE_PIN, bufferPIN1, sizeof(*bufferPIN1));

			yesOrNo = waitForButtonPress();
			clearDisplay();

			if(!yesOrNo)
			{
				nonVolatileWrite(hash, PIN_ADDRESS, 32);

				int s = 127;
				uint8_t temp1[1];
				temp1[0] = s;
				nonVolatileWrite(temp1, HAS_PIN_ADDRESS, 1);

				int type = 128;
				uint8_t temp2[1];
				temp2[0] = type;
				nonVolatileWrite(temp2, SETUP_TYPE_ADDRESS, 1);

				delay(1000);
			}else if(yesOrNo)
			{
				strcpy(bufferPIN1array, "");
				useWhatSetup();
			}
		}
		else
		{
//			writeNotEqual_Screen();
//			writeEink("PINs DO NOT MATCH", false, STATUS_X, STATUS_Y);
//			pinStatusCheck();
		}
	}
	else if (pinStatus == 127)
	{
		checkDevicePIN(true);
//
//		memset(bufferPIN3, 0, 20);
//		strcpy(bufferPIN3, "");
//
//		initDisplay();
//		overlayBatteryStatus();
//		writeEinkNoDisplay("PIN:",  STATUS_X, STATUS_Y, "",5,25, "",5,40, "",5,55, "",25,80);
//		display();
//
//		bufferPIN3 = getInput(true, false);
//		checkHashes(bufferPIN3, true);
	}
	else
	{
		char textDisplay[] = {"PIN ERROR - REFLASH"};
		writeEink(textDisplay, false, STATUS_X, STATUS_Y);
		while(1){;;};
	}
	return;
}

void pinStatusCheckExpert()
{
	uint8_t temp[32];
	uint8_t ref_compare_hash[32];
	uint8_t j;
	HashState *sig_hash_hs_ptr;
	HashState sig_hash_hs;
	HashState *sig_hash_hs_ptr2;
	HashState sig_hash_hs2;

	sig_hash_hs_ptr = &sig_hash_hs;
	sig_hash_hs_ptr2 = &sig_hash_hs2;

	uint8_t *hash_ptr;
	uint8_t hash[32] = {};
	uint8_t *hash2_ptr;
	uint8_t hash2[32] = {};


	char bufferPIN1array[21]={};
	char bufferPIN2array[21]={};
	char bufferPIN3array[21]={};

	char *bufferPIN1;
	char *bufferPIN2;
	char *bufferPIN3;

	bufferPIN1 = &bufferPIN1array[0];
	bufferPIN2 = &bufferPIN2array[0];
	bufferPIN3 = &bufferPIN3array[0];


	int pinStatus;
	pinStatus = checkHasPIN();

	if(pinStatus != 127)
	{
		char rChar;
		int r;
		bool yesOrNo;

		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_EXPERT_SETUP_2);
//		initDisplay();
//
//		overlayBatteryStatus(BATT_VALUE_DISPLAY);
//		writeEinkNoDisplay("EXPERT SETUP",  COL_1_X, LINE_1_Y, "TO SET DEVICE PIN",COL_1_X,LINE_2_Y, "-PRESS AND HOLD TO CYCLE",COL_1_X,LINE_3_Y, "-RELEASE TO SELECT",COL_1_X,LINE_4_Y, "",25,80);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "GO",22,80, "BACK",148,80);
//		drawCheck(3,85);
//		drawX(180,87);
//
//		display();

		yesOrNo = waitForButtonPress();
		clearDisplay();
		if(!yesOrNo)
		{
			buttonInterjectionNoAckSetup(ASKUSER_ALPHA_INPUT_PREFACE);

//			initDisplay();
//			writeEinkNoDisplaySingle("MINIMUM 4 CHARACTERS",  COL_1_X, LINE_1_Y);
//			writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//			display();
//			initDisplay();
//			writeEinkNoDisplaySingle("PRESS/HOLD/RELEASE",  COL_1_X, LINE_1_Y);
//			writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//			display();
			bufferPIN1 = getInput(true, true);
			clearDisplay();
		}else if(yesOrNo)
		{
			useWhatSetup();
		}

		sha256Begin(sig_hash_hs_ptr);

		for (j=0;j<PIN_MAX_SIZE;j++)
		{
			sha256WriteByte(sig_hash_hs_ptr, bufferPIN1[j]);
		}
		sha256FinishDouble(sig_hash_hs_ptr);
		writeHashToByteArray(hash, sig_hash_hs_ptr, false);


		if(1)
		{
			int s = 0;
			uint8_t temp1[1];
			temp1[0] = s;
			nonVolatileWrite(temp1, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);

			buttonInterjectionNoAckPlusData(ASKUSER_SET_DEVICE_PIN, bufferPIN1, 0);


			yesOrNo = waitForButtonPress();
			clearDisplay();

			if(!yesOrNo)
			{
				nonVolatileWrite(hash, PIN_ADDRESS, 32);

				int s = 127;
				uint8_t temp1[1];
				temp1[0] = s;
				nonVolatileWrite(temp1, HAS_PIN_ADDRESS, 1);

				int type = 129;
				uint8_t temp2[1];
				temp2[0] = type;
				nonVolatileWrite(temp2, SETUP_TYPE_ADDRESS, 1);

				delay(1000);
			}else if(yesOrNo)
			{
				useWhatSetup();
			}
		}
		else
		{
//			writeNotEqual_Screen();
//			writeEink("PINs DO NOT MATCH", false, STATUS_X, STATUS_Y);
//			pinStatusCheck();
		}
	}
	else if (pinStatus == 127)
	{
		checkDevicePIN(true);
//		memset(bufferPIN3, 0, 20);
//		strcpy(bufferPIN3, "");
//
//		initDisplay();
//		overlayBatteryStatus();
//		writeEinkNoDisplay("PIN:",  STATUS_X, STATUS_Y, "",5,25, "",5,40, "",5,55, "",25,80);
//		display();
//
//		bufferPIN3 = getInput(true, false);
//		checkHashes(bufferPIN3, true);
	}
	else
	{
		char textDisplay[] = {"PIN ERROR - REFLASH"};
		writeEink(textDisplay, false, STATUS_X, STATUS_Y);
		while(1){;;};
	}
	return;
}

char *mnemonic_input_stacker(int mlen)
{

	int i, j, idx;
	const char *theResult;
	static char mnemo[24 * 10];
	char *p = mnemo;


	char mlenText[1];
	sprintf(mlenText,"%lu", mlen);


	for (i=0; i<mlen;i++)
	{
		char iText[1];
		sprintf(iText,"%lu", i+1);

		initDisplay();
		writeEinkNoDisplay("MNEMONIC",  COL_1_X, LINE_1_Y, iText,COL_1_X+75,LINE_1_Y, "of",COL_1_X+100,LINE_1_Y, mlenText,COL_1_X+125,LINE_1_Y, "",25,80);
		display();
		theResult = mnemonic_input();
		strcpy(p, theResult);
		p += strlen(theResult);
		*p = (i < mlen - 1) ? ' ' : 0;
		p++;
	}
	showWorking();
	return mnemo;
}

//Need to put a wrapper and display on this to loop the query
char *mnemonic_input(void)
{
	  int i;
	  char theChar;
	  static char staticBuffer[21] = {};
	  char tempBuffer[21] = {};
	  int j;
	  bool caps = false;
	  bool displayInput = true;
	  memset(staticBuffer, 0, 21);
	  strcpy(staticBuffer, "");

	  for (i=0; i<20; i++)
	  {

			#if defined(__SAM3A8C__)
			if(displayInput)
				{
				if(caps)
					{
						theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
					}else
					{
						theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
					}
				}
				else
				{
					theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
				}
			#endif

			#if defined(NRF52840_XXAA)
			if(displayInput)
				{
				if(caps)
					{
//						theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
					}else
					{
//						theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
					}
				}
				else
				{
//					theChar = alphkeypad_noNumbers_3A8C((8*i)+INPUT_X, INPUT_Y);
				}
			#endif

	            if(theChar == '@')
	             {
	             	break;
	             }
	             else if(theChar == '^')
	             {
	             	i = i - 1;
	             	caps = !caps;
	             	writeSelectedCharAndStringBlankingMnemonics((8*i)+INPUT_X, INPUT_Y, caps);
	             }
	             else if(theChar == '~' && (i != 0))
	             {
	             	strncpy(tempBuffer, staticBuffer, i-1);
	     	  		strcpy(staticBuffer, "");
	     	  		strcpy(staticBuffer, tempBuffer);
	             	i= i - 2;
	             	if(displayInput){
	             		writeSelectedCharAndStringBlankingMnemonics((8*i)+INPUT_X, INPUT_Y, caps);
	             	}
	             }
	             else if(theChar == '~' && (i == 0))
	             {
	             	i = i - 1;
	     				if(displayInput){
//	     					checkDevicePIN(false);
	     					break;
	     				}else{
//	     					checkDevicePIN(true);
	     					break;
	     				}
	             }
	             else
	             {
	             	staticBuffer[i] = theChar;
	             }

	  }
	  return staticBuffer;
}


// int getAndReturnInput()

char *getInput(bool displayInput, bool initialSetup) {
  int i;
  char theChar;
  static char staticBuffer[21] = {};
  char tempBuffer[21] = {};
  int j;
//   bool caps = false;

  memset(&staticBuffer[0], 0, sizeof(staticBuffer));

  for (i = 0; i < 20; i++)
  {
	  if (displayInput)
	  {
		  if (capsLock)
		  {
			  					theChar = getAndReturnInputDisplay((50*i)+INPUT_X, INPUT_Y);
		  }
		  else
		  {
			  					theChar = getAndReturnInputDisplay((50*i)+INPUT_X, INPUT_Y);
		  }
	  }
	  else
	  {
		theChar = alphkeypad_3A8CnoDisplay((8 * i) + INPUT_X, INPUT_Y);
		// theChar = getAndReturnInputDisplay((8 * i) + INPUT_X, INPUT_Y);
		// test
	}

	  if (initialSetup)
	  {
		  j = 3;
		  if (theChar == '@' && i > j)
		  {
			  break;
		  }
		  else if (theChar == '@' && i <= j)
		  {
			  i = i - 1;
		  }
		  else if (theChar == '^')
		  {
			  i = i - 1;
			//   caps = !caps;
			//   writeSelectedCharAndStringBlanking((8 * i) + INPUT_X, INPUT_Y, caps);
		  }
		  else if (theChar == '~' && (i != 0))
		  {
			  memset(&tempBuffer[0], 0, sizeof(tempBuffer));
			  strncpy(tempBuffer, staticBuffer, i - 1);
			  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
			  strncpy(staticBuffer, tempBuffer, i - 1);
			  i = i - 2;
			  if (display)
			  {
				  writeSelectedCharAndStringBlanking((8 * i) + INPUT_X, INPUT_Y, capsLock);
			  }
		  }
		  else if (theChar == '~' && (i == 0))
		  {
			  i = i - 1;
			  if (!initialSetup)
			  {
				  if (displayInput)
				  { // ############################################################
					  //        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
					  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
					  checkDevicePIN(false);
					  //        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);

					  break;
				  }
				  else
				  {
					  //        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
					  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
					  checkDevicePIN(true);
					  //        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
					  break;
				  }
			  }
			  else if (initialSetup)
			  {
				  useWhatSetup();
			  }
		  }
		  else
		  {
			  staticBuffer[i] = theChar;
		  }
	  }
	  else
	  {
		  if (theChar == '@')
		  {
			  break;
		  }
		  else if (theChar == '^')
		  {
			  i = i - 1;
			//   caps = !caps;
			//   writeSelectedCharAndStringBlanking((8 * i) + INPUT_X, INPUT_Y, caps);
		  }
		  else if (theChar == '~' && (i != 0) && displayInput)
		  {
			  memset(&tempBuffer[0], 0, sizeof(tempBuffer));
			  strncpy(tempBuffer, staticBuffer, i - 1);
			  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
			  strncpy(staticBuffer, tempBuffer, i - 1);
			  i = i - 2;
			  if (display)
			  {
				  writeSelectedCharAndStringBlanking((8 * i) + INPUT_X, INPUT_Y, capsLock);
			  }
		  }
		  else if (theChar == '~' && (i == 0))
		  {
			  i = i - 1;
			  if (!initialSetup)
			  {
				  if (displayInput)
				  {
					  //        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
					  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
					  checkDevicePIN(false);
					  //        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);
					  break;
				  }
				  else
				  {
					  //        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
					  memset(&staticBuffer[0], 0, sizeof(staticBuffer));
					  checkDevicePIN(true);
					  //        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
					  break;
				  }
			  }
			  else if (initialSetup)
			  {
				  useWhatSetup();
			  }
		  }
		  else
		  {
			  staticBuffer[i] = theChar;
		  }
	  }
  }
  return staticBuffer;
}

char *getInputIndices(bool displayInput, bool initialSetup) {
  int i;
  char theChar;
  static char staticBuffer[21] = {};
  static char finalBuffer[21] = {};
  char tempBuffer[21] = {};
  int j;
//   bool caps = false;

  memset(staticBuffer, 0, 21);
  strcpy(staticBuffer, "");

  for (i=0; i<20; i++)
  {


	if(displayInput)
		{
		if(capsLock)
			{
				theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
			}else
			{
				theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
			}
		}
		else
		{
			theChar = alphkeypad_3A8CnoDisplay((8*i)+INPUT_X, INPUT_Y);
		}

if(initialSetup)
        {
        	j = 3;
            if(theChar == '@' && i > j)
             {
             	break;
             }
             else if(theChar == '@' && i <= j)
             {
             	i = i - 1;
             }
             else if(theChar == '^')
             {
             	i = i - 1;
             	// caps = !caps;
             	// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             }
             else if(theChar == '~' && (i != 0))
             {
             	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
              	strncpy(tempBuffer, staticBuffer, i-1);
              	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
      	  		strncpy(staticBuffer, tempBuffer, i-1);
             	i= i - 2;
             	if(display){
             		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             	}
             }
             else if(theChar == '~' && (i == 0))
             {
             	i = i - 1;
             	if(!initialSetup)
             	{
     				if(displayInput){//############################################################
     	//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(false);
     	//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);

     					break;
     				}else{
     	//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(true);
     	//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
     					break;
     				}
             	}else if(initialSetup)
             	{
             		useWhatSetup();
             	}
             }
             else
             {
             	staticBuffer[i] = theChar;
             }
        }
        else
        {
            if(theChar == '@')
             {
             	break;
             }
//             else if(theChar == '^')
//             {
//             	i = i - 1;
//             	caps = !caps;
//             	writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, caps);
//             }
             else if(theChar == '~' && (i != 0))
             {
             	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
              	strncpy(tempBuffer, staticBuffer, i-1);
              	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
      	  		strncpy(staticBuffer, tempBuffer, i-1);
             	i= i - 1;
             	if(display){
             		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             	}
             }
//             else if(theChar == '~' && (i == 0))
//             {
//             	i = i - 1;
//             	if(!initialSetup)
//             	{
//     				if(displayInput){
//     	//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
//     					strcpy(staticBuffer, "");
//     					checkDevicePIN(false);
//     	//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);
//     					break;
//     				}else{
//     	//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
//     					strcpy(staticBuffer, "");
//     					checkDevicePIN(true);
//     	//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
//     					break;
//     				}
//             	}else if(initialSetup)
//             	{
//             		useWhatSetup();
//     //        		initDisplay();
//     //        		overlayBatteryStatus(BATT_VALUE_DISPLAY);
//     //        		writeEinkNoDisplay("SET NUMERIC PIN:",  COL_1_X, LINE_1_Y, "",0,0, "",  0,0, "",  0,0, "",5,35);
//     //        		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//     //				writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "GO",22,80, "NUM",145,80);
//     //				drawCheck(3,85);
//     //           		drawX(180,87);
//     //        		display();
//     //        		break;
//     //        		getInput(!displayInput, true);
//             	}
//             }
//             else
             {
             	staticBuffer[i] = theChar;
             }
       }
  }
//  strncpy(finalBuffer, staticBuffer, i);
  return staticBuffer;
}

char *getInputAEM(bool displayInput, bool initialSetup)
{
  int i;
  char theChar;
  static char staticBuffer[21] = {};
  static char finalBuffer[21] = {};
  char tempBuffer[21] = {};
  int j;
//   bool caps = false;

  memset(staticBuffer, 0, 21);
  strcpy(staticBuffer, "");

  for (i=0; i<20; i++)
  {
        if(displayInput)
		{
			if(capsLock)
			{
				// theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
			}
			else
			{
				// theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
			}
		}
		else
		{
//			theChar = alphkeypad_3A8CnoDisplay((8*i)+INPUT_X, INPUT_Y);
		}


		if(theChar == '@')
		 {
			break;
		 }
		 else if(theChar == '^')
		 {
			i = i - 1;
			// caps = !caps;
			// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
		 }
		 else if(theChar == '~' && (i != 0) && displayInput)
		 {
         	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
          	strncpy(tempBuffer, staticBuffer, i-1);
          	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
  	  		strncpy(staticBuffer, tempBuffer, i-1);
			i= i - 2;
			if(displayInput)
			{
				writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
			}
		 }
		 else if(theChar == '~' && (i == 0))
			 {
				i = i - 1;
				if(!initialSetup)
					{
						if(displayInput)
							{
				//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
//								checkDevicePIN(false);
								memset(&staticBuffer[0], 0, sizeof(staticBuffer));
								doAEMValidate(false);
//								getInputAEM(false, false);
				//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);
								break;
							}
						else
							{
				//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
//								checkDevicePIN(true);
								memset(&staticBuffer[0], 0, sizeof(staticBuffer));
								doAEMValidate(true);
//								getInputAEM(true, false);
				//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
								break;
							}
					}

//				if(!initialSetup)
//					{
//						if(displayInput)
//							{
//				//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
////								checkDevicePIN(false);
//				          		memset(&staticBuffer[0], 0, sizeof(staticBuffer));
//								doAEMValidate(false);
//				//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);
//								break;
//							}
//						else
//							{
//				//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
////								checkDevicePIN(true);
//				          		memset(&staticBuffer[0], 0, sizeof(staticBuffer));
//								doAEMValidate(true);
//				//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
//								break;
//							}
//					}
			 }
			 else
		 {
			staticBuffer[i] = theChar;
		 }

  } // end for 20
//  strncpy(finalBuffer, staticBuffer, i);
  return staticBuffer;
}

char *getInputVariable(bool displayInput, bool initialSetup, int numChars) {
  int i;
  char theChar;
  static char staticBuffer[21] = {};
  static char finalBuffer[21] = {};
  char tempBuffer[21] = {};
  int j;
//   bool caps = false;

  memset(staticBuffer, 0, 21);
  strcpy(staticBuffer, "");

  for (i=0; i<20; i++)
  {


        if(displayInput)
			{
        	if(capsLock)
				{
					theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
				}else
				{
					theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
				}
			}
			else
			{
				theChar = alphkeypad_3A8CnoDisplay((8*i)+INPUT_X, INPUT_Y);
			}

        if(initialSetup)
        {
        	j = 3;
            if(theChar == '@' && i > j)
             {
             	break;
             }
             else if(theChar == '@' && i <= j)
             {
             	i = i - 1;
             }
             else if(theChar == '^')
             {
             	i = i - 1;
             	// caps = !caps;
             	// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             }
             else if(theChar == '~' && (i != 0))
             {
             	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
              	strncpy(tempBuffer, staticBuffer, i-1);
              	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
      	  		strncpy(staticBuffer, tempBuffer, i-1);
             	i= i - 2;
             	if(display){
             		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             	}
             }
             else if(theChar == '~' && (i == 0))
             {
             	i = i - 1;
             	if(!initialSetup)
             	{
     				if(displayInput){
     	//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(false);
     	//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);

     					break;
     				}else{
     	//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(true);
     	//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
     					break;
     				}
             	}else if(initialSetup)
             	{
             		useWhatSetup();
             	}
             }
             else
             {
             	staticBuffer[i] = theChar;
             }
        }
        else
        {
            if(theChar == '@')
             {
             	break;
             }
             else if(theChar == '^')
             {
             	i = i - 1;
             	// caps = !caps;
             	// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, caps);
             }
             else if(theChar == '~' && (i != 0))
             {
             	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
              	strncpy(tempBuffer, staticBuffer, i-1);
              	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
      	  		strncpy(staticBuffer, tempBuffer, i-1);
             	i= i - 2;
             	if(display){
             		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
             	}
             }
             else if(theChar == '~' && (i == 0))
             {
             	i = i - 1;
             	if(!initialSetup)
             	{
     				if(displayInput){
     	//        		writeEink("before cDP true>false", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(false);
     	//        		writeEink("stuck cDP true>false", false, STATUS_X, STATUS_Y);

     					break;
     				}else{
     	//        		writeEink("before cDP false>true", false, STATUS_X, STATUS_Y);
     					checkDevicePIN(true);
     	//        		writeEink("stuck cDP false>true", false, STATUS_X, STATUS_Y);
     					break;
     				}
             	}else if(initialSetup)
             	{
             		useWhatSetup();
              	}
             }
             else
             {
             	staticBuffer[i] = theChar;
             }
       }
  }
//  strncpy(finalBuffer, staticBuffer, i);
  return staticBuffer;
}

char *getInputWallets(bool displayInput, bool initialSetup) {
  int i;
  char theChar;
  static char staticBuffer[21] = {};
  static char finalBuffer[21] = {};
  char tempBuffer[21] = {};
  int j;
//   bool caps = false;

  memset(staticBuffer, 0, 21);
  strcpy(staticBuffer, "");

  for (i=0; i<20; i++)
  {

        if(displayInput)
		{
			if(capsLock)
				{
					theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
				}else
				{
					theChar = getAndReturnInputDisplay((8*i)+INPUT_X, INPUT_Y);
				}
		}
		else
		{
			theChar = alphkeypad_3A8CnoDisplay((8*i)+INPUT_X, INPUT_Y);
		}

        if(initialSetup)
        {
        	j = 3;
            if(theChar == '@' && i > j)
            {
            	break;
            }
            else if(theChar == '@' && i <= j)
            {
            	i = i - 1;
            }
            else if(theChar == '^')
            {
            	i = i - 1;
            	// caps = !caps;
            	// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
            }
            else if(theChar == '~' && (i != 0))
            {
            	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
             	strncpy(tempBuffer, staticBuffer, i-1);
             	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
     	  		strncpy(staticBuffer, tempBuffer, i-1);
            	i= i - 2;
            	if(display){
            		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
            	}
            }
            else if(theChar == '~' && (i == 0))
            {
           		passwordInterjectionAutoPIN(1);
            }
            else
            {
            	staticBuffer[i] = theChar;
            }
        }
        else
        {
            if(theChar == '@')
            {
            	return staticBuffer;
            	break;
            }
            else if(theChar == '^')
            {
            	i = i - 1;
            	// caps = !caps;
            	// writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
            }
            else if(theChar == '~' && (i != 0)  && displayInput)
            {
            	memset(&tempBuffer[0], 0, sizeof(tempBuffer));
             	strncpy(tempBuffer, staticBuffer, i-1);
             	memset(&staticBuffer[0], 0, sizeof(staticBuffer));
     	  		strncpy(staticBuffer, tempBuffer, i-1);
            	i= i - 2;
            	if(display){
            		writeSelectedCharAndStringBlanking((8*i)+INPUT_X, INPUT_Y, capsLock);
            	}
            }
            else if(theChar == '~' && (i == 0))
            {
            	i = i - 1;
				displayInput = !displayInput;
				if(displayInput)
				{
					strcpy(staticBuffer, "");
					buttonInterjectionNoAck(ASKUSER_ENTER_WALLET_PIN_ALPHA);
				}else{
					strcpy(staticBuffer, "");
					buttonInterjectionNoAck(ASKUSER_ENTER_WALLET_PIN);
				}
            }
            else
            {
            	staticBuffer[i] = theChar;
            }
        }

  }
//  strncpy(finalBuffer, staticBuffer, i);
  return staticBuffer;
}

void checkHashes(char *buffer4, bool displayAlpha)
{
	uint8_t k;
	HashState *sig_hash_hs_ptr3;
	HashState sig_hash_hs3;
	uint8_t hash3[32] = {};

	HashState *sig_hash_hs_ptrDuress;
	HashState sig_hash_hsDuress;
	uint8_t hashDuress[32] = {};

	uint8_t toBeMatched[32] = {};
	uint8_t matchCheckDuress;
	uint8_t matchCheck;
	uint8_t tempPIN[32];

	char duressarray[21]={};
	char *duress;

	duress = &duressarray[0];

	memset(duress, 0, 20);
	strcpy(duress, "911");

	sig_hash_hs_ptrDuress = &sig_hash_hsDuress;

	sha256Begin(sig_hash_hs_ptrDuress);

	for (k=0;k<20;k++)
	{
		sha256WriteByte(sig_hash_hs_ptrDuress, duress[k]);
	}
	sha256FinishDouble(sig_hash_hs_ptrDuress);
	writeHashToByteArray(hashDuress, sig_hash_hs_ptrDuress, false);

	sig_hash_hs_ptr3 = &sig_hash_hs3;

	sha256Begin(sig_hash_hs_ptr3);

	for (k=0;k<20;k++)
	{
		sha256WriteByte(sig_hash_hs_ptr3, buffer4[k]);
	}
	sha256FinishDouble(sig_hash_hs_ptr3);
	writeHashToByteArray(hash3, sig_hash_hs_ptr3, false);


	matchCheckDuress = memcmp(hash3,hashDuress,32);

	if(matchCheckDuress == 0){
//		writeEink("STOMP", false, STATUS_X, STATUS_Y);
		duressFormat();
		while(1){;;};
	}



	nonVolatileRead(tempPIN, PIN_ADDRESS, 32);

	int entryCount;
	int wrongTransactionPINCount;
	int entryDelay;
	int entryMultiplier;
	int entryMultiplierTRANS;
	int totalDelay;
	long totalDelaySeconds;
	int haltThreshold = 5;
	char intervalText[20];
	char *secondsText = "seconds";
	char *minutesText = "minutes";
	char *hoursText = "hours";
	char *daysText = "days";
	char *monthsText = "months";
	char *yearsText = "years";
	char *decadesText = "decades";
	char *centuriesText = "centuries";
	char *millenniaText = "millennia";
	long intervalValue;

	entryCount = fetchDevicePINWrongCount();
	wrongTransactionPINCount = fetchTransactionPINWrongCount();



	if(entryCount != 0 || wrongTransactionPINCount != 0)
	{
		entryMultiplier = pow(2, entryCount);
		if(wrongTransactionPINCount != 0)
		{
			wrongTransactionPINCount = wrongTransactionPINCount + 5;
			entryMultiplierTRANS = pow(3, wrongTransactionPINCount);
		}else{
			entryMultiplierTRANS = 0;
		}
		totalDelay = 1000 * (entryMultiplier + entryMultiplierTRANS);
		totalDelaySeconds = totalDelay/1000;
		char entryCountPrint[16];
		sprintf(entryCountPrint,"%lu", entryCount+1);



		if(totalDelaySeconds < 60)
		{
			strcpy(intervalText, secondsText);
			intervalValue = 1;
		}else if(totalDelaySeconds >= 60 &&  totalDelaySeconds < 3600)
		{
			strcpy(intervalText, minutesText);
			intervalValue = 60;
		}else if(totalDelaySeconds >= 3600 && totalDelaySeconds < 86400)
		{
			strcpy(intervalText, hoursText);
			intervalValue = 3600;
		}else if(totalDelaySeconds >= 86400 && totalDelaySeconds < 2592000)
		{
			strcpy(intervalText, daysText);
			intervalValue = 86400;
		}else if(totalDelaySeconds >= 2592000 && totalDelaySeconds < 31104000)
		{
			strcpy(intervalText, monthsText);
			intervalValue = 2592000;
		}else if(totalDelaySeconds >= 31104000 && totalDelaySeconds < 311040000)
		{
			strcpy(intervalText, yearsText);
			intervalValue = 31104000;
		}

		long resultingDelaySeconds;
		resultingDelaySeconds = totalDelaySeconds/intervalValue;
		char entryMultPrint[16];
		sprintf(entryMultPrint,"%lu", resultingDelaySeconds);


		initDisplay();
		overlayBatteryStatus(BATT_VALUE_DISPLAY);
		writeEinkNoDisplay("SECURITY DELAY",  COL_1_X, LINE_0_Y, entryCountPrint,COL_1_X,LINE_2_Y, "attempts",35,LINE_2_Y, entryMultPrint,COL_1_X,LINE_3_Y, intervalText,35,LINE_3_Y);
//		writeEinkNoDisplay("SECURITY DELAY",  COL_1_X, LINE_1_Y, "<<TEST DELAY 5 SEC>>",COL_1_X,LINE_2_Y, "",35,LINE_2_Y, entryMultPrint,COL_1_X,LINE_3_Y, intervalText,35,LINE_3_Y);
		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
		display();

		delay(totalDelay);
//		delay(5000);
	}

	matchCheck = memcmp(hash3,tempPIN,32);

	if(matchCheck != 0)
	{
		entryCount++;
		int s = entryCount;
		uint8_t temp1[1];
		temp1[0] = s;
		nonVolatileWrite(temp1, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);

		writeX_Screen();
		if(entryCount % 10 == 0){
			writeEink("HALT", false, STATUS_X, STATUS_Y);
			while(1){;;}
		}
		else
		{
			if(displayAlpha){
				pinStatusCheck();
			}else{
				pinStatusCheckandPremadePIN();
			}
		}
	}
	else if(matchCheck == 0)
	{
		writeCheck_Screen();
		if(entryCount != 0)
		{
			int s1 = 0;
			uint8_t temp11[1];
			temp11[0] = s1;
			nonVolatileWrite(temp11, WRONG_DEVICE_PIN_COUNT_ADDRESS, 1);
		}
	}
	else
	{
		writeEink("WTF", false, STATUS_X, STATUS_Y);
		while(1){;;};
	}
}

void writeSelectedCharAndString(char currentChar, int current_x, int current_y, bool caps)
{
	initDisplay();
//	writeEinkNoDisplaySingle("PRESS/HOLD/RELEASE",  5, 3);
//	writeUnderline(5, 29, 195, 29);

	int i;
	if(currentChar != '~'){
		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
		EPAPER.drawChar(currentChar, current_x, current_y);
	}

	for(i=0;i<((current_x - INPUT_X)/8);i++)
	{
		EPAPER.drawChar('-', (8*i)+INPUT_X, INPUT_Y);
	}
	if(caps)
	{
		drawCAPSLOCK(caps_lock_X,caps_lock_Y);
	}

	display();

}

void writeSelectedCharAndStringBlanking(int current_x, int current_y, bool caps)
{
	initDisplay();

	int i;

	for(i=0;i<(((current_x - INPUT_X)+8)/8);i++)
	{
		EPAPER.drawChar('-', (8*i)+INPUT_X, INPUT_Y);
	}

	if(i<4)
	{
		buttonInterjectionNoAckSetup(ASKUSER_DELETE_ONLY_EX_DISPLAY);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "",22,80, "DELETE",132,80);
////		drawCheck(3,85);
//		drawX(180,87);
	}else
	{
		buttonInterjectionNoAckSetup(ASKUSER_ACCEPT_AND_DELETE_EX_DISPLAY);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "ACCEPT",22,80, "DELETE",132,80);
//		drawCheck(3,85);
//		drawX(180,87);
	}

	if(caps)
	{
		drawCAPSLOCK(caps_lock_X,caps_lock_Y);
	}
	display();
}

void writeSelectedCharAndStringBlankingMnemonics(int current_x, int current_y, bool caps)
{
	initDisplay();

	int i;

	for(i=0;i<(((current_x - INPUT_X)+8)/8);i++)
	{
		EPAPER.drawChar('-', (8*i)+INPUT_X, INPUT_Y);
	}

	if(i<3)
	{
		buttonInterjectionNoAckSetup(ASKUSER_DELETE_ONLY_EX_DISPLAY);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "",22,80, "DELETE",132,80);
////		drawCheck(3,85);
//		drawX(180,87);
	}else
	{
		buttonInterjectionNoAckSetup(ASKUSER_ACCEPT_AND_DELETE_EX_DISPLAY);
//		writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//		writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "ACCEPT",22,80, "DELETE",132,80);
//		drawCheck(3,85);
//		drawX(180,87);
	}

	if(caps)
	{
		drawCAPSLOCK(caps_lock_X,caps_lock_Y);
	}
	display();
}


void writeSelectedCharAndStringBlankingPlus(int current_x, int current_y)
{
	initDisplay();
	writeEinkNoDisplaySingle("OK",  5, 3);
	writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);

	int i;

	for(i=0;i<(((current_x - INPUT_X)+8)/8)-1;i++)
	{
		EPAPER.drawChar('+', (8*i)+INPUT_X, INPUT_Y);
	}
//    overlayBatteryStatus();
//
//	writeEinkNoDisplay("PIN SETUP",  5, 3, "MANUALLY SET DEVICE PIN",5,25, "PRESS AND HOLD TO SELECT",5,40, "",5,55, "",25,80);
//	writeUnderline(STRIPE_X_START, STRIPE_Y_START, STRIPE_X_END, STRIPE_Y_END);
//			writeEinkNoDisplay("",  0, 0, "",0,0, "",0,0, "GO",22,80, "BACK",148,80);
//
//			drawCheck(3,85);
//			drawX(180,87);
//
	display();
}

// char alphkeypad_3A8C(int current_x, int current_y)
// {
// 	pinMode(ROW_PIN_3, INPUT_PULLUP);
// 	pinMode(ROW_PIN_2, INPUT_PULLUP);
// 	pinMode(ROW_PIN_1, INPUT_PULLUP);
// 	pinMode(COL_PIN_1, OUTPUT);
// 	pinMode(COL_PIN_2, OUTPUT);
// 	pinMode(COL_PIN_3, OUTPUT);
// 	pinMode(COL_PIN_4, OUTPUT);
// 	digitalWrite(COL_PIN_1, HIGH);
// 	digitalWrite(COL_PIN_2, HIGH);
// 	digitalWrite(COL_PIN_3, HIGH);
// 	digitalWrite(COL_PIN_4, HIGH);

// 	int recoverDelay = 1;
// 	int postDelay = 5;
// 	bool displayOrNot = true;
// 	char selectedChar;

// 	bool caps = false;

// 	char a = 'a';

//   while (a != 'c') {




//     //  @
//     digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeys[0][0][0];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_3) == LOW) {
// //			selectedChar = hexaKeys[1][0][0];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// ////			if (digitalRead(ROW_PIN_3) == LOW) {
// ////			  selectedChar = hexaKeys[2][0][0];
// ////			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  if (digitalRead(ROW_PIN_3) == LOW) {
// //////				selectedChar = hexaKeys[3][0][0];
// //////				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  }
// ////			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  4
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeys[0][1][0];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);

//     //   8 hexaKeys[0][2][0]
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeys[0][2][0];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeys[1][2][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeys[2][2][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[3][2][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//     //   @ hexaKeys[0][0][1]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeys[0][0][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeys[1][0][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeys[2][0][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_3) == LOW) {
// 				selectedChar = hexaKeys[3][0][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     5  hexaKeys[0][1][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeys[0][1][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     9  hexaKeys[0][2][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeys[0][2][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeys[1][2][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeys[2][2][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[3][2][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);





//     //      9  hexaKeys[0][0][2]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeys[0][0][2];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeys[1][0][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			if (digitalRead(ROW_PIN_3) == LOW) {
// //			  selectedChar = hexaKeys[2][0][2];
// //			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_3) == LOW) {
// //				selectedChar = hexaKeys[3][0][2];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// //			}
// 		  }
// 		}
//       a = 'c'   ;
//     }
// 	delay(recoverDelay);



//     //     6  hexaKeys[0][1][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeys[0][1][2];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][2];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  Y=@  hexaKeys[0][2][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeys[0][2][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[1][2][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 				if (digitalRead(ROW_PIN_1) == LOW) {
// 					selectedChar = hexaKeys[2][2][2];
// 					writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					if (digitalRead(ROW_PIN_1) == LOW) {
// 						selectedChar = hexaKeys[3][2][2];
// 						writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					}
// 				}
// 			}
// 		}
// 		a = 'c';
// 	}
// 	delay(recoverDelay);




//     //  3  hexaKeys[0][0][3]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeys[0][0][3];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeys[1][0][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeys[2][0][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_3) == LOW) {
// //				selectedChar = hexaKeys[3][0][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  7  hexaKeys[0][1][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeys[0][1][3];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][3];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


// 	//  N  hexaKeys[0][2][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeys[0][2][3];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_1) == LOW) {
// //			selectedChar = hexaKeys[1][2][3];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			if (digitalRead(ROW_PIN_1) == LOW) {
// //			  selectedChar = hexaKeys[2][2][3];
// //			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_1) == LOW) {
// //				selectedChar = hexaKeys[3][2][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// //			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//   }//WHILE ENDING
//   if(selectedChar == '@')
//   {
// //	  writeSelectedCharAndStringBlankingPlus(current_x, current_y);
//   }
//   else if(selectedChar == '~')
//   {
// 	  return selectedChar;
//   }
//   else if(selectedChar == '^')
//   {
// 	  return selectedChar;
//   }
//   else
//   {
// 	  writeSelectedCharAndStringBlanking(current_x, current_y, caps);
//   }
//   return selectedChar;

// }//ALPHKEYPAD_3A8C  ENDING


// char alphkeypad_noNumbers_3A8C(int current_x, int current_y)
// {
// 	pinMode(ROW_PIN_3, INPUT_PULLUP);
// 	pinMode(ROW_PIN_2, INPUT_PULLUP);
// 	pinMode(ROW_PIN_1, INPUT_PULLUP);
// 	pinMode(COL_PIN_1, OUTPUT);
// 	pinMode(COL_PIN_2, OUTPUT);
// 	pinMode(COL_PIN_3, OUTPUT);
// 	pinMode(COL_PIN_4, OUTPUT);
// 	digitalWrite(COL_PIN_1, HIGH);
// 	digitalWrite(COL_PIN_2, HIGH);
// 	digitalWrite(COL_PIN_3, HIGH);
// 	digitalWrite(COL_PIN_4, HIGH);

// 	int recoverDelay = 1;
// 	int postDelay = 5;
// 	bool displayOrNot = true;
// 	char selectedChar;

// 	bool caps = false;

// 	char a = 'a';

//   while (a != 'c') {




//     //  @
//     digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeys[0][0][0];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_3) == LOW) {
// //			selectedChar = hexaKeys[1][0][0];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// ////			if (digitalRead(ROW_PIN_3) == LOW) {
// ////			  selectedChar = hexaKeys[2][0][0];
// ////			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  if (digitalRead(ROW_PIN_3) == LOW) {
// //////				selectedChar = hexaKeys[3][0][0];
// //////				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  }
// ////			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  4
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// //		if (digitalRead(ROW_PIN_2) == LOW) {
// //		  selectedChar = hexaKeys[0][1][0];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);

//     //   8 hexaKeys[0][2][0]
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// //		if (digitalRead(ROW_PIN_1) == LOW) {
// //		  selectedChar = hexaKeys[0][2][0];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeys[1][2][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeys[2][2][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[3][2][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//     //   @ hexaKeys[0][0][1]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// //		if (digitalRead(ROW_PIN_3) == LOW) {
// //		  selectedChar = hexaKeys[0][0][1];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeys[1][0][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeys[2][0][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_3) == LOW) {
// 				selectedChar = hexaKeys[3][0][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     5  hexaKeys[0][1][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// //		if (digitalRead(ROW_PIN_2) == LOW) {
// //		  selectedChar = hexaKeys[0][1][1];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     9  hexaKeys[0][2][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// //		if (digitalRead(ROW_PIN_1) == LOW) {
// //		  selectedChar = hexaKeys[0][2][1];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeys[1][2][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeys[2][2][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[3][2][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);





//     //      0  hexaKeys[0][0][2]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// //		if (digitalRead(ROW_PIN_3) == LOW) {
// //		  selectedChar = hexaKeys[0][0][2];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_3) == LOW) {
// //			selectedChar = hexaKeys[1][0][2];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			if (digitalRead(ROW_PIN_3) == LOW) {
// //			  selectedChar = hexaKeys[2][0][2];
// //			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_3) == LOW) {
// //				selectedChar = hexaKeys[3][0][2];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// //			}
// //		  }
// //		}
//       a = 'c'   ;
//     }
// 	delay(recoverDelay);



//     //     6  hexaKeys[0][1][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// //		if (digitalRead(ROW_PIN_2) == LOW) {
// //		  selectedChar = hexaKeys[0][1][2];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][2];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  Y=@  hexaKeys[0][2][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	while(digitalRead(ROW_PIN_1) == LOW){
// //		if (digitalRead(ROW_PIN_1) == LOW) {
// //			selectedChar = hexaKeys[0][2][2];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeys[1][2][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 				if (digitalRead(ROW_PIN_1) == LOW) {
// 					selectedChar = hexaKeys[2][2][2];
// 					writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					if (digitalRead(ROW_PIN_1) == LOW) {
// 						selectedChar = hexaKeys[3][2][2];
// 						writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					}
// 				}
// //			}
// 		}
// 		a = 'c';
// 	}
// 	delay(recoverDelay);




//     //  3  hexaKeys[0][0][3]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// //		if (digitalRead(ROW_PIN_3) == LOW) {
// //		  selectedChar = hexaKeys[0][0][3];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeys[1][0][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeys[2][0][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_3) == LOW) {
// //				selectedChar = hexaKeys[3][0][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  7  hexaKeys[0][1][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_2) == LOW){
// //		if (digitalRead(ROW_PIN_2) == LOW) {
// //		  selectedChar = hexaKeys[0][1][3];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeys[1][1][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeys[2][1][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeys[3][1][3];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// //		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


// 	//  N  hexaKeys[0][2][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeys[0][2][3];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_1) == LOW) {
// //			selectedChar = hexaKeys[1][2][3];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			if (digitalRead(ROW_PIN_1) == LOW) {
// //			  selectedChar = hexaKeys[2][2][3];
// //			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_1) == LOW) {
// //				selectedChar = hexaKeys[3][2][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// //			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//   }//WHILE ENDING
//   if(selectedChar == '@')
//   {
// //	  writeSelectedCharAndStringBlankingPlus(current_x, current_y);
//   }
//   else if(selectedChar == '~')
//   {
// 	  return selectedChar;
//   }
//   else if(selectedChar == '^')
//   {
// 	  return selectedChar;
//   }
//   else
//   {
// 	  writeSelectedCharAndStringBlanking(current_x, current_y, caps);
//   }
//   return selectedChar;

// }//ALPHKEYPAD_3A8C_NO_NUMBERS  ENDING


// char alphkeypad_3A8C_CAPS(int current_x, int current_y)
// {
// 	pinMode(ROW_PIN_3, INPUT_PULLUP);
// 	pinMode(ROW_PIN_2, INPUT_PULLUP);
// 	pinMode(ROW_PIN_1, INPUT_PULLUP);
// 	pinMode(COL_PIN_1, OUTPUT);
// 	pinMode(COL_PIN_2, OUTPUT);
// 	pinMode(COL_PIN_3, OUTPUT);
// 	pinMode(COL_PIN_4, OUTPUT);
// 	digitalWrite(COL_PIN_1, HIGH);
// 	digitalWrite(COL_PIN_2, HIGH);
// 	digitalWrite(COL_PIN_3, HIGH);
// 	digitalWrite(COL_PIN_4, HIGH);

// 	int recoverDelay = 1;
// 	int postDelay = 5;
// 	bool displayOrNot = true;
// 	char selectedChar;

// 	bool caps = true;

// 	char a = 'a';

//   while (a != 'c') {




//     //  @
//     digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][0][0];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_3) == LOW) {
// //			selectedChar = hexaKeys[1][0][0];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// ////			if (digitalRead(ROW_PIN_3) == LOW) {
// ////			  selectedChar = hexaKeys[2][0][0];
// ////			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  if (digitalRead(ROW_PIN_3) == LOW) {
// //////				selectedChar = hexaKeys[3][0][0];
// //////				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);       delay(keyDelay);
// //////			  }
// ////			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  4
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][1][0];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][1][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][1][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][1][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);

//     //   8 hexaKeys[0][2][0]
// //    digitalWrite(COL_PIN_1, LOW); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][2][0];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][2][0];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][2][0];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][2][0];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//     //   @ hexaKeys[0][0][1]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][0][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][0][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][0][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_3) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][0][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     5  hexaKeys[0][1][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][1][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][1][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][1][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][1][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //     9  hexaKeys[0][2][1]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, LOW);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][2][1];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][2][1];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][2][1];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][2][1];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);





//     //      9  hexaKeys[0][0][2]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][0][2];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][0][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][0][2];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_3) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][0][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c'   ;
//     }
// 	delay(recoverDelay);



//     //     6  hexaKeys[0][1][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][1][2];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][1][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][1][2];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][1][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  Y=@  hexaKeys[0][2][2]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, LOW); digitalWrite(COL_PIN_4, HIGH);
// 	while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 			selectedChar = hexaKeysCAPS[0][2][2];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_1) == LOW) {
// 				selectedChar = hexaKeysCAPS[1][2][2];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 				if (digitalRead(ROW_PIN_1) == LOW) {
// 					selectedChar = hexaKeysCAPS[2][2][2];
// 					writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					if (digitalRead(ROW_PIN_1) == LOW) {
// 						selectedChar = hexaKeysCAPS[3][2][2];
// 						writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 					}
// 				}
// 			}
// 		}
// 		a = 'c';
// 	}
// 	delay(recoverDelay);




//     //  3  hexaKeys[0][0][3]
//     digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
//     digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
// 	delay(postDelay);
//     while(digitalRead(ROW_PIN_3) == LOW){
// 		if (digitalRead(ROW_PIN_3) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][0][3];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_3) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][0][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_3) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][0][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_3) == LOW) {
// //				selectedChar = hexaKeysCAPS[3][0][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


//     //  7  hexaKeys[0][1][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_2) == LOW){
// 		if (digitalRead(ROW_PIN_2) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][1][3];
// 		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 		  if (digitalRead(ROW_PIN_2) == LOW) {
// 			selectedChar = hexaKeysCAPS[1][1][3];
// 			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			if (digitalRead(ROW_PIN_2) == LOW) {
// 			  selectedChar = hexaKeysCAPS[2][1][3];
// 			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  if (digitalRead(ROW_PIN_2) == LOW) {
// 				selectedChar = hexaKeysCAPS[3][1][3];
// 				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// 			  }
// 			}
// 		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);


// 	//  N  hexaKeys[0][2][3]
// //    digitalWrite(COL_PIN_1, HIGH); digitalWrite(COL_PIN_2, HIGH);
// //    digitalWrite(COL_PIN_3, HIGH); digitalWrite(COL_PIN_4, LOW);
//     while(digitalRead(ROW_PIN_1) == LOW){
// 		if (digitalRead(ROW_PIN_1) == LOW) {
// 		  selectedChar = hexaKeysCAPS[0][2][3];
// //		  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //		  if (digitalRead(ROW_PIN_1) == LOW) {
// //			selectedChar = hexaKeysCAPS[1][2][3];
// //			writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			if (digitalRead(ROW_PIN_1) == LOW) {
// //			  selectedChar = hexaKeysCAPS[2][2][3];
// //			  writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  if (digitalRead(ROW_PIN_1) == LOW) {
// //				selectedChar = hexaKeysCAPS[3][2][3];
// //				writeSelectedCharAndString(selectedChar, current_x, current_y, caps);         delay(keyDelay);
// //			  }
// //			}
// //		  }
// 		}
//       a = 'c';
//     }
// 	delay(recoverDelay);




//   }//WHILE ENDING
//   if(selectedChar == '@')
//   {
// //	  writeSelectedCharAndStringBlankingPlus(current_x, current_y);
//   }
//   else if(selectedChar == '~')
//   {
// 	  return selectedChar;
//   }
//   else
//   {
// 	  writeSelectedCharAndStringBlanking(current_x, current_y, caps);
//   }
//   return selectedChar;

// }//ALPHKEYPAD_3A8C_CAPS  ENDING

char alphkeypad_3A8CnoDisplay(int current_x, int current_y)
{
	int result;
	char temp1;

	result = getAndReturnInput();
	switch (result){
	case 0:
		temp1 = '0';
		break;
	case 1:
		temp1 = '1';
		break;
	case 2:
		temp1 = '2';
		break;
	case 3:
		temp1 = '3';
		break;
	case 4:
		temp1 = '4';
		break;
	case 5:
		temp1 = '5';
		break;
	case 6:
		temp1 = '6';
		break;
	case 7:
		temp1 = '7';
		break;
	case 8:
		temp1 = '8';
		break;
	case 9:
		temp1 = '9';
		break;
	case 10:
		temp1 = '~';
		break;
	case 11:
		temp1 = '@';
		break;
	}

	return temp1;


}//ALPHKEYPAD_3A8CNoDisplay  ENDING
