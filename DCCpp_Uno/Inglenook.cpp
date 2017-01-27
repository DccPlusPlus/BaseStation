#include <Arduino.h>
#include <MenuBackend.h>
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
#include <Adafruit_RGBLCDShield.h>
#include <utility/AdafruitMCP23017.h> // is this necessary?
#else
#error CANNOT COMPILE -- INVALID LCD LIBRARY SELECTED
#endif

#include "Inglenook.h"

// Define generic button names for multi-library compatibility
#define KEYS_NONE -1
#define KEYS_RIGHT 0
#define KEYS_UP 1
#define KEYS_DOWN 2
#define KEYS_LEFT 3
#define KEYS_SELECT 4

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

#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
static LCDKeypad lcd = LCDKeypad();
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
static Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

static InglenookGame *thegame = NULL;


static InglenookGame *InglenookGame::getTheGame() {
  if (thegame == NULL) {
    thegame = new InglenookGame();
  }
  return(thegame);
}

InglenookGame::InglenookGame() {
  //lcd = LCDKeypad();
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
  lcd.begin(16, 2);
  //Serial.println("lcd begun");
  printWelcome();
  game_state = STATE_IDLE;
  car_index = -1;
}

void InglenookGame::printWelcome() {
  lcd.clear(); 
  //Serial.println("lcd clear");
  lcd.setCursor(0, 0);
  //Serial.println("lcd setcursor");
  lcd.print("INGLENOOK GAME");
  lcd.setCursor(0,1);
  lcd.print("Select to start");
}

void InglenookGame::play() {
    // Read and respond to the buttons
  int buttons = checkButtons();
  switch(game_state) {
  case STATE_IDLE:
    if (buttons == KEYS_SELECT) {
      //Serial.println("Starting game...");
      game_state = STATE_MENUS;
      menu->moveDown();
      //menu.moveDown();
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
      //menu.moveBack(); 
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
        //menu.moveUp();
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


int InglenookGame::checkButtons() {
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
  // OSEPP LCDKeypad
  int buttons = lcd.button();
  if (buttons != -1) {
    debounceButton(buttons);
    //Serial.print("Button Check = ");
    //Serial.println(buttons);
    return(buttons);
  } else {
    return(KEYS_NONE);
  }
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
  // ADAFRUIT RGBLCD
  uint8_t buttons = lcd.readButtons();
  if (buttons & BUTTON_UP) {
    return(KEYS_UP);
  }
  if (buttons & BUTTON_DOWN) {
    return(KEYS_DOWN);
  }
  if (buttons & BUTTON_LEFT) {
    return(KEYS_LEFT);
  }
  if (buttons & BUTTON_RIGHT) {
    return(KEYS_RIGHT);
  }
  if (buttons & BUTTON_SELECT) {
    return(KEYS_SELECT);
  }
  return(KEYS_NONE);
#endif
}

int InglenookGame::debounceButton(int button) {
  if (button != KEYS_NONE) {
    while(lcd.button() != KEYS_NONE) {
      ;
    }
  }
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

void InglenookGame::doMenuDisplay() {
  lcd.clear();
  if (menu->getCurrent() == "Build Train") {
    lcd.setCursor(0,1); lcd.print("List Cars");
    lcd.setCursor(0,0); lcd.print("Build Train");
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "List Cars") {
    lcd.setCursor(0,1); lcd.print("");
    lcd.setCursor(0,0); lcd.print("List Cars");
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 0") {
    lcd.setCursor(0,0); lcd.print("1:" + carnames[0]);
    lcd.setCursor(0,1); lcd.print("2:" + carnames[1]);
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 1") {
    lcd.setCursor(0,0); lcd.print("2:" + carnames[1]);
    lcd.setCursor(0,1); lcd.print("3:" + carnames[2]);
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 2") {
    lcd.setCursor(0,0); lcd.print("3:" + carnames[2]);
    lcd.setCursor(0,1); lcd.print("4:" + carnames[3]);
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 3") {
    lcd.setCursor(0,0); lcd.print("4:" + carnames[3]);
    lcd.setCursor(0,1); lcd.print("5:" + carnames[4]);
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 4") {
    lcd.setCursor(0,0); lcd.print("5:" + carnames[4]);
    lcd.setCursor(0,1); lcd.print("6:" + carnames[5]);
    lcd.setCursor(0,0);
  }
  if (menu->getCurrent() == "Car 5") {
    lcd.setCursor(0,0); lcd.print("6:" + carnames[5]);
    lcd.setCursor(0,1); lcd.print("7:" + carnames[6]);
    lcd.setCursor(0,0);
  }
  if ((menu->getCurrent() == "Car 6") || (menu->getCurrent() == "Car 7")) {
    lcd.setCursor(0,0); lcd.print("7:" + carnames[6]);
    lcd.setCursor(0,1); lcd.print("8:" + carnames[7]);
    lcd.setCursor(0,0);
  }
  lcd.setCursor(0,0);
  lcd.blink();
}

void InglenookGame::doDisplayTrain() {
  // DEBUG:
  //Serial.println("BUILD THIS TRAIN");
  for (int i = 0; i < TRAIN_LENGTH; i++) {
    //Serial.print(String(i) + ": " + carnames[train[i]] + "; ");
  }
  //Serial.println("");
  // END DEBUG
  lcd.clear();
  //lcd.autoscroll();
  lcd.setCursor(0,0);
  lcd.print("BUILD THIS TRAIN");
  lcd.setCursor(0,1);
  for (int i = 0; i < TRAIN_LENGTH; i++) {
    lcd.print(String(train[i]) + " ");
  }
  lcd.noBlink();
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
}

void menuChangeEvent(MenuChangeEvent changed) {
  // Update the display to reflect the current menu state
  // For now we'll use serial output.
  //Serial.print("Menu change ");
  //Serial.print(changed.from.getName());
  //Serial.print(" -> ");
  //Serial.println(changed.to.getName());
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


