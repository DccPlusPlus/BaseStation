#include <Arduino.h>
#include <EEPROM.h>
#include <MenuBackend.h>
#include "SerialCommand.h"
#include "EEStore.h"
#include "LCDThrottle.h"
#include "Inglenook.h"

//--------------------------------------------------------------------
/* LCD Throttle
 *
 * Uses an LCD with buttons to provide a directly-connected throttle
 * interface for DCC++.  Includes an "Inglenook Sidings" game that will
 * randomize a set of cars and tell you a 5-car train to build.
 *
 * Boots up in the Menu state.  Menu options include:
 *   -- Use Throttle
 *   -- Set Track Power On/Off
 *   -- Set Address
 *   -- Set Display Mode
 *   -- Set maximum speed (in 128 speed steps)
 *   -- Play Inglenook Sidings
 *
 * A long (2-sec+) press on SELECT will toggle between Menu and Throttle view.
 *
 * Throttle views (Display Modes):
 *   -- Standard:  Separate Direction and Speed indicators
 *   -- Switcher:  Combined bidirectional Direction+Speed indicator
 *
 * Each click of LEFT/RIGHT will increase/decrease the speed by a fraction
 * of the maximum speed set in the menus.  The amount depends on the view mode.
 * Standard view has 15 steps, so (e.g.) with max speed = 60, each click is
 * 4 speed steps.
 * Switcher view has +/- 7 steps, so (e.g.) with max speed = 63, each click is
 * 9 speed steps.
 *
 * KNOWN BUGS:
 *   -- The "spinners" for the highest digit of the Set Address and Set Max Speed
 *      menu items don't work right.
 *   -- Some of the menu responses to navigation buttons aren't consistent
 */
//--------------------------------------------------------------------

#define MAX_SPEED           126 /*126 */
#define MAX_NOTCH_NORMAL     15
#define MAX_NOTCH_SWITCHER    7
#define DEFAULT_CAB           3

#define THROTTLE_STATE_RUN         0
#define THROTTLE_STATE_DEBOUNCE    1
#define THROTTLE_STATE_MENUS       2
#define THROTTLE_STATE_MENU_ACTION 3
#define THROTTLE_STATE_GAME_MENUS  4
#define THROTTLE_STATE_GAME_BUILD  5

struct LCDThrottleData LCDT_EEPROM_Store;

// MAX_COMMAND_LENGTH is defined in DCC++ SerialCommand.h
//#define MAX_COMMAND_LENGTH 30
// Buffer for writing DCC++ commands to the core base station code.
char command[MAX_COMMAND_LENGTH];

// Holder for display string construction
static char display[2][17];

// Menu subsystem "stuff"
static void menuUseEvent(MenuUseEvent e);
static void menuChangeEvent(MenuChangeEvent e);
static MenuBackend *menu;

/** Constructor
 *
 */
LCDThrottle::LCDThrottle() {
  ; // do nothing
}


void LCDThrottle::begin(int reg) {
  lcd = new LCD();
  lcd->begin();
  //EEPROM_GetAll();
  load();
  if (maxSpeed == 0) {
    // EEPROM not yet initialized -> set default.
    maxSpeed = MAX_SPEED;
  }
  if (cab == 0) {
    // EEPROM not yet initialized -> set default.
    cab = DEFAULT_CAB;
  }
  throttleState = THROTTLE_STATE_MENUS; // Always start in the menus.
  jumpbackState = THROTTLE_STATE_MENUS;
  this->reg = reg; // Store the register we should use.
  notch = 0; // Idle the loco
  speed = 0;
  dir = FORWARD;
  // TODO: Get the actual track power state from the base station.
  power_state = false; // Assume track power is off.
  sendPowerCommand(power_state); // Don't assume. Force it off.
  // Fire up the Inglenook game code.
  game = InglenookGame();
  game.begin();
  // Construct the menus.
  menu = new MenuBackend(menuUseEvent, menuChangeEvent);
  menuSetup();
  doMenuDisplay();
}

/** run()
 * 
 * Main Run loop
 */
