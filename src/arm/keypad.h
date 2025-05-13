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

#ifdef __cplusplus
extern "C" {
#endif

// Caps lock state
extern bool capsLock; // False = lowercase, True = uppercase

// FUNCTION DECLARATIONS
void initKeypad(void);
void getInput121(void);
int getAndReturnInput(void);
void cleanI2C(void);
int getAndReturnInputPlusDisplay(int x, int y);
char getAndReturnInputDisplay(int x, int y);

void writeSelectedCharAndString(char currentChar, int current_x, int current_y, bool caps);
void writeSelectedCharAndStringBlanking(int current_x, int current_y, bool caps);
char alphkeypad(int current_x, int current_y);

// char alphkeypad_3A8C(int current_x, int current_y);
char alphkeypad_3A8CnoDisplay(int current_x, int current_y);
// char alphkeypad_3A8C_CAPS(int current_x, int current_y);


int fetchTransactionPINWrongCount(void);

char *getTransactionPINfromUser(void);
char *getInput(bool display, bool initialSetup);
char *getInputWallets(bool display, bool initialSetup);
char *getInputIndices(bool displayInput, bool initialSetup);
void pinStatusCheck(void);
void pinStatusCheckExpert(void);
void pinStatusCheckandPremadePIN(void);
void checkHashes(char * buffer, bool displayAlpha);
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
} // extern "C"
#endif

//Do not add code below this line
#endif /* _KEYPAD_H_ */
