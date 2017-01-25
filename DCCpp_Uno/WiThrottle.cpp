/**********************************************************************

WiThrottle.cpp
COPYRIGHT (c) 2017 Mark S. Underwood

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include <Arduino.h>
#include <string.h>
#include "DCCpp_Uno.h"
#include "Comm.h"
#include "SerialCommand.h"
#include "PacketRegister.h"
#include "WiThrottle.hpp"

#define FORCED_REGISTER_NUMBER 1

extern RegisterList mainRegs;

static char command[MAX_COMMAND_LENGTH+1];
static int address = 3;

static const byte byte1FuncOnVals[29] = { 144, 129, 130, 132, 136,
					  177, 178, 180, 184,
					  161, 162, 164, 168,
					  222, 222, 222, 222,
					  222, 222, 222, 222,
					  223, 223, 223, 223,
					  223, 223, 223, 223 };

static const byte byte1FuncOffVals[29] = { 128, 128, 128, 128, 128,
					   176, 176, 176, 176,
					   160, 160, 160, 160,
					   222, 222, 222, 222,
					   222, 222, 222, 222,
					   223, 223, 223, 223,
					   223, 223, 223, 223 };

static const byte byte2FuncOnVals[29] = { 1, 2, 4, 8,
					  16, 32, 64, 128,
					  1, 2, 4, 8,
					  16, 32, 64, 128 };
  

static void WiThrottle::readCommand(char c) {
  char x;
  sprintf(command, "%c", c);

#if COMM_TYPE == 0

  // Read all the bytes until we encounter a newline, end-of-string, or
  // run out of bytes.
  while(INTERFACE.available() > 0) {
    x = INTERFACE.read();
    Serial.print(x);
    if (x == '\n' || x == '\0') {
      sprintf(command,"%s%c",command, x);
      Serial.println("Received");
      parseToDCCpp(command);
    } else if (strlen(command) < MAX_COMMAND_LENGTH) {
      sprintf(command, "%s%c", command, x);
    }
  }

#elif COMM_TYPE == 1

  // Connect to the source
  EthernetClient client = INTERFACE.available();

  // Read all the bytes until we encounter a newline, end-of-string, or
  // run out of bytes.
  if (client) {
    while (client.connected() && client.available()) {
      x = client.read();
      if (x == '\n' || x == '\0') {
	sprintf(command,"%s%c",command, x);
	parseToDCCpp(command);
      } else if (strlen(command) < MAX_COMMAND_LENGTH) {
	sprintf(command, "%s%c", command, x);
      }
    }
  }
#endif   
}

static void WiThrottle::parseToDCCpp(char *s) {
  // Do something :)
  Serial.println(s);
  switch (s[0]) {
  case 'T': // Throttle
  case 'S': // Second Throttle
    doThrottleCommand(NULL, s+1);
    break;
  case 'M': // Multi-Throttle
    parseMCommand(s);
    break;
  case 'C': // Old "T" command
    if (s[1] == 'T') {
      doThrottleCommand(NULL, s+2);
    }
    break;
  case 'N': // Name of throttle
    parseNCommand(s);
    break;
  case 'H': // Hardware (get device UUID)
    parseHCommand(s);
    break;
  case '*': // Heartbeat
  case 'D': // Direct hex packet
  case 'P': // Panel stuff
  case 'R': // Roster stuff
  case 'Q': // Quit
    break;
  default:
    return(s);
  }
}

static bool WiThrottle::isWTCommand(char c) {
  switch (c) {
  case 'T': // Throttle
  case 'S': // Second Throttle
  case 'M': // Multi-Throttle
  case 'D': // Direct hex packet
  case '*': // Heartbeat
  case 'C': // Old 'T' command
  case 'N': // Name
  case 'H': // Hardware
  case 'P': // Panel
  case 'R': // Roster
  case 'Q': // Quit
    Serial.print(c);
    Serial.println(" is a WiThrottle cmd");
    return(true);
  default:
    return(false);
  }
}

static void WiThrottle::parseHCommand(char *s) {
  switch(s[1]) {
  case 'U':
    // get device UDID and do something with it.
    return;
  default:
    return;
  }
}

static void WiThrottle::parseNCommand(char *s) {
  // Get the Name and store it somewhere... if needed.
  Serial.print("Name = ");
  Serial.println(String(s));
  return;
}

static void WiThrottle::parseMCommand(char *s) {
  char *key, *action;
  switch(s[1]) {
  case 'A':
    key = strtok(s, "<;>");
    action = strtok(NULL, "<;>");
    Serial.println(key);
    Serial.println(action);
    doThrottleCommand(key, action);
  case '+':
  case '-':
  default:
    return;
  }
}

static void WiThrottle::doThrottleCommand(char *key, char *action) {
  int reg, dir, spd, f;
  byte byte1, byte2;
  // TODO: When supporting multiple throttles, KEY will tell us which
  // throttle to do the action on.
  switch(action[0]) {
  case 'V': // Velocity
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION>
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    dir = getDirForCab(address);
    sprintf(command, "t %d %d %s %d", reg, address, (action+1), dir );
    SerialCommand::parse(command);
    break;
    
  case 'X': // E-Stop
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION> with SPEED = -1
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    dir = getDirForCab(address);
    sprintf(command, "t %d %d -1 %d", reg, address, (action+1), dir);
    SerialCommand::parse(command);
    break;
    
  case 'F': // Function
  case 'f': // force function (v>=2.0)
    // DCC++ Format: <f CAB BYTE1 [BYTE2]>
    // DCC++ Returns: (none)
    f = strtol((action+2), NULL,10);
    if (f == 0 || f > 28) {
      // Invalid conversion
      break;
    }
    byte1 = getFuncByte1((action[1] == '1'), f);
    byte2 = getFuncByte2((action[1] == '1'), f);
    if (byte2 == 255) { 
      sprintf(command, "f %d %d", address, byte1);
    } else {
      sprintf(command, "f %d %d %d", address, byte1, byte2);
    }
    SerialCommand::parse(command);
    break;
    
  case 'R': // Direction
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION>
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    spd = getSpeedForCab(address);
    sprintf(command, "t %d %d %d %c", reg, address, spd,
	    (action[1] == '0' ? '0' : '1'));
      Serial.print("cmd = " + String(command));
    SerialCommand::parse(command);
    break;
    
  case 'I': // Idle
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION>
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    dir = getDirForCab(address);
    sprintf(command, "t %d %d 0 %d", reg, address, dir);
    SerialCommand::parse(command);
    break;
    
  case 'r': // Release
  case 'd': // Dispatch
    address = 0;
    break;
    
  case 'L': // set long address
  case 'S': // set short address
    address = strtol((action+1), NULL, 10);
    break;

  case 'q': // request (v>=2.0)
    handleRequest(action);
    break;

  case 'E': // set address from roster (v>=1.7)
  case 'C': // consist
  case 'c': // consist lead from roster (v>=1.7)
  case 's': // speed step mode (v>= 2.0)
  case 'm': // momentary (v>=2.0)
  case 'Q': // quit
  default:
    return;
  }
}

byte WiThrottle::getFuncByte1(bool t, int f) {
  if (t) {
    return(byte1FuncOnVals[f]);
  } else {
    return(byte1FuncOffVals[f]);
  }
}

byte WiThrottle::getFuncByte2(bool t, int f) {
  if (f < 13) {
    return(255);
  } else if (t) {
    return(byte2FuncOnVals[f-13]);
  } else {
    return(0);
  }
}

void WiThrottle::handleRequest(char *s) {
  // TODO: Handle Requests
  return;
}

int WiThrottle::getRegisterForCab(int c) {
  return(FORCED_REGISTER_NUMBER);
}

int WiThrottle::getSpeedForCab(int c) {
  int spd = mainRegs.speedTable[FORCED_REGISTER_NUMBER];
  if (spd < 0) { spd = -spd; }
  return(spd);
}

int WiThrottle::getDirForCab(int c) {
  int spd = mainRegs.speedTable[FORCED_REGISTER_NUMBER];
  return(spd >= 0 ? 1 : 0);
}

void WiThrottle::sendIntroMessage(void) {
  INTERFACE.println("VN2.0");
  // TODO: Send roster here
#if COMM_TYPE == 1
  INTERFACE.print("PW");
  INTERFACE.println(ETHERNET_PORT);
#endif
}
