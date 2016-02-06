/**********************************************************************

PacketRegister.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef PacketRegister_h
#define PacketRegister_h

#include "Arduino.h"

// Define constants used for reading CVs from the Programming Track

#define  ACK_BASE_COUNT            100      // number of analogRead samples to take before each CV verify to establish a baseline current
#define  ACK_SAMPLE_COUNT          500      // number of analogRead samples to take when monitoring current after a CV verify (bit or byte) has been sent 
#define  ACK_SAMPLE_SMOOTHING      0.2      // exponential smoothing to use in processing the analogRead samples after a CV verify (bit or byte) has been sent
#define  ACK_SAMPLE_THRESHOLD       30      // the threshold that the exponentially-smoothed analogRead samples (after subtracting the baseline current) must cross to establish ACKNOWLEDGEMENT

// Define a series of registers that can be sequentially accessed over a loop to generate a repeating series of DCC Packets

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
