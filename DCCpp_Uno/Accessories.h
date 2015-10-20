/**********************************************************************

Accessories.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include "Arduino.h"

#ifndef Accessories_h
#define Accessories_h

struct Turnout{
  static int nTurnouts;
  int num;
  int id;
  int address;
  int subAddress;
  int tStatus;
  Turnout(int, int, int);
  void activate(int s);
  static void parse(char *c);
  static Turnout* get(int);
}; // Turnout
  
#endif

extern Turnout turnouts[];

