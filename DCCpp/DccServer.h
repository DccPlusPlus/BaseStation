/**********************************************************************

DccServer.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef DccServer_h
#define DccServer_h

#include "Arduino.h"
#include <Wire.h>
#include "Sensor.h"

struct DccServer{
  static byte serverID;
  static void load();
  static void store();
  static void status();
  static void parse(char *c);
  static void setServer(int);
  static void setMaster();
  static void receiveWire(int);
  static void upload(Sensor *);
}; // DccServer

struct RemoteSensor{
  static RemoteSensor *firstSensor;
  int snum;
  boolean active;
  RemoteSensor *nextSensor;
  static RemoteSensor *create(int);
  static RemoteSensor *get(int);  
  static void status();
}; // Sensor

#endif