void LCDThrottle::run() {
  // Run the underlying LCD stuff
  lcd->run();

  // Grab any button presses and process them
  int button = lcd->getButtons();
  
  switch(throttleState) {

  case THROTTLE_STATE_RUN:
    switch(button) {
    case KEYS_RIGHT:
      // Speed up
      increaseSpeed();
      sendThrottleCommand();
      updateDisplay();
      break;
      
    case KEYS_LEFT:
      // Slow down
      decreaseSpeed();
      sendThrottleCommand();
      updateDisplay();
      break;
      
    case KEYS_UP:
    case KEYS_DOWN:
      // For now, dumbly toggle direction with either up or down key.
      // Maybe make this smarter or repurpose later.
      // Possibly use up/down keys for Functions.
      dir = (dir == FORWARD ? REVERSE : FORWARD);
      sendThrottleCommand();
      updateDisplay();
      break;
      
    case KEYS_SELECT:
      // For now, this (Short tap) is emergency stop.
      speed = -1;
      notch = 0;
      sendThrottleCommand();
      // reset speed to Zero after sending "Emergency Stop" special value of -1
      // This won't hurt b/c the Base Station will set the loco's speed to zero as well.
      speed = 0; 
      updateDisplay();
      break;
      
    case KEYS_LONG_SELECT:
      // Idle the Loco and switch to Menus mode.
      speed = 0;
      notch = 0;
      sendThrottleCommand();
      //Serial.println("JumpbackState == " + String(jumpbackState));
      // Jump back to the last non-throttle state we were in.
      // This is so when playing the game you can hop back and forth
      // directly between the throttle and the built train view.
      throttleState = jumpbackState;
      // What to display depends on which state we're returning to.
      // TODO: Update the updateDisplay() method to handle this instead, if possible.
      if (throttleState == THROTTLE_STATE_GAME_BUILD) {
	game.doDisplayTrain(lcd, display[0], display[1]);
      } else if (throttleState == THROTTLE_STATE_GAME_MENUS) {
	doGameMenuDisplay();
      } else {
	doMenuDisplay();
      }
      break;
        
    default:
      break;
      // Do nothing.
    } // switch(button)
    break;

  case THROTTLE_STATE_DEBOUNCE:
    // ???
    break;

  case THROTTLE_STATE_MENUS:
    // Present the top-level menu.
    switch(button) {
    case KEYS_UP:
      Serial.println("Up/Left");
      // Don't let the menu system move up to Root.
      if (menu->getCurrent() == "Set Track Power") {
	// do nothing.
      } else {
	menu->moveUp();
      }
      doMenuDisplay();
      break;
      
    case KEYS_DOWN:
      Serial.println("Down/Right");
      menu->moveDown();
      doMenuDisplay();
      break;
      
    case KEYS_RIGHT:
      //menu->moveRight();
      doMenuDisplay();
      break;
      
    case KEYS_LEFT:
      menu->moveLeft();
      doMenuDisplay();
      break;
      
    case KEYS_SELECT:
      Serial.println("Select");
      throttleState = THROTTLE_STATE_MENU_ACTION;
      doMenuAction(button);
      break;
      
    case KEYS_LONG_SELECT:
      // Jump to throttle mode.  Save Menus as jumpback state.
      Serial.println("Long Select");
      throttleState = THROTTLE_STATE_RUN;
      jumpbackState = THROTTLE_STATE_MENUS;
      updateDisplay();
      break;
    }
    break;

  case THROTTLE_STATE_MENU_ACTION:
    // Handle top-level menu actions.
    switch(button) {
    case KEYS_SELECT:
      // Do the action and return.
      //EEPROM_StoreAll();
      store();
      throttleState = THROTTLE_STATE_MENUS;
      doMenuDisplay();
      break;
      
    case KEYS_UP:
    case KEYS_DOWN:
    case KEYS_LEFT:
    case KEYS_RIGHT:
      doMenuAction(button);
      break;
      
    case KEYS_LONG_SELECT:
      // Jump back to throttle state.  Don't retun here
      // Return to the menu itself instead.
      //EEPROM_StoreAll();
      store();
      throttleState = THROTTLE_STATE_RUN;
      jumpbackState = THROTTLE_STATE_MENUS;
      button = KEYS_NONE;
      updateDisplay();
      break;
    }
    break;

  case THROTTLE_STATE_GAME_MENUS:
  case THROTTLE_STATE_GAME_BUILD:
    // Handle the game action.
    doGameMenus(button);
    break;

  default:
    break;
    
  } // switch(throttleState)
}

