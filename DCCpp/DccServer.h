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
#include "Outputs.h"

struct RemoteSensor{
  static RemoteSensor *firstSensor;
  int snum;
  boolean active;
  RemoteSensor *nextSensor;
  static RemoteSensor *create(int);
  static RemoteSensor *get(int);  
  static void status();
}; // RemoteSensor

struct RemoteOutput{
  static RemoteOutput *firstOutput;
  int snum;
  boolean active;
  byte serverID;
  byte uploaded;
  RemoteOutput *nextOutput;
  static RemoteOutput *create(int, int);
  static RemoteOutput *get(int);  
  static void status();
  static void check();
}; // RemoteOutput

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
  static void upload(Output *);
  static void upload(RemoteOutput *);
}; // DccServer



#endif

