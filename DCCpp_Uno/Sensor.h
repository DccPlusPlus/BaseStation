/**********************************************************************

Sensor.cpp
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"

#define  SENSOR_DECAY  0.03

struct Sensor{
  static int nSensors;
  int pin;
  int snum;
  boolean active;
  float signal;
  Sensor(int, int, int);
  void check();  
}; // Sensor

#endif

