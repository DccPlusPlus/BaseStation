#include <Arduino.h>
#include "SerialCommand.h"
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
#include <Adafruit_RGBLCDShield.h>
#include <utility/AdafruitMCP23017.h> // is this necessary?
#else
#error CANNOT COMPILE -- INVALID LCD LIBRARY SELECTED
#endif

#include "LCDThrottle.h"

// Define generic button names for multi-library compatibility
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

#define SPEED_UP_INCREMENT   13
#define SPEED_DOWN_INCREMENT 13
#define MAX_SPEED           126
#define MAX_NOTCH_NORMAL     15
#define MAX_NOTCH_SWITCHER    7

#define THROTTLE_STATE_RUN      0
#define THROTTLE_STATE_DEBOUNCE 1

// MAX_COMMAND_LENGTH is defined in SerialCommand.h
#define MAX_COMMAND_LENGTH 30
char command[MAX_COMMAND_LENGTH];

#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
static LCDKeypad lcd = LCDKeypad();
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
static Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

//static char display[2][17];

static LCDThrottle *lcdThrottle = NULL;

static LCDThrottle *LCDThrottle::getThrottle(int r, int c, char **d) {
  if (d == NULL) { return(NULL); }
  if (lcdThrottle == NULL) {
    lcdThrottle = new LCDThrottle(r, c, d);
  }

  return(lcdThrottle);
}

LCDThrottle::LCDThrottle(int reg, int cab, char **d) {
  //lcd = new LCDKeypad();
  lcd.begin(16,2);
  throttleState = THROTTLE_STATE_RUN;
  this->reg = reg;
  this->cab = cab;
  notch = 0;
  speed = 0;
  dir = FORWARD;
  display = d;
  displayMode = DISPLAY_MODE;
  updateDisplay();
}

void LCDThrottle::run() {
  int button = debounceButtons();
  switch(throttleState) {
  case THROTTLE_STATE_RUN:
    switch(button) {
    case KEYS_RIGHT:
      increaseSpeed();
      sendThrottleCommand();
      updateDisplay();
      break;
    case KEYS_LEFT:
      decreaseSpeed();
      sendThrottleCommand();
      updateDisplay();
      break;
    case KEYS_UP:
    case KEYS_DOWN:
      // For now, dumbly toggle direction with either left or right key.
      // Maybe make this smarter or repurpose later.
      dir = (dir == FORWARD ? REVERSE : FORWARD);
      sendThrottleCommand();
      updateDisplay();
      break;
    case KEYS_SELECT:
      // For now, this is emergency stop.
      speed = -1;
      notch = 0;
      sendThrottleCommand();
      // reset speed to Zero after sending "Emergency Stop" special value of -1
      // This won't hurt b/c the Base Station will set the loco's speed to zero as well.
      speed = 0; 
      updateDisplay();
      break;
    default:
      break;
      // Do nothing.
    } // switch(button)
    break;

  case THROTTLE_STATE_DEBOUNCE:
    // ???
    break;
  } // switch(throttleState)
}

void LCDThrottle::increaseSpeed() {
  int tmp_notch;
  //int maxnotch;
  if (displayMode == DISPLAY_MODE_NORMAL) {
    // in DISPLAY_MODE_NORMAL, notch is 0->maxnotch.
    // since this is the "increase" function we never have to worry about
    // flipping the direction bit.
    notch = (notch == MAX_NOTCH_NORMAL ? MAX_NOTCH_NORMAL : notch + 1);
    speed = notch * (MAX_SPEED / MAX_NOTCH_NORMAL);
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
    speed = notch * (MAX_SPEED / MAX_NOTCH_SWITCHER);
  } // if(displayMode)

  Serial.println("inc: N= " + String(notch) + " S=" + String(speed));
}

void LCDThrottle::decreaseSpeed() {
  int tmp_notch;
  if (displayMode == DISPLAY_MODE_NORMAL) {
    notch = (notch == 0 ? 0 : notch - 1);
    speed = notch * (MAX_SPEED / MAX_NOTCH_NORMAL);
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
    speed = notch * (MAX_SPEED / MAX_NOTCH_SWITCHER);
  }
  Serial.println("dec: N= " + String(notch) + " S=" + String(speed));
}