/** menuSetup()
 *
 * Construct the menu tree
 */
void LCDThrottle::menuSetup() {
  MenuItem *miPower = new MenuItem("Set Track Power");
  MenuItem *miRun = new MenuItem("Use Throttle");
  MenuItem *miAddr = new MenuItem("Set Address");
  MenuItem *miDisp = new MenuItem("Select Display");
  MenuItem *miMax = new MenuItem("Max Speed");
  MenuItem *miGame = new MenuItem("Inglenook");
  menu->getRoot().add(*miPower);
  miPower->add(*miRun);
  miRun->add(*miAddr);
  miAddr->add(*miDisp);
  miDisp->add(*miMax);
  miMax->add(*miGame);
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
  miGame->addRight(*miBuild);
  miBuild->add(*miList);
  miList->addRight(*miCarList[0]);
  for (int i = 0; i < NUM_CARS-2; i++) {
    miCarList[i]->add(*miCarList[i+1]);
  }
  // Get off of the root node.
  menu->moveDown();
}

/** doMenuDisplay()
 *
 * Change the display in response to the current top-level menu selection
 */
void LCDThrottle::doMenuDisplay() {
  lcd->noCursor();

  if (menu->getCurrent() == "Set Track Power") {
    lcd->updateDisplay("THROTTLE MENU:", (power_state == true ? "Track Power ON" : "Track Power OFF"));
  }
  if (menu->getCurrent() == "Use Throttle") {
    lcd->updateDisplay("THROTTLE MENU:", "Use Throttle");
  }
  if (menu->getCurrent() == "Set Address") {
    lcd->updateDisplay("THROTTLE MENU:", "Set Address");
  }
  if (menu->getCurrent() == "Select Display") {
    lcd->updateDisplay("THROTTLE MENU:", "Select Display");
  }
  if (menu->getCurrent() == "Max Speed") {
    lcd->updateDisplay("THROTTLE MENU:", "Max Speed Step");
  }
  if (menu->getCurrent() == "Inglenook") {
    lcd->updateDisplay("THROTTLE MENU:", "Inglenook Game");
  }
}

/** doMenuAction()
 *
 * Take action in response to the user selecting a menu item
 */
void LCDThrottle::doMenuAction(int button) {
  Serial.println("doMenuAction(" + String(button) + ")");

  // Set Track Power:
  // Selecting this menu item toggles the track power on or off.
  if (menu->getCurrent() == "Set Track Power") {
    power_state = !power_state;
    sendPowerCommand(power_state);
    doMenuDisplay();
  }

  // Use Throttle
  // Selecting this jumps you to the throttle mode.
  if(menu->getCurrent() == "Use Throttle") {
    throttleState = THROTTLE_STATE_RUN;
    jumpbackState = THROTTLE_STATE_MENUS;
    updateDisplay();
  }

  // Inglenook
  // Selecting this moves you into the game menus.
  if (menu->getCurrent() == "Inglenook") {
    throttleState = THROTTLE_STATE_GAME_MENUS;
    menu->moveRight();
    doGameMenuDisplay();
  }

  // Select Display
  // Selecting this allows you to change the throttle display mode
  // Up/Down keys toggle the value.
  // TODO: Left/Right probably should too
  if (menu->getCurrent() == "Select Display") {
    if (button == KEYS_UP || button == KEYS_DOWN) {
      if (displayMode == DISPLAY_MODE_NORMAL) {
	displayMode = DISPLAY_MODE_SWITCHER;
      } else {
	displayMode = DISPLAY_MODE_NORMAL;
      }
    }
    lcd->updateDisplay("Select Display:",
		      displayMode == DISPLAY_MODE_NORMAL ?
		      "Standard" : "Switcher");
  } // Select Display

  // Set Address
  // Selecting this allows you to "dial in" the loco address.
  if (menu->getCurrent() == "Set Address") {
    cab = calcIncValue(button, 3, cab, 9999, "%04d", "Set Address:");
  } // Set Address

  // Max Speed
  // Selecting this allows you to change the maximum speed setting of the
  // loco (effectively setting the speed range of the throttle "knob")
  if (menu->getCurrent() == "Max Speed") {
    maxSpeed = calcIncValue(button, 2, maxSpeed, 126, "%03d", "Max Speed:");
  } // Set Address

}

