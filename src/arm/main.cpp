#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h> 

#include "main.h"
#include "ST7789.h"
#include "keypad_MPR121.h"
#include "../storage_common.h"
#include "../hwinterface.h"
#include "BLE.h"
#include "lcd_and_input.h"
#include "usart.h"
#include "keypad_alpha.h"
// #include "../stream_comm.h"


void Software_Reset(void) {
	//============================================================================================
	//   führt ein Reset des Arduino DUE aus...
	//
	//   Parameter: keine
	//   Rueckgabe: keine
	//============================================================================================
	// sd_nvic_SystemReset();	
	;;
}
	


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
	uint8_t temp2[1];

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
		temp1[0] = 1;
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

void useWhatCommsStealth(void)
{
	int result;
	int s;
	uint8_t temp1[1];
	// result = getAndReturnInput();
	result = 1;
	switch (result){
	case 1:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		// writeUSB_Screen();
		deactivateBLE();
		break;
	case 2:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 3:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 11:
		temp1[0] = 0;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		deactivateBLE();
		break;
	case 7:
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


	case 4:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 5:
		temp1[0] = 1;
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
	case 9:
		temp1[0] = 1;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
		activateBLE();
		break;
	case 0:
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

	default:
		s = 0;
		temp1[0] = s;
		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		break;
	}
}

void useWhatCommsDuress(void)
{
	int result;
	int s;
	uint8_t temp1[1];
	writeUSB_BLE_Screen();
	result = getAndReturnInput();
	result = 1;
	switch (result){
	case 1:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;
	case 2:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;
	case 3:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;
	case 10:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;
	case 7:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;
	case 8:
//		temp1[0] = 0;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
//		deactivateBLE();
		break;


	case 4:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;
	case 5:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;
	case 6:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;
	case 9:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;
	case 0:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;
	case 11:
//		temp1[0] = 1;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeBLE_Screen();
//		activateBLE();
		break;

	default:
//		s = 0;
//		temp1[0] = s;
//		nonVolatileWrite(temp1, DEVICE_COMMS_SET_ADDRESS, 1);
		writeUSB_Screen();
		break;
	}
showReady();
}


