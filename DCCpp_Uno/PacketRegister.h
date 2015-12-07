/**********************************************************************

PacketRegister.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef PacketRegister_h
#define PacketRegister_h

#include "Arduino.h"

// Define a series of registers that can be sequentially accessed over a loop to generate a repeating series of DCC Packets

#define  ACK_CURRENT_MIN            50
#define  ACK_CURRENT_COUNT          30

struct Packet{
  byte buf[10];
  byte nBits;
}; // Packet

struct Register{
  Packet packet[2];
  Packet *activePacket;
  Packet *updatePacket;
  void initPackets();
}; // Register
  
struct RegisterList{  
  int maxNumRegs;
  Register *reg;
  Register **regMap;
  Register *currentReg;
  Register *maxLoadedReg;
  Register *nextReg;
  Packet  *tempPacket;
  byte currentBit;
  byte nRepeat;
  int *speedTable;
  static byte idlePacket[];
  static byte resetPacket[];
  static byte bitMask[];
  RegisterList(int);
  void loadPacket(int, byte *, int, int, int=0) volatile;
  void setThrottle(char *) volatile;
  void setFunction(char *) volatile;  
  void setAccessory(char *) volatile;
  void writeTextPacket(char *) volatile;
  void readCV(char *) volatile;
  void writeCVByte(char *) volatile;
  void writeCVBit(char *) volatile;
  void writeCVByteMain(char *) volatile;
  void writeCVBitMain(char *s) volatile;  
  void printPacket(int, byte *, int, int) volatile;
};

#endif
