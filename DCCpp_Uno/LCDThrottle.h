#ifndef LCD_THROTTLE_H
#define LCD_THROTTLE_H

#include "LCD.h"
#include "Inglenook.h"

// DCC++ Throttle using the buttons on a 16x2 + 5 button Display Shield

#define DISPLAY_MODE_NORMAL   1
#define DISPLAY_MODE_SWITCHER 2
#define DISPLAY_MODE DISPLAY_MODE_NORMAL

// Throttle Directions
#define FORWARD 1
#define REVERSE 0

class MenuItem;

class LCDThrottle {
private:
  int throttleState;
int jumpbackState;
  int reg;
  int cab;
  int speed;
  int dir;
  byte displayMode;
  int notch;
  LCD *lcd;
  int maxSpeed;
  bool power_state;
  InglenookGame game;
  
 public:
  LCDThrottle();
  void begin(int reg);
  void run();

 protected:
  void sendThrottleCommand();
  void sendPowerCommand(bool on);
  int debounceButtons();
  int getButton();
  void increaseSpeed();
  void decreaseSpeed();
  void updateDisplay();
  void menuSetup();
  void doMenuDisplay();
  void doMenuAction(int button);
  void load();
  void store();
  void doGameMenus(int b);
  void setupInglenookMenu(MenuItem *m);
  void inglenookBegin();
  void doGameMenuDisplay(); 
  int calcIncValue(int button, int maxpos, int val, int maxval, const char *fmt, const char *label); 
  
};

struct LCDThrottleData {
byte dislay;
byte reserved1;
int  address;
int maxspeed;
int reserved[5];
};


#endif // LCD_THROTTLE_H
