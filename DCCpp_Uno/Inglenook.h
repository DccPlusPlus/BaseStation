#ifndef INGLENOOK_H
#define INGLENOOK_H

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
  int car_index;
  
 public:
  InglenookGame(); 
  void begin();
  void buildTrain(void);
  void doDisplayTrain(LCD *lcd, char *row1, char *row2);
  int carIndex();
  void setCarIndex(int c);
  void doMenuDisplay(LCD *lcd, char *row1, char *row2);
  void doListTrain(LCD *lcd, char *row1, char *row2, int car);

 protected:
  //void printWelcome(LCD *lcd, char *row1, char *row2);
};


#endif
