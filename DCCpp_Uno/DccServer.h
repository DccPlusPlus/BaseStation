/**********************************************************************

DccServer.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef DccServer_h
#define DccServer_h

#include <Arduino.h>

struct DccServer{
  static byte serverID;
  static boolean isMaster;
  static void load();
  static void store();
  static void status();
  static void parse(char *);  
  static void busWrite(int, char *);
  static void busRead(char *);
}; // DccServer

#endif

