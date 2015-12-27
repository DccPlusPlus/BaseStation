/**********************************************************************

AutoPilot.h
COPYRIGHT (c) 2013 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include "Arduino.h"
#include "EggTimer.h"

#ifndef AutoPilot_h
#define AutoPilot_h

#define INPUT_BUTTON  99

struct AutoPilot{
  static int enabled;
  static int status;
  static int activeStatus;
  static int eeNum;
  static int program;
  static int selectedCab;
  static EggTimer autoPilotTimer;
  static void process(int snum);
  static void special(int s, int cab);
  static void load();
  static void store();
}; // AutoPilot

struct Route{
  int nTurnouts;
  int *t;
  static EggTimer routeTimer;
  static int cTurnout;
  static Route *cRoute;
  static int callBack;
  static void (*eFunc)(int);
  Route(int n, int *t);
  static void setTurnout(int s);
  static void activate(Route *r, int s, void (*)(int)=NULL, int=0);
};

struct Cab{
  static int nCabs;
  int num;
  byte reg;
  byte lights;
  byte reverseLights;
  byte noLights;
  byte fullSpeed;
  byte slowSpeed;
  byte reverseSpeed;
  byte reverseSlowSpeed;
  byte parkingSensor;
  byte sidingSensor;
  byte prepareToParkSensor;
  Route *route;
  Cab(int num, byte reg, byte lights, byte reverseLights, byte noLights, byte fullSpeed, byte slowSpeed, byte reverseSpeed, byte reverseSlowSpeed,
    byte parkingSensor, byte sidingSensor, byte prepareToParkSensor, Route *route);
  
}; // Cab

#endif

