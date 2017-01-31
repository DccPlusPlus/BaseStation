#include <Arduino.h>
#include <MenuBackend.h>

#include "Inglenook.h"

// Game State Machine States
#define STATE_IDLE   0
#define STATE_MENUS  1
#define STATE_ACTION 2
#define STATE_BUILD  3
int game_state = STATE_MENUS;

// Index used for displaying the cars in the Build Train state.
int car_index;

// Array holding the current sorting of the train to be built.
int train[TRAIN_LENGTH];

// Menu subsystem "stuff"
void menuUseEvent(MenuUseEvent e);
void menuChangeEvent(MenuChangeEvent e);
MenuBackend *menu = new MenuBackend(menuUseEvent, menuChangeEvent);

static char display[2][17];

static InglenookGame *thegame = NULL;


static InglenookGame *InglenookGame::getTheGame() {
  if (thegame == NULL) {
    thegame = new InglenookGame();
  }
  return(thegame);
}

InglenookGame::InglenookGame() {
  lcd = new LCD();
  menuSetup();
}

void InglenookGame::menuSetup() {
  //Serial.println("Setting up menu...");
  MenuItem *miBuild = new MenuItem("Build Train");
  MenuItem *miList = new MenuItem("List Cars");
  MenuItem *miCarList[7] = {
    new MenuItem("Car 0"),
    new MenuItem("Car 1"),
    new MenuItem("Car 2"),
    new MenuItem("Car 3"),
    new MenuItem("Car 4"),
    new MenuItem("Car 5"),
    new MenuItem("Car 6")
  };
  menu->getRoot().add(*miBuild);
  miBuild->add(*miList);
  miList->addRight(*miCarList[0]);
  for (int i = 0; i < NUM_CARS-2; i++) {
    miCarList[i]->add(*miCarList[i+1]);
  }
}

static void InglenookGame::begin() {
  randomSeed(analogRead(44));
  menuSetup();
  lcd->begin();
  printWelcome();
  game_state = STATE_IDLE;
  car_index = -1;
}

void InglenookGame::printWelcome() {
  lcd->clear(); 
  sprintf(display[0], "INGLENOOK GAME");
  sprintf(display[1], "Select to start");
  lcd->updateDisplay(display[0], display[1]);
}