void showReady(void){
	tftBlackScreen();
	char p[] = "READY";
	drawtext(p, ST77XX_RED, 3, 0, 0);
	// writeSplashScreen();
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

void useWhatSetup(void)
{
Serial.println(" ---------in useWhatSetup----------");

	char rChar;
	bool yesOrNo;
	int r;
	int s;
	uint8_t temp1[1];

	inputInterjectionNoAck(ASKUSER_INITIAL_SETUP);

// 	rChar = waitForNumberButtonPress();

// 	clearDisplay();
// 	r = rChar - '0';
// 	switch (rChar){
// 	case '1':
// 		temp1[0] = 0;

// 		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_STANDARD_SETUP);

// 		yesOrNo = waitForButtonPress();
// 		clearDisplay();
// 		if(!yesOrNo)
// 		{
// 			pinStatusCheckandPremadePIN();
// 		}else if(yesOrNo)
// 		{
// 			useWhatSetup();
// 			break;
// 		}
// 		level = 1;
// 		break;

// 	case '2':
// 		temp1[0] = 0;

// 		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_ADVANCED_SETUP);

// 		yesOrNo = waitForButtonPress();

// 		clearDisplay();

// 		if(!yesOrNo)
// 		{
// 			pinStatusCheck();
// 		}else if(yesOrNo)
// 		{
// 			useWhatSetup();
// 			break;
// 		}
// 		level = 2;

// 		doAEMSet();

// 		break;
// 	case '3':
// 		temp1[0] = 0;

// 		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_EXPERT_SETUP);

// 		yesOrNo = waitForButtonPress();
// 		clearDisplay();

// 		if(!yesOrNo)
// 		{
// 			pinStatusCheckExpert();
// 		}else if(yesOrNo)
// 		{
// 			useWhatSetup();
// 			break;
// 		}
// 		level = 3;

// 		doAEMSet();

// 		break;
// 	case 'Y':
// 		temp1[0] = 0;

// 		buttonInterjectionNoAckSetup(ASKUSER_DESCRIBE_STANDARD_SETUP);

// 		yesOrNo = waitForButtonPress();
// 		clearDisplay();

// 		if(!yesOrNo)
// 		{
// 			pinStatusCheckandPremadePIN();
// 		}else if(yesOrNo)
// 		{
// 			useWhatSetup();
// 			break;
// 		}
// 		level = 1;

// 		break;


// 	case 'N':
// 		temp1[0] = 1;
// 		writeX_Screen();

// 		#if defined(__MSP430_CPU__) || defined(__SAM3X8E__)|| defined(__SAM3A8C__)
// 		Software_Reset();
// 		#endif

// 		#if defined(NRF52840_XXAA)
// 		;
// 		#endif

// 		break;

// 	case '0':
// 		writeEinkDisplay("READY FOR RESTORE...", false, COL_1_X, LINE_0_Y, "MAY TAKE UP TO 2 MINUTES",false,COL_1_X,LINE_1_Y, "TO DECRYPT AND WRITE",false,COL_1_X,LINE_2_Y, "WALLET DATA",false,COL_1_X,LINE_3_Y, "",false,0,0);
// 		useWhatCommsStealth();
// 		initUsart();
// 		loop();
// 		break;

// 	default:
// 		useWhatSetup();
// 		break;
// 	}
}




void setupSequence(int level){
// 	bool canceledWalletCreation;
// 	int strength;
// 	if(level == 1)
// 	{
// 		strength = 128;
// 		initialFormatAuto();
// 		canceledWalletCreation = createDefaultWalletAuto(strength, level);
// 		useWhatComms();
// 		initUsart();
// 		if(!canceledWalletCreation)
// 		{
// 			showQRcode(0,0,0);
// 		}else{
// 			showReady();
// 		}

// 	}
// 	else if(level == 2)
// 	{
// 		strength = 192;
// 		initialFormatAuto();
// 		canceledWalletCreation = createDefaultWalletAuto(strength, level);
// 		useWhatComms();
// 		initUsart();

// 		if(!canceledWalletCreation)
// 		{
// 			showQRcode(0,0,0);
// 		}else{
// 			showReady();
// 		}
// 	}
// 	else if(level == 3)
// 	{
// 		strength = 256;
// 		initialFormatAuto();
// 		canceledWalletCreation = createDefaultWalletAuto(strength, level);
// 		useWhatComms();
// 		initUsart();

// 		if(!canceledWalletCreation)
// 		{
// 			showQRcode(0,0,0);
// 		}else{
// 			showReady();
// 		}
// 	}
}

void setup()
{
	int level;
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

	// tftBlackScreen();
	// char p[] = "INPUT PIN:";
	// drawtext(p, ST77XX_RED, 3, 0, 0);
	// delay(1000);

	initKeypad();
	languageMenuInitially();
	
	tftBlackScreen();
	char p[] = "after languageMenuInitially()";
	drawtext(p, ST77XX_RED, 2, 0, 0);
	delay(1000);

	initFormatting();

	// int AemStatus;
	// AemStatus = checkUseAEM();
	// Serial.print(AemStatus);
	// Serial.println(" ---------checkUseAEM----------");
	// if(AemStatus != 127)
	// {
	// 	;
	// }
	// else if (AemStatus == 127)
	// {
	// 	doAEMValidate(false);
	// }

	int pinStatus;
	pinStatus = checkHasPIN();
	if(pinStatus != 127)
	{
		useWhatSetup();
	}
	else if (pinStatus == 127)
	{
		checkDevicePIN(false);
	}

	useWhatCommsStealth();
	// initUsart();

	if(is_formatted != 123)
	{
		setupSequence(level);
	}
	else
	{
		useWhatComms();
		// initUsart();
		showReady();
	}


}

void loop() {
	// processPacket();

  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(100);                       // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  // delay(100);                       // wait for a second
}

