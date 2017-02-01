#include <Arduino.h>
#include <string.h>

// Include our own header with definitions and such.
#include "LCD.h"

// Include the necessary libraries for the different display shields.
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h> // is this necessary?
#else
#error CANNOT COMPILE -- INVALID LCD LIBRARY SELECTED
#endif

// Create the local object of the display we are
// controlling.
#if (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_OSEPP)
static LCDKeypad lcd = LCDKeypad();
#elif (LCD_DISPLAY_TYPE == LCD_DISPLAY_TYPE_ADAFRUIT)
static Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

// Define the size of the display.  Pretty much assumes
// 16x2 but could be modded to change that.
#define LCD_NUM_ROWS 2
#define LCD_NUM_COLS 16

// State machine for handling processing of the buttons.
#define DISPLAY_STATE_RUN 0
#define DISPLAY_STATE_DEBOUNCE 1
#define DISPLAY_STATE_DEBOUNCE_COMPLETE 2
#define DISPLAY_STATE_LONG_DEBOUNCE 3
#define DISPLAY_STATE_LONG_DEBOUNCE_COMPLETE 4
#define DISPLAY_STATE_LONG_DEBOUNCE_WAIT 5

// ctor
LCD::LCD() {
  // Nothing to do ... yet ...
}

// Setup function.  Call from setup()
void LCD::begin() {
  buttonVal = KEYS_NONE;
  displayState = DISPLAY_STATE_RUN;
  lcd.begin(LCD_NUM_COLS, LCD_NUM_ROWS);
}

// Run loop for processing the buttons.  Call from loop()
void LCD::run() {
  debounceButtons();
}

//------------------------------------------------
// Pass-through methods for handling the display
void LCD::clear() {
  lcd.clear();
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

//------------------------------------------------
// Custom methods

// Update the two lines of the display with these two strings.
void LCD::updateDisplay(char *row1, char *row2) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(row1);
  lcd.setCursor(0,1); lcd.print(row2);
}


// Get the current button value.
int LCD::getButtons() {
  int bv = buttonVal;
  buttonVal = KEYS_NONE;
  return(bv);
}

//------------------------------------------------
// Private / Protected internal methods

// Retrieve the hardware button value and translate
// it into one of our "standard" button values.
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

// Debounce the button press and figure out if it's a long press.
void LCD::debounceButtons() {
  int button = getButton();
  static long startDebounce;
  static int keyval;
  int retv;
  switch(displayState) {
  case DISPLAY_STATE_RUN:
    if (button != KEYS_NONE) {
      Serial.println("Raw Key: " + String(button));
      startDebounce = millis();
      keyval = button;
      displayState = DISPLAY_STATE_DEBOUNCE;
    }
    // Always return KEYS_NONE from this state
    break;

  case DISPLAY_STATE_DEBOUNCE:
    // actively debouncing...
    if ((button != KEYS_NONE) && ((millis() - startDebounce) > LONG_PRESS_MS)) {
      // Longer than 2 second hold
      displayState = DISPLAY_STATE_DEBOUNCE_COMPLETE;
      switch(keyval) {
      case KEYS_RIGHT:
	keyval = KEYS_LONG_RIGHT; break;
      case KEYS_UP:
	keyval = KEYS_LONG_UP; break;
      case KEYS_DOWN:
	keyval = KEYS_LONG_DOWN; break;
      case KEYS_LEFT:
	keyval = KEYS_LONG_LEFT; break;
      case KEYS_SELECT:
	keyval = KEYS_LONG_SELECT; break;
      default:
	break;
      }
      buttonVal = keyval;
      keyval = KEYS_NONE;
      Serial.println("2 second button press! Val = " + String(buttonVal));
      
    } else if (button == KEYS_NONE) {
      // Short press.
      Serial.println("Short Press. Debounced Key: " + String(keyval));
      // Debounce complete. Decide if it's long or not.
      displayState = DISPLAY_STATE_DEBOUNCE_COMPLETE;
      buttonVal = keyval;
    } else {
      // Not finished debouncing yet. Do nothing.
    } // KEYS_NONE
    break;
    
  case DISPLAY_STATE_LONG_DEBOUNCE_WAIT:
  case DISPLAY_STATE_DEBOUNCE_COMPLETE:
    // Long press detected, waiting for release before handling more button presses
    if (button == KEYS_NONE) {
      // User has released button.
      displayState = DISPLAY_STATE_RUN;
      buttonVal = KEYS_NONE;
    }
    break;

  } // switch(displayState)
}
