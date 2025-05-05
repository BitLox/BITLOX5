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
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include "main.h"
#include "keypad_MPR121.h"
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

// Caps lock state
// bool capsLock = false; // False = lowercase, True = uppercase


// #define DEBUG_MODE

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


void initKeypad(void){
    // Serial.begin(9600);

    // while (!Serial)
    // { // needed to keep leonardo/micro from starting too fast!
    //     delay(10);
    // }

    // Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
    if (!cap.begin(0x5A))
    {
        #ifdef DEBUG_MODE
        Serial.println("MPR121 not found, check wiring?");
        #endif
        // while (1)
        //     ;
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
