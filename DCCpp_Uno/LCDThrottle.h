#ifndef LCD_THROTTLE_H
#define LCD_THROTTLE_H

// DCC++ Throttle using the buttons on a 16x2 + 5 button Display Shield

// LCD Display Types:
//
//  0 = OSEPP LCDKeypad
//  1 = Adafruit RGB LCD  
#define LCD_DISPLAY_TYPE_OSEPP    0
#define LCD_DISPLAY_TYPE_ADAFRUIT 1
#define LCD_DISPLAY_TYPE LCD_DISPLAY_TYPE_OSEPP

#define DISPLAY_MODE_NORMAL   1
#define DISPLAY_MODE_SWITCHER 2
#define DISPLAY_MODE DISPLAY_MODE_SWITCHER

// Throttle Directions
#define FORWARD 1
#define REVERSE 0

class LCDThrottle {
 private:
  int throttleState; 
  int reg;
  int cab;
  int speed;
  int dir;
  int displayMode;
  int notch;
  char **display;

 public:
  static LCDThrottle *getThrottle(int r, int c, char** d);
  //void begin(LCDKeypad *lcd);
  void run();

 protected:
  LCDThrottle(int reg, int cab);
  void sendThrottleCommand();
  //int checkButtons();
  int debounceButtons();
  int getButton();
  void increaseSpeed();
  void decreaseSpeed();
  void updateDisplay();
  
};

#endif // LCD_THROTTLE_H