void LCDThrottle::sendThrottleCommand() {
  sprintf(command, "");
  sprintf(command, "t%d %d %d %d", reg, cab, speed, dir);
  Serial.println(command);
  SerialCommand::parse(command);
}
/*
int LCDThrottle::checkButtons() {
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
*/

/** getButton()
 * Translates the different LCD button values
 * to a common set
 */
int LCDThrottle::getButton() {
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
  // OSEPP LCDKeypad
  return(lcd.button());
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

int LCDThrottle::debounceButtons() {
  int button = getButton();
  static long startDebounce;
  static int keyval;
  int retv;
  if (throttleState == THROTTLE_STATE_RUN) {
    if (button == KEYS_NONE) {
      return(KEYS_NONE);
    } else {
      Serial.println("Raw Key: " + String(button));
      startDebounce = millis();
      keyval = button;
      throttleState = THROTTLE_STATE_DEBOUNCE;
    }
  } else {
    // actively debouncing...
    if (button == KEYS_NONE) {
      Serial.println("Debounced Key: " + String(keyval));
      // Debounce complete. Decide if it's long or not.
      throttleState = THROTTLE_STATE_RUN;
      retv = keyval;
      keyval = KEYS_NONE;
      if (millis() - startDebounce > 2000) {
	Serial.println("2 second button press! Val = " + String(retv));
	// Longer than 2 second hold
	switch(retv) {
	case KEYS_RIGHT:
	  return(KEYS_LONG_RIGHT);
	case KEYS_UP:
	  return(KEYS_LONG_UP);
	case KEYS_DOWN:
	  return(KEYS_LONG_DOWN);
	case KEYS_LEFT:
	  return(KEYS_LONG_LEFT);
	case KEYS_SELECT:
	  return(KEYS_LONG_SELECT);
	default:
	  return(KEYS_NONE);
	}
      } else {
	return(retv);
      } // long key
    } // keys_none
  } // throttle state == RUN
}

// Display Modes...

void LCDThrottle::updateDisplay() {
  switch(displayMode) {
  case DISPLAY_MODE_SWITCHER:
    // SWITCHER: Speed/Direction together
    // (something useful)
    // <------0------>
    lcd.clear();
    //lcd.setCursor(0,1);
    // Draw the line.
    sprintf(display[0], "Throttle: %04d", cab);
    sprintf(display[1], "<------0------>");
    //lcd.print("<------0------>");
    // Figure out where to put the cursor
    lcd.setCursor(0,0); lcd.print(display[0]);
    lcd.setCursor(0,1); lcd.print(display[1]);
    Serial.println("D0:" + String(display[0]));
    Serial.println("D1:" + String(display[1]));    
    if (notch == 0) {
      lcd.setCursor(7,1);
    } else {
      int tmp_notch = notch;
      if (tmp_notch == 0) {
        tmp_notch += 7;
      } else {
        tmp_notch = (dir == FORWARD ? tmp_notch + 7 : 7 - tmp_notch);
      }
      Serial.println("S=" + String(speed) + " N=" + String(notch) + " T=" + String(tmp_notch));
      lcd.setCursor(tmp_notch, 1);
    }
    lcd.blink();
    break;

  case DISPLAY_MODE_NORMAL:
  default:
    // NORMAL:  Speed + Direction
    // <--          -->
    // 0--------------+
    lcd.clear();
    lcd.setCursor(0,0);
    if (dir == REVERSE) {
      sprintf(display[0], "<---  Loco: %04d", cab);
      //lcd.print("<---            ");
    } else {
      sprintf(display[0], "Loco: %04d  --->", cab);
      //lcd.print("        DIR --->");
    }
    sprintf(display[1], "0               ");
    if (speed > 0) {
      for (int i = 0; i < notch-1; i++) {
        display[1][i+1] = '-';
      }
      display[1][notch] = '|';
      display[1][notch+1] = 0;
    }
    lcd.setCursor(0,0); lcd.print(display[0]);
    lcd.setCursor(0,1); lcd.print(display[1]);
    Serial.println("D0:" + String(display[0]));
    Serial.println("D1:" + String(display[1]));    
    break;
  }
}
