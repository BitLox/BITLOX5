// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

/*
 * keypad.h
 *
 *  Created on: Feb 5, 2015
 *      Author: thesquid
 */

#ifndef KEYPAD_H
#define KEYPAD_H



//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

#define PIN_MAX_SIZE 20

#define STRIPE_X_START 5
#define STRIPE_Y_START 20
#define STRIPE_X_END 195
#define STRIPE_Y_END 20

#define caps_lock_X 182
#define caps_lock_Y 41


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



// Caps lock state
extern bool capsLock; // False = lowercase, True = uppercase


// FUNCTION DECLARATIONS
void initKeypad(void);
void getInput121(void);
int getAndReturnInput(void);
void cleanI2C(void);
int getAndReturnInputPlusDisplay(int x, int y);
char getAndReturnInputDisplay(int x, int y);


#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _KEYPAD_H_ */
