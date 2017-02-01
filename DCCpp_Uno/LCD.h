#ifndef LCD_H
#define LCD_H

//------------------------------------------------
// Generic wrapper class for various different
// 16x2 LCD shields with buttons..  Namely:
// * the 4/8-pin OSEPP model
// * the Adafruit I2C version
//
// Allows for long vs. short presses of the buttons
// (but not momentary action -- yet)

// LCD Display Types:
//
//  0 = OSEPP LCDKeypad
//  1 = Adafruit RGB LCD  
#define LCD_DISPLAY_TYPE_OSEPP    0
#define LCD_DISPLAY_TYPE_ADAFRUIT 1
#define LCD_DISPLAY_TYPE LCD_DISPLAY_TYPE_ADAFRUIT

// Defines how long a "long press" is in milliseconds.
#define LONG_PRESS_MS 2000

// Define generic button names for multi-library compatibility
// Return values from getButtons();
#define KEYS_NONE  -1
#define KEYS_RIGHT  0
#define KEYS_UP     1
#define KEYS_DOWN   2
#define KEYS_LEFT   3
#define KEYS_SELECT 4
#define KEYS_LONG_RIGHT  128
#define KEYS_LONG_UP     129
#define KEYS_LONG_DOWN   130
#define KEYS_LONG_LEFT   131
#define KEYS_LONG_SELECT 132


class LCD {
 private:
  byte displayState;
  int buttonVal;
 public:
  LCD();
  void begin();  // Call from setup()
  void run();    // Call from loop()
  int getButtons();
  void updateDisplay(char *row1, char *row2);
  void clear();
  void setCursor(int c, int r);
  void blink();
  void noBlink();
  void cursor();
  void noCursor();
 protected:
  int getButton();
  void debounceButtons();
  
};

#endif // LCD_H
