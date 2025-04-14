// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _MAIN_H_
#define _MAIN_H_

#define DEBUG_MODE

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif


// FUNCTION DECLARATIONS
void loop(void);
void setup(void);
void useWhatComms(void);
void useWhatCommsStealth(void);
void useWhatCommsDuress(void);
void showReady(void);
void writeUSB_BLE_Screen(void);
void writeUSB_Screen(void);
void writeBLE_Screen(void);
void setupSequence(int level);
void useWhatSetup(void);
void fatalError(void);
void notify1(void);
void notify2(void);
void notify3(void);
void notify4(void);
void notify5(void);
void HardFault_Handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _MAIN_H_ */
