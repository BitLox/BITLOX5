// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _KEYPAD_MPR121_H_
#define _KEYPAD_MPR121_H_



//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

// FUNCTION DECLARATIONS
void initKeypad(void);
void getInput121(void);
int getAndReturnInput(void);
void cleanI2C(void);
int getAndReturnInputPlusDisplay(int x, int y);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _KEYPAD_MPR121_H_ */
