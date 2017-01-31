#ifndef LCD_H
#define LCD_H

// LCD Display Types:
//
//  0 = OSEPP LCDKeypad
//  1 = Adafruit RGB LCD  
#define LCD_DISPLAY_TYPE_OSEPP    0
#define LCD_DISPLAY_TYPE_ADAFRUIT 1
#define LCD_DISPLAY_TYPE LCD_DISPLAY_TYPE_ADAFRUIT


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
  int displayState;
 public:
  int buttonVal;
  LCD();
  void begin();
  void run();
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
  int debounceButtons();
  
};

#endif // LCD_H
