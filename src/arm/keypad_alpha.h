/*
 * keypad_alpha.h
 *
 *  Created on: Feb 5, 2015
 *      Author: thesquid
 */

#ifndef _KEYPAD_ALPHA_H_
#define _KEYPAD_ALPHA_H_

#define PIN_MAX_SIZE 20

#define STRIPE_X_START 5
#define STRIPE_Y_START 20
#define STRIPE_X_END 195
#define STRIPE_Y_END 20

#define caps_lock_X 182
#define caps_lock_Y 41

//#define COL_1_X 5
//#define LINE_1_Y 1
//#define LINE_2_Y 21
//#define LINE_3_Y 40
//#define LINE_4_Y 59
//#define LINE_5_Y 80

// Original spacing seems to die on the ST7789
// #define COL_1_X 5
// #define LINE_0_Y 1
// #define LINE_1_Y 21
// #define LINE_2_Y 40
// #define LINE_3_Y 59
// #define LINE_4_Y 80

#define COL_1_X 5
#define LINE_0_Y 4
#define LINE_1_Y 24
#define LINE_2_Y 43
#define LINE_3_Y 62
#define LINE_4_Y 83


#define STATUS_X 5
#define STATUS_Y 25

#define STATUS_X_1 5
#define STATUS_Y_1 40

#define INPUT_X 5
#define INPUT_Y 40

#define OUTPUT_X 75
#define OUTPUT_Y 55





#ifdef __cplusplus
     extern "C" {
#endif



void writeSelectedCharAndString(char currentChar, int current_x, int current_y, bool caps);
void writeSelectedCharAndStringBlanking(int current_x, int current_y, bool caps);
char alphkeypad(int current_x, int current_y);

#if defined(__SAM3A8C__)
char alphkeypad_3A8C(int current_x, int current_y);
char alphkeypad_3A8CnoDisplay(int current_x, int current_y);
char alphkeypad_3A8C_CAPS(int current_x, int current_y);
#endif

#if defined(NRF52840_XXAA)
char alphkeypad_3A8C(int current_x, int current_y);
char alphkeypad_3A8CnoDisplay(int current_x, int current_y);
char alphkeypad_3A8C_CAPS(int current_x, int current_y);
#endif


int fetchTransactionPINWrongCount(void);

char *getTransactionPINfromUser(void);
char *getInput(bool display, bool initialSetup);
char *getInputWallets(bool display, bool initialSetup);
char *getInputIndices(bool displayInput, bool initialSetup);
//int getInputIndicesInt(bool displayInput, bool initialSetup);
//char * getInputNoDisplay(void);
void pinStatusCheck(void);
void pinStatusCheckExpert(void);
void pinStatusCheckandPremadePIN(void);
void checkHashes(char * buffer, bool displayAlpha);
//void checkHashesNoDisplay(char * buffer);
void duressFormat(void);
void checkDevicePIN(bool displayAlpha);
char *mnemonic_input_stacker(int mlen);

bool doAEMValidate(bool displayAlpha);
void doAEMSet(void);

char *getInputAEM(bool displayInput, bool initialSetup);

char *mnemonic_input(void);
char alphkeypad_noNumbers_3A8C(int current_x, int current_y);
void writeSelectedCharAndStringBlankingMnemonics(int current_x, int current_y, bool caps);



#ifdef __cplusplus
     }
#endif


#endif /* _KEYPAD_ALPHA_H_ */
