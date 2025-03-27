// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ST7789_H_
#define _ST7789_H_

//add your includes here
#define PIN_MAX_SIZE 20

#define STRIPE_X_START 5
#define STRIPE_Y_START 17
#define STRIPE_X_END 195
#define STRIPE_Y_END 17

#define caps_lock_X 182
#define caps_lock_Y 41

#define COL_1_X 5
#define LINE_0_Y 1
#define LINE_1_Y 21
#define LINE_2_Y 40
#define LINE_3_Y 59
#define LINE_4_Y 80

#define STATUS_X 5
#define STATUS_Y 25

#define STATUS_X_1 5
#define STATUS_Y_1 40

#define INPUT_X 5
#define INPUT_Y 40

#define OUTPUT_X 75
#define OUTPUT_Y 55


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

// FUNCTION DECLARATIONS
void testlines(uint16_t color);
void testdrawtext(char *text, uint16_t color);
void testfastlines(uint16_t color1, uint16_t color2);
void testdrawrects(uint16_t color);
void testfillrects(uint16_t color1, uint16_t color2);
void testfillcircles(uint8_t radius, uint16_t color);
void testdrawcircles(uint8_t radius, uint16_t color);
void testtriangles();
void testroundrects();
void tftPrintTest();
void mediabuttons();
void initDisplay(void);
void tftBlackScreen(void);
void drawtext(char *text, uint16_t color, int size, int x, int y);
void drawtextW2(char *text, int x, int y);
void drawtextW3(char *text, int x, int y);
void drawtextW5(const char *text, int x, int y);
// void drawCheck(int x, int y);
void drawLine(int x, int y, int dx, int dy);
void fillRectangle(int x, int y, int dx, int dy);
void drawDot(int pX, int pY, uint16_t color);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _ST7789_H_ */


