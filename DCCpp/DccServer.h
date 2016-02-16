/**********************************************************************

DccServer.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef DccServer_h
#define DccServer_h

#include "Arduino.h"
#include "Sensor.h"
#include "Outputs.h"

#define TWI_BUF_SIZE  4     // max number of bytes in TWI read/write buffer

enum { READY, READING, WRITING, DATA_RECEIVED, DATA_SEND_SUCCESS, DATA_SEND_FAIL };

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
  boolean activeDesired;
  byte serverID;
  boolean uploaded;
  RemoteOutput *nextOutput;
  static RemoteOutput *create(int, int);
  static RemoteOutput *get(int);  
  static void status();
  static void check();
  void activate(int);
}; // RemoteOutput

struct DccServer{
  static byte serverID;
  volatile static byte rData[TWI_BUF_SIZE];
  volatile static byte rDataIdx;
  volatile static byte wData[TWI_BUF_SIZE];
  volatile static byte wDataIdx;
  volatile static byte wAddress;
  volatile static byte state;
  static boolean *uploaded;
  static boolean rUploaded;
  static void load();
  static void store();
  static void status();
  static void refresh();
  static void parse(char *c);
//  static void setServer(int);
//  static void setMaster();
  static void check();
//  static void upload(Sensor *);
//  static void upload(Output *);
  static void init();
  static void twiWrite(boolean&, byte, byte, byte, byte, byte);
}; // DccServer



#endif

