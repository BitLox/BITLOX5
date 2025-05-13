// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef KEYPAD_MPR121_H
#define KEYPAD_MPR121_H



//end of add your includes here
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


#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _KEYPAD_MPR121_H_ */
