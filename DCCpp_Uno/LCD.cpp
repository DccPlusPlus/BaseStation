#include <Arduino.h>
#include <string.h>
#include "LCD.h"

#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#else
#error CANNOT COMPILE -- INVALID LCD LIBRARY SELECTED
#endif


#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
static LCDKeypad lcd = LCDKeypad();
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
static Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

#define LCD_NUM_ROWS 2
#define LCD_NUM_COLS 16
static char display[LCD_NUM_ROWS][LCD_NUM_COLS+1];

#define DISPLAY_STATE_RUN 0
#define DISPLAY_STATE_DEBOUNCE 1
#define DISPLAY_STATE_DEBOUNCE_COMPLETE 2

#define LONG_PRESS_MS 2000

LCD::LCD() {
  // Nothing to do ... yet ...
  buttonVal = KEYS_NONE;
  displayState = DISPLAY_STATE_RUN;
}

void LCD::begin() {
  lcd.begin(LCD_NUM_COLS, LCD_NUM_ROWS);
}

void LCD::clear() {
  lcd.clear();
}

void LCD::run() {
  int bv = debounceButtons();
  // Not sure yet what needs done here.  For now it's optional.
  switch(displayState) {
  case DISPLAY_STATE_DEBOUNCE_COMPLETE:
    buttonVal = bv;
    displayState = DISPLAY_STATE_RUN;
    break;
  case DISPLAY_STATE_RUN:
  case DISPLAY_STATE_DEBOUNCE:
  default:
    break;
  }
}

void LCD::setCursor(int c, int r) {
  lcd.setCursor(c, r);
}

void LCD::blink() {
  lcd.blink();
}

void LCD::noBlink() {
  lcd.noBlink();
}

void LCD::cursor() {
  lcd.cursor();
}

void LCD::noCursor() {
  lcd.noCursor();
}

void LCD::updateDisplay(char *row1, char *row2) {
  lcd.clear();
  sprintf(display[0],"%s", row1);
  sprintf(display[1],"%s", row2);
  lcd.setCursor(0,0); lcd.print(display[0]);
  lcd.setCursor(0,1); lcd.print(display[1]);
}


int LCD::getButtons() {
  int bv = buttonVal;
  buttonVal = KEYS_NONE;
  return(bv);
}

int LCD::getButton() {
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
  // OSEPP LCDKeypad
  int b = lcd.button();
  //Serial.print(String(b) + " ");
  return(b);
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

int LCD::debounceButtons() {
  int button = getButton();
  static long startDebounce;
  static int keyval;
  int retv;
  if (displayState == DISPLAY_STATE_RUN) {
    if (button != KEYS_NONE) {
      Serial.println("Raw Key: " + String(button));
      startDebounce = millis();
      keyval = button;
      displayState = DISPLAY_STATE_DEBOUNCE;
    }
    // Always return KEYS_NONE from this state
    return(KEYS_NONE);
    
  } else if (displayState == DISPLAY_STATE_DEBOUNCE) {
    // actively debouncing...
    if ((button != KEYS_NONE) && ((millis() - startDebounce) > LONG_PRESS_MS)) {
      // Longer than 2 second hold
      displayState = DISPLAY_STATE_DEBOUNCE_COMPLETE;
      retv = keyval;
      keyval = KEYS_NONE;
      Serial.println("2 second button press! Val = " + String(retv));
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
    } else if (button == KEYS_NONE) {
      // Short press.
      Serial.println("Short Press. Debounced Key: " + String(keyval));
      // Debounce complete. Decide if it's long or not.
      displayState = DISPLAY_STATE_DEBOUNCE_COMPLETE;
      retv = keyval;
      keyval = KEYS_NONE;
      return(retv);
    } else {
      // Not finished debouncing yet.
      return(KEYS_NONE);
    } // KEYS_NONE
  } // throttle state == RUN
}
