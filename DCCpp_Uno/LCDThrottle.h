#ifndef LCD_THROTTLE_H
#define LCD_THROTTLE_H

#include "LCD.h"

// DCC++ Throttle using the buttons on a 16x2 + 5 button Display Shield

#define DISPLAY_MODE_NORMAL   1
#define DISPLAY_MODE_SWITCHER 2
#define DISPLAY_MODE DISPLAY_MODE_NORMAL

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
  LCD *lcd;
  bool power_state;
  
 public:
  static LCDThrottle *getThrottle(int r, int c);
  void run();

 protected:
  LCDThrottle(int reg, int cab);
  void sendThrottleCommand();
  void sendPowerCommand(bool on);
  int debounceButtons();
  int getButton();
  void increaseSpeed();
  void decreaseSpeed();
  void updateDisplay();
  
  
};

#endif // LCD_THROTTLE_H
