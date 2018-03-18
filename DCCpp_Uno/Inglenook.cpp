#include <Arduino.h>
#include "LCD.h"
//#include <MenuBackend.h>

#include "Inglenook.h"

// Game State Machine States
#define STATE_IDLE   0
#define STATE_MENUS  1
#define STATE_ACTION 2
#define STATE_BUILD  3
int game_state = STATE_MENUS;

// Array holding the current sorting of the train to be built.
int train[TRAIN_LENGTH];

InglenookGame::InglenookGame() {
  ; // Do nothing
}

static void InglenookGame::begin() {
  randomSeed(analogRead(44));
  car_index = -1;
}

/*
void InglenookGame::printWelcome(LCD *lcd, char *row1, char *row2) {
  lcd->clear(); 
  sprintf(row1, "INGLENOOK GAME");
  sprintf(row2, "Select to start");
  lcd->updateDisplay(row1, row2); // TODO: Fix this.
}
*/

int InglenookGame::carIndex() {
  return(car_index);
}

void InglenookGame::setCarIndex(int c) {
  car_index = c;
}

void InglenookGame::buildTrain() {
  bool cars_used[NUM_CARS] = { false, false, false, false, false, false, false, false };
  int car = 0;
  bool found_one = false;
  game_state = STATE_BUILD;
  for (int i = 0; i < TRAIN_LENGTH; i++) {
    found_one = false;
    do {
      car = random(0, NUM_CARS) & 0xFFFF;
      //Serial.println("found " + String(car));
      if (cars_used[car] == false) {
        // Car is not used. Use it.
        cars_used[car] = true;
        train[i] = car;
        found_one = true;
        //Serial.println("using " + String(car));
      } // if
    } while (!found_one);
    //Serial.println("i = " + String(i) + " car = " + String(car)); 
  } // for
}

void InglenookGame::doDisplayTrain(LCD *lcd, char *row1, char *row2) {
  // DEBUG:
  //Serial.println("BUILD THIS TRAIN");
  //for (int i = 0; i < TRAIN_LENGTH; i++) {
    //Serial.print(String(i+1) + ": " + carnames[train[i]] + "; ");
  //}
  //Serial.println("");
  // END DEBUG
  lcd->clear();
  sprintf(row1, "BUILD THIS TRAIN");
  sprintf(row2, "");
  for (int i = 0; i < TRAIN_LENGTH; i++) {
    sprintf(row2, "%s%d ", row2, train[i]+1);
  }
  lcd->updateDisplay(row1, row2);
  lcd->noBlink();
}

void InglenookGame::doListTrain(LCD *lcd, char *row1, char *row2, int car) {
  String s1, s2;
  switch(car) {
    case 1:
      s1 = "2:" + carnames[train[1]];
      s2 = "3:" + carnames[train[2]];
      break;    
    case 2:
      s1 = "3:" + carnames[train[2]];
      s2 = "4:" + carnames[train[3]];
      break;    
    case 3:
      s1 = "4:" + carnames[train[3]];
      s2 = "5:" + carnames[train[4]];
      break;    
    case 0:
    default:
      s1 = "1:" + carnames[train[0]];
      s2 = "2:" + carnames[train[1]];
      break;    
  } // switch(car)
  lcd->clear();
  sprintf(row1, s1.c_str());
  sprintf(row2, s2.c_str());
  lcd->updateDisplay(row1, row2);
}