/** calcIncValue()
 *
 * Calculate the incremented value to show when "dialing" a settings number.
 * TODO: Handling of the most significant digit is broken.
 */
int LCDThrottle::calcIncValue(int button, int maxpos, int val, int maxval, const char *fmt, const char *label) {
  static int incval, incpos;
  if (button == KEYS_SELECT) {
    incval = 1;
    incpos = maxpos;
    
  } else if (button == KEYS_UP) {
    if (val + incval > 9999) {
      // This will roll over the value.
      // For now, don't do anything.
      // TODO: Figure out how to roll ONLY the correct digit to zero.
    }
    else if (val == 9999) { val = 0; }
    else { val += incval; }
    
  } else if (button == KEYS_DOWN) {
    if (val - incval < 0) {
      // This will roll under the value.
      // For now, don't do anything.
      // TODO: Figure out how to roll ONLY the correct digit to 9.
    }
    else if (val == 0) { val = maxval; }
    else { val -= incval; }
  } else if (button == KEYS_LEFT) {
    if (incval < pow(10, maxpos)) { incval *= 10; }
    if (incpos > 0) { incpos -= 1; }
  } else if (button == KEYS_RIGHT) {
    if (incval > 1) { incval /= 10; }
    if (incpos < maxpos) { incpos += 1; }
  }
  Serial.println("val: " + String(val) + " incval: " + String(incval) + " incpos: " + String(incpos));
  sprintf(display[1], fmt, val);
  lcd->updateDisplay(label, display[1]);
  lcd->setCursor(incpos, 1);
  lcd->cursor();
  return(val);
}

/** sendPowerCommand()
 *
 * Send a Power on/off command to the DCC++ Base Station
 */
void LCDThrottle::sendPowerCommand(bool on) {
  sprintf(command, "");
  sprintf(command, on == true ? "1" : "0");
  Serial.println("LCD Command: " + String(command));
  SerialCommand::parse(command);
}

/** sendThrottleCommand()
 *
 * Send a Throttle command to the DCC++ Base Station
 */
void LCDThrottle::sendThrottleCommand() {
  sprintf(command, "");
  sprintf(command, "t%d %d %d %d", reg, cab, speed, dir);
  Serial.println("LCD Command: " + String(command));
  SerialCommand::parse(command);
}

/** increaseSpeed()
 *
 * Increment the current speed by one "notch"
 */
void LCDThrottle::increaseSpeed() {
  int tmp_notch;
  if (displayMode == DISPLAY_MODE_NORMAL) {
    // in DISPLAY_MODE_NORMAL, notch is 0->maxnotch.
    // since this is the "increase" function we never have to worry about
    // flipping the direction bit.
    notch = (notch == MAX_NOTCH_NORMAL ? MAX_NOTCH_NORMAL : notch + 1);
    speed = notch * (maxSpeed / MAX_NOTCH_NORMAL);
  } else {
    // in DISPLAY_MODE_SWITCHER the direction can change when "increasing"
    // the throttle, it depends.  Have to deal with absolute value.
    // First, get the signed version of "notch" and increment it.
    tmp_notch = (dir == REVERSE ? -notch : notch);
    tmp_notch = (tmp_notch == MAX_NOTCH_SWITCHER ? MAX_NOTCH_SWITCHER : tmp_notch + 1);
    // Now handle the possible sign change by setting the direction and 
    // storing notch = abs(tmp_notch)
    if (tmp_notch >= 0) {
      dir = FORWARD;
      notch = tmp_notch;    
    } else {
      dir = REVERSE;
      notch = -tmp_notch;
    }
    speed = notch * (maxSpeed / MAX_NOTCH_SWITCHER);
  } // if(displayMode)

  Serial.println("inc: N= " + String(notch) + " S=" + String(speed));
}