void InglenookGame::play() {
    // Read and respond to the buttons
  //int buttons = checkButtons();
  lcd->run();
  int buttons = lcd->getButtons();
  switch(game_state) {
  case STATE_IDLE:
    if (buttons == KEYS_SELECT) {
      //Serial.println("Starting game...");
      game_state = STATE_MENUS;
      menu->moveDown();
      doMenuDisplay();
    }
    break;

    
  case STATE_MENUS:
    switch(buttons) {
    case KEYS_UP:
      if (menu->getCurrent() == "Build Trains") {
        game_state == STATE_IDLE;
        printWelcome();
      } else {
        menu->moveUp();
        doMenuDisplay();
      }
      break;
    case KEYS_DOWN:
      menu->moveDown();
      doMenuDisplay();
      break;
    case KEYS_LEFT:
      if (menu->getCurrent() == "Build Trains") {
        game_state == STATE_IDLE;
        printWelcome();
      } else {
        menu->moveLeft();
        doMenuDisplay();
      }
      break;
    case KEYS_RIGHT:
      menu->moveRight();
      doMenuDisplay();
      break;
    case KEYS_SELECT:
      if (menu->getCurrent() == "List Cars") {
        menu->moveRight(); // for this one "use" == "move right"
        doMenuDisplay();
      } else {
        menu->use();
      }
      break;
    // Add other combinations here. 
    } // switch(buttons)
    break;

  case STATE_BUILD:
    switch(buttons) {
      case KEYS_NONE:
        break;
      case KEYS_LEFT:
      case KEYS_SELECT:
        game_state = STATE_MENUS;
        doMenuDisplay();
        break;
      case KEYS_DOWN:
      case KEYS_RIGHT:
        if (car_index >= 3) {
          car_index = 3;
        } else {
          car_index++;
        }
        doListTrain(car_index);
        break;
      case KEYS_UP:
        if (car_index > -1) {
          car_index--;
        }
        if (car_index == -1) {
          doDisplayTrain();
        } else {
          doListTrain(car_index);
        }
        break;
    } // switch(buttons)
  } // switch(game_state)
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

void InglenookGame::updateDisplay(char *row1, char *row2) {
  sprintf(display[0], row1);
  sprintf(display[1], row2);
  lcd->updateDisplay(display[0], display[1]);
}

void InglenookGame::doMenuDisplay() {
  lcd->clear();
  
  if (menu->getCurrent() == "Build Train") {
    sprintf(display[0], "Build Train");
    sprintf(display[1], "List Cars");
  }
  if (menu->getCurrent() == "List Cars") {
    sprintf(display[0], "List Cars");
    sprintf(display[1], "");
  }
  if (menu->getCurrent() == "Car 0") {
    sprintf(display[0],"%s%s", "1:",carnames[0].c_str());
    sprintf(display[1],"%s%s", "2:",carnames[1].c_str());
  }
  if (menu->getCurrent() == "Car 1") {
    sprintf(display[0],"%s%s", "2:",carnames[1].c_str());
    sprintf(display[1],"%s%s", "3:",carnames[2].c_str());
  }
  if (menu->getCurrent() == "Car 2") {
    sprintf(display[0],"%s%s", "3:",carnames[2].c_str());
    sprintf(display[1],"%s%s", "4:",carnames[3].c_str());
  }
  if (menu->getCurrent() == "Car 3") {
    sprintf(display[0],"%s%s", "4:",carnames[3].c_str());
    sprintf(display[1],"%s%s", "5:",carnames[4].c_str());
  }
  if (menu->getCurrent() == "Car 4") {
    sprintf(display[0],"%s%s", "5:",carnames[4].c_str());
    sprintf(display[1],"%s%s", "6:",carnames[5].c_str());
  }
  if (menu->getCurrent() == "Car 5") {
    sprintf(display[0],"%s%s", "6:",carnames[5].c_str());
    sprintf(display[1],"%s%s", "7:",carnames[6].c_str());
  }
  if ((menu->getCurrent() == "Car 6") || (menu->getCurrent() == "Car 7")) {
    sprintf(display[0],"%s%s", "7:",carnames[6].c_str());
    sprintf(display[1],"%s%s", "8:",carnames[7].c_str());
  }
  lcd->updateDisplay(display[0], display[1]);
  lcd->setCursor(0,0);
  lcd->blink();
}

void InglenookGame::doDisplayTrain() {
  // DEBUG:
  //Serial.println("BUILD THIS TRAIN");
  //for (int i = 0; i < TRAIN_LENGTH; i++) {
    //Serial.print(String(i+1) + ": " + carnames[train[i]] + "; ");
  //}
  //Serial.println("");
  // END DEBUG
  lcd->clear();
  sprintf(display[0], "BUILD THIS TRAIN");
  sprintf(display[1], "");
  for (int i = 0; i < TRAIN_LENGTH; i++) {
    sprintf(display[1], "%s%d ", display[1], train[i]+1);
  }
  updateDisplay(display[0], display[1]);
  lcd->noBlink();
}

void InglenookGame::doListTrain(int car) {
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
  sprintf(display[0], s1.c_str());
  sprintf(display[1], s2.c_str());
  lcd->updateDisplay(display[0], display[1]);
}

void menuChangeEvent(MenuChangeEvent changed) {
  // Nothing to do here...
}

void menuUseEvent(MenuUseEvent used) {
  InglenookGame *thegame = InglenookGame::getTheGame();
  //Serial.print("Menu use ");
  //Serial.println(used.item.getName());
  if (used.item == "Build Train") {
    //Serial.println("Building Train...");
    thegame->buildTrain();
    thegame->doDisplayTrain();
    /*
    for (int i = 0; i < TRAIN_LENGTH; i++) {
      Serial.println("Car " + String(i) + ": " + carnames[train[i]]);
    }
    */
  }
  if (used.item == "List Cars") {
    //Serial.println("List Cars...");
    for (int i = 0; i < NUM_CARS; i++) {
      //Serial.println("Car " + String(i) + ": " + carnames[i]);
    }
  }
}


