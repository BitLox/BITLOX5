// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _MAIN_H_
#define _MAIN_H_

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

// FUNCTION DECLARATIONS
void loop(void);
void setup(void);
void useWhatComms(void);

void writeUSB_BLE_Screen(void);
void writeUSB_Screen(void);
void writeBLE_Screen(void);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _ARM_Lockbox_1_H_ */