/** decreaseSpeed()
 *
 * Decrement the current speed by one "notch"
 */
void LCDThrottle::decreaseSpeed() {
  int tmp_notch;
  if (displayMode == DISPLAY_MODE_NORMAL) {
    notch = (notch == 0 ? 0 : notch - 1);
    speed = notch * (maxSpeed / MAX_NOTCH_NORMAL);
  } else {
    tmp_notch = (dir == REVERSE ? -notch : notch);
    tmp_notch = (tmp_notch == -MAX_NOTCH_SWITCHER ? -MAX_NOTCH_SWITCHER : tmp_notch - 1);
    if (tmp_notch < 0) {
      dir = REVERSE;
      notch = -tmp_notch;  
    } else {
      dir = FORWARD;
      notch = tmp_notch;
    }
    speed = notch * (maxSpeed / MAX_NOTCH_SWITCHER);
  }
  Serial.println("dec: N= " + String(notch) + " S=" + String(speed));
}

/** updateDisplay()
 *
 * Update the display when in Throttle mode
 */
void LCDThrottle::updateDisplay() {
  switch(displayMode) {
  case DISPLAY_MODE_SWITCHER:
    // SWITCHER: Speed/Direction together
    // Loco: <addr>
    // <------0------>
    // The blinking cursor shows the current value
    lcd->clear();
    // Draw the line.
    sprintf(display[0], "Loco: %04d", cab);
    if (power_state == true) {
      sprintf(display[1], "<------0------>");
    } else {
      sprintf(display[1], "Track Power Off");
    }
      lcd->updateDisplay(display[0], display[1]);
      Serial.println("D0:" + String(display[0]));
      Serial.println("D1:" + String(display[1]));    
    // Figure out where to put the cursor
    if (notch == 0) {
      lcd->setCursor(7,1);
    } else {
      int tmp_notch = notch;
      if (tmp_notch == 0) {
        tmp_notch += 7;
      } else {
        tmp_notch = (dir == FORWARD ? tmp_notch + 7 : 7 - tmp_notch);
      }
      Serial.println("S=" + String(speed) + " N=" + String(notch) + " T=" + String(tmp_notch));
      lcd->setCursor(tmp_notch, 1);
    }
    if (power_state == true) {
      lcd->blink();
    } else {
      lcd->noBlink(); 
    }
    break;

  case DISPLAY_MODE_NORMAL:
  default:
    // NORMAL:  Speed + Direction
    // <--  <addr>  -->
    // 0--------------+
    // The length of the bar shows the speed
    // The arrow shows the direction
    lcd->clear();
    if (dir == REVERSE) {
      sprintf(display[0], "<---  Loco: %04d", cab);
    } else {
      sprintf(display[0], "Loco: %04d  --->", cab);
    }
    if (power_state == true) {
      sprintf(display[1], "0               ");
      if (speed > 0) {
        for (int i = 0; i < notch-1; i++) {
          display[1][i+1] = '-';
        }
        display[1][notch] = '|';
        display[1][notch+1] = 0;
      }
    } else {
      sprintf(display[1], "Track Power Off");
    }
    lcd->updateDisplay(display[0], display[1]);
    Serial.println("D0:" + String(display[0]));
    Serial.println("D1:" + String(display[1]));    
    break;
  }
}

//---------------------------------------------------------------
// MenuBackend support functions

// TODO: I could probably use these effectively to clean up a bunch
// of those switch() statements above.

/** menuChangeEvent() 
 * 
 * Callback for change events.
 */
static void menuChangeEvent(MenuChangeEvent changed) {
  // Update the display to reflect the current menu state
  // For now we'll use serial output.
  Serial.print("Menu change ");
  Serial.print(changed.from.getName());
  Serial.print(" -> ");
  Serial.println(changed.to.getName());
}

/** menuUseEvent()
 * 
 * Callback for "use" events
 */
static void menuUseEvent(MenuUseEvent used) {
  //Serial.print("Menu use ");
  //Serial.println(used.item.getName());
}

//---------------------------------------------------------------
// EEPROM Interface Functions

