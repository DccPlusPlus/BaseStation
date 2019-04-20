/**********************************************************************

Sensor.h
COPYRIGHT (c) 2018 Dan Worth

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef RemoteSensor_h
#define RemoteSensor_h

#include "Sensor.h"

#define REMOTE_SENSORS_PREFIX "sensor"
#define REMOTE_SENSORS_DECAY 10000

struct RemoteSensorData {
  int snum;
  int value;
};

struct RemoteSensor{
  static RemoteSensor *firstRemoteSensor;
  RemoteSensorData data;
  long lastUpdate;
  RemoteSensor *nextRemoteSensor;
  static RemoteSensor *create(int, int);
  static RemoteSensor* get(int);
  static void remove(int);  
//  static void status();
  static void show();
  static void parse(char *c);
  static void check();   
}; // Remote Sensor

#endif

