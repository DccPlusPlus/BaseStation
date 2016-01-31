/**********************************************************************

DccServer.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef DccServer_h
#define DccServer_h

#ifdef ARDUINO_AVR_UNO                        // Configuration for UNO
  #define  SERVER_SAMPLE_TIME        10000
#else                                         // Configuration for MEGA    
  #define  SERVER_SAMPLE_TIME        1000
#endif

#include "Arduino.h"
#include <Wire.h>

struct DccServerData {
  byte snum;
};

struct DccServer{
  static DccServer *firstDccServer;
  static long int checkTime;
  static boolean masterStarted;
  static byte serverID;
  DccServerData data;
  boolean active;
  boolean upLoaded;
  boolean reset;
  boolean readyForQuery;
  DccServer *nextDccServer;
  static void load();
  static void store();
  static DccServer *create(int, int=0);
  static DccServer* get(int);  
  static void remove(int);  
  static void status();
  static void parse(char *c);
  static void check();
  static void init(int=-1);   
  static void receiveWire(int);
  static void serverQuery();
  static boolean serverReset;
}; // DccServer

#endif