// Memory Locations (byte address)
// NOTE: Deprecated in favor of struct LCDThrottleData and
// DCC++ EEStore interface

// DCC++ - compliant EEPROM access.
/** load()
 *
 * load sticky data from EEStore interface
 */
void LCDThrottle::load() {
  EEStore::reset();
  EEPROM.get(EEStore::pointer(), displayMode);
  EEStore::advance(sizeof(displayMode));
  EEPROM.get(EEStore::pointer(), cab);
  EEStore::advance(sizeof(cab));
  EEPROM.get(EEStore::pointer(), maxSpeed);
  EEStore::advance(sizeof(maxSpeed));
}

/** load()
 *
 * store sticky data from EEStore interface
 */
void LCDThrottle::store() {
  EEStore::reset();
  EEPROM.put(EEStore::pointer(), displayMode);
  EEStore::advance(sizeof(displayMode));
  EEPROM.put(EEStore::pointer(), cab);
  EEStore::advance(sizeof(cab));
  EEPROM.put(EEStore::pointer(), maxSpeed);
  EEStore::advance(sizeof(maxSpeed));
}

//---------------------------------------------------------------
// Inglenook Game Functions

/** doGameMenus(int button)
 *
 * Handle the Inglenook Game sub-menu
 *
 * param button: int -> current button value
 */
void LCDThrottle::doGameMenus(int button) {
  switch(throttleState) {
  case THROTTLE_STATE_GAME_MENUS:
    switch(button) {
    case KEYS_UP:
      menu->moveUp();
      doGameMenuDisplay();
      break;
      
    case KEYS_DOWN:
      menu->moveDown();
      doGameMenuDisplay();
      break;
      
    case KEYS_LEFT:
      menu->moveLeft();
      if (menu->getCurrent() == "Inglenook") {
	throttleState = THROTTLE_STATE_MENUS;
	doMenuDisplay();
      } else {
	doGameMenuDisplay();
      }
      break;
      
    case KEYS_RIGHT:
      menu->moveRight();
      doGameMenuDisplay();
      break;
      
    case KEYS_SELECT:
      if (menu->getCurrent() == "List Cars") {
        menu->moveRight(); // for this one "use" == "move right"
        doGameMenuDisplay();
      } else if (menu->getCurrent() == "Build Train") {
	throttleState = THROTTLE_STATE_GAME_BUILD;
        game.buildTrain();
	game.doDisplayTrain(lcd, display[0], display[1]);
      }
      break;
      
    case KEYS_LONG_SELECT:
      throttleState = THROTTLE_STATE_RUN;
      jumpbackState = THROTTLE_STATE_GAME_MENUS;
      updateDisplay();
      break;
    }
    break;

  case THROTTLE_STATE_GAME_BUILD:
    switch(button) {
      case KEYS_NONE:
        break;
	
      case KEYS_LEFT:
      case KEYS_SELECT:
        throttleState = THROTTLE_STATE_GAME_MENUS;
        doGameMenuDisplay();
        break;
	
      case KEYS_DOWN:
      case KEYS_RIGHT:
        if (game.carIndex() >= 3) {
          game.setCarIndex(3);
        } else {
          game.setCarIndex(game.carIndex()+1);
        }
	Serial.println("List Train: Car index " + String(game.carIndex()));
        game.doListTrain(lcd, display[0], display[1], game.carIndex());
        break;
	
      case KEYS_UP:
        if (game.carIndex() > -1) {
          game.setCarIndex(game.carIndex()-1);
        }
        if (game.carIndex() == -1) {
          game.doDisplayTrain(lcd, display[0], display[1]);
        } else {
	  Serial.println("List Train: Car index " + String(game.carIndex()));
          game.doListTrain(lcd, display[0], display[1], game.carIndex());
        }
        break;
	
    case KEYS_LONG_SELECT:
      throttleState = THROTTLE_STATE_RUN;
      jumpbackState = THROTTLE_STATE_GAME_BUILD;
      updateDisplay();
      break;
      
    } // switch(buttons)
  } // switch(state)
}

/** doGameMenuDisplay()
 *
 * Handle display output for the Inglenook Game sub-menu
 */
void LCDThrottle::doGameMenuDisplay() {
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
