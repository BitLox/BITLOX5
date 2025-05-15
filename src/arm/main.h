// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _MAIN_H_
#define _MAIN_H_

// #define DEBUG_MODE

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

// definitions
#define TFT_CS         10
#define TFT_RST        11 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         6

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
