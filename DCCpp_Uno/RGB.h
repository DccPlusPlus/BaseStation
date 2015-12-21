/**********************************************************************

RGB.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"

#ifndef RGB_h
#define RGB_h

#define RED_LED 44
#define GREEN_LED 45
#define BLUE_LED 46

struct RGBLight{
  static int rValue,gValue,bValue;
  static void parse(char *c);
  static void activate(int, int, int, int=0);
  static void HSV2RGB(int&, int&, int&);
  static void store();
  static void load();
  static void init();
  static void show();
//  static void blink(int n);
}; // RGBLight

#endif


