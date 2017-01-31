#ifndef INGLENOOK_H
#define INGLENOOK_H

#include "LCD.h"

// NOTE: These are actually set by the rules of the game...
// WARNING: There are several places (initialization, etc.) where
// the value of these is assumed fixed at 5 and 8.
#define TRAIN_LENGTH 5
#define NUM_CARS 8

const String carnames[NUM_CARS] = { // String names of cars on the layout (for display)
  "Blue Boxcar",
  "Red Boxcar",
  "Green Boxcar",
  "Black Tank Car",
  "Black Gondola",
  "Grey Hopper",
  "Brown Flatcar",
  "Brown Boxcar"
};

class InglenookGame {
 private:
  LCD *lcd;
  
 public:
  static InglenookGame *getTheGame();
  void begin();
  void play();
  void buildTrain(void);
  void doDisplayTrain();

 protected:
  InglenookGame(); 
  void menuSetup();
  void printWelcome();
  int checkButtons();
  int debounceButton(int button);
  void doMenuDisplay();
  void doListTrain(int car);
  void updateDisplay(char *row1, char *row2);
  
};


#endif
