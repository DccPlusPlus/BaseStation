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
#include "Outputs.h"
#include "Accessories.h"
#include "WiThrottle.hpp"

#define FORCED_REGISTER_NUMBER 1

// Store decoder addresses for DCC turnouts here.  These take
// priority over actual built-in outputs.  One should take
// care not to have the name spaces overlap...

#define MAX_DCC_TURNOUTS 5
struct TurnoutData dccTurnouts[MAX_DCC_TURNOUTS] = {
  { 1, 0, 1, 1 },
  { 1, 0, 2, 2 },
  { 1, 0, 3, 3 },
  { 1, 0, 4, 4 },
  { 1, 0, 5, 5 },
};

extern RegisterList mainRegs;

static char message[MAX_COMMAND_LENGTH+1];
static char *command = SerialCommand::commandString;
static int address = 3;
static int speed = 0;
static int dir = 1;

bool fstate[29] = {
  false, false, false, false, false,
  false, false, false, false, false,
  false, false, false, false, false,
  false, false, false, false, false,
  false, false, false, false, false,
  false, false, false, false
};
  

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
  Serial.print("RX: ");
  Serial.println(s);
  switch (s[0]) {
  case 'T': // Throttle
  case 'S': // Second Throttle
    doThrottleCommand(NULL, s+1);
    break;
  case 'M': // Multi-Throttle
    parseMCommand(s);
    break;
  case 'C': // Old "T" command - not used anymore. Kept for backward compatibility
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
  case 'P': // Panel stuff
    parsePCommand(s);
    break;
  case '*': // Heartbeat
  case 'D': // Direct hex packet
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
    //Serial.print(c);
    //Serial.println(" is a WiThrottle cmd");
    return(true);
  default:
    return(false);
  }
}

static void WiThrottle::parsePCommand(char *p) {
  // Commands:
  //     PPAx : Track power on/off
  //     PTAx : Turnout throw/close
  //     PRAx : Route set/unset
  switch(p[1]) {
  case 'P':
    // Track power on or off
    switch(p[2]) {
    case 'A':
      if (p[3] == '1') {
	// Track power ON
	Serial.println("Power ON");
	sprintf(command, "1");
	SerialCommand::parse(command);
	INTERFACE.println("PPA1");
      } else if (p[3] == '0') {
	// Track power OFF
	Serial.println("Power OFF");
	sprintf(command, "0");
	SerialCommand::parse(command);
	INTERFACE.println("PPA0");
      } // p[3]
      // Else ignore.
      break;
    } // p[2]
    break;

  case 'T':
    // Turnout command
    // C = close T = throw 2 = toggle
    if (p[2] == 'A') {
      handleTurnout(p+3);
    }
    break;
  } // switch(p[1])
}

static void WiThrottle::parseHCommand(char *s) {
  switch(s[1]) {
  case 'U':
    // get device UDID and do something with it.
    Serial.println("RX Device ID: " + String(s+2));
    return;
  default:
    return;
  }
}

static void WiThrottle::parseNCommand(char *s) {
  // Get the Name and store it somewhere... if needed.
  Serial.print("Name = ");
  Serial.println(String(s));
  // Reply *<heartbeat time> e.g. *10 for 10 seconds or *0 for no heartbeat expected
  doPrintln("*0");
  return;
}

static void WiThrottle::parseMCommand(char *s) {
  char *key, *action;
  switch(s[2]) {
  case 'A':
  case '+':
    key = strtok(s, "<;>");
    action = strtok(NULL, "<;>");
    Serial.println("Key = " + String(key));
    Serial.println("Action = " + String(action));
    doThrottleCommand(key, action);
    if (s[2] == '+') {
      doPrint(key);
      doPrint("<;>");
      doPrintln(action);
    }
    break;  
  case '-':
  default:
    return;
  }
}

static void WiThrottle::doThrottleCommand(char *key, char *action) {
  int reg, spd, f;
  byte byte1, byte2;
  address = strtol(key+4, NULL, 10);
  // TODO: When supporting multiple throttles, KEY will tell us which
  // throttle to do the action on.
  switch(action[0]) {
  case 'V': // Velocity
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION>
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    //dir = getDirForCab(address);
    sprintf(command, "t %d %d %s %d", reg, address, (action+1), dir );
    sscanf(action+1, "%d", &spd);
    //speed = strtol((action+1), NULL, 10);
    Serial.print("new speed = ");
    Serial.print(action+1);
    Serial.print(" dir = ");
    Serial.println(dir);
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
    // WiThrottle Format: FxVV
    // x = 1 (on) or 0 (off)
    // VV is the function number
    // DCC++ Format: <f CAB BYTE1 [BYTE2]>
    // DCC++ Returns: (none)
    address = strtol(key+4, NULL, 10);
    f = strtol((action+2), NULL,10);
    Serial.println("addr = " + String(address) + " F = " + String(f) + " is " + String(action[1] == '1' ? "ON" : "OFF"));
    if (f < 0 || f > 28) {
      // Invalid conversion
      break;
    }
    // NOTE:  strtol() returns zero on an invalid conversion
    // That is harmless here. F0 is the headlight, so the worst
    // thing that will happen on an invalid conversion is we tooggle
    // the headlight.  Oh well.
    if ((action[1] == '1') || (f == 2)) {
      // Button Pressed.. Take action
      // Horn (F2) is momentary.  Take action even if action[1] == 0
      // Toggle the state.
      fstate[f] = !fstate[f];
      // Get the bytes to send.
      byte1 = getFuncByte1(fstate[f], f);
      byte2 = getFuncByte2(fstate[f], f);
      // Build the DCC++ message and send it
      if (byte2 == 255) { 
	sprintf(message, "f %d %d", address, byte1);
      } else {
	sprintf(message, "f %d %d %d", address, byte1, byte2);
      }
      SerialCommand::parse(message);
      // Send the response to the WiThrottle
      doPrint(key);
      doPrint("<;>");
      action[1] = (fstate[f] == true ? '1' : '0');
      doPrintln(action);
    }
    break;
    
  case 'R': // Direction
    // DCC++ Format: <t REGISTER CAB SPEED DIRECTION>
    // DCC++ Returns: <T REGISTER SPEED DIRECTION>
    if (address < 0) { return; }
    reg = getRegisterForCab(address);
    spd = getSpeedForCab(address);
    if (action[1] == '0') {
      dir = 0;
    } else {
      dir = 1;
    }
    sprintf(command, "t %d %d %d %d", reg, address, spd, dir);
    Serial.println("cmd = " + String(command));
    Serial.println("dir = " + String(dir));
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
    doPrintln("MT+L" + String(address) + "<;>");
    break;

  case 'q': // request (v>=2.0)
    handleRequest(action);
    break;

  case 'Q': // quit
  case 'E': // set address from roster (v>=1.7)
  case 'C': // consist
  case 'c': // consist lead from roster (v>=1.7)
  case 's': // speed step mode (v>= 2.0)
  case 'm': // momentary (v>=2.0)
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
  //int spd = speed;
  int spd = mainRegs.speedTable[FORCED_REGISTER_NUMBER];
  if (spd < 0) { spd = -spd; }
  return(spd);
}

int WiThrottle::getDirForCab(int c) {
  // In the speedTable, reverse speeds are negative.
  // See PacketRegister::setThrottle()
  //int spd = 0;
  // Have to special-handle the speed = 0 case
  // since there is no implied direction.
  
  int spd = mainRegs.speedTable[FORCED_REGISTER_NUMBER];
  if (spd == 0) {
    return(dir);
  } else {
    dir = (spd > 0 ? 1 : 0);
    return(dir);
  }
}

void WiThrottle::sendIntroMessage(void) {
  // Send version number of protocol supported
  doPrintln("VN2.0");
  // Send the roster (no roster entries, so 0)
  doPrintln("RL0");
  // Send Power Status
  // check pin SIGNAL_ENABLE_PIN_MAIN
  // PPA0=off PPA1=on PPA2=unknown
  if (digitalRead(SIGNAL_ENABLE_PIN_MAIN) == HIGH) {
    doPrintln("PPA1");
  } else {
    doPrintln("PPA0");
  }
  // Send any defined consists.
  doPrintln("RCC0"); // 0 Consists defined.
  // Send turnout info ...
  doPrintln("PTT]\[Turnouts}|{Turnout]\[Closed}|{2]\[Thrown}|{4");
  // TODO: Send turnouts ("PTL<blah>") and Routes ("PRT<blah>") here.
  // Send the port number
  listTurnouts();
#if COMM_TYPE == 1
  doPrint("PW");
  doPrintln(ETHERNET_PORT);
#endif
}

#define TURNOUT_UNKNOWN 1
#define TURNOUT_CLOSED  2
#define TURNOUT_THROWN  4

void WiThrottle::listTurnouts(void) {
  if (Output::firstOutput != NULL) {
    doPrint("PTL");
    for (int i = 0; i < MAX_DCC_TURNOUTS; i++) {
      if (dccTurnouts[i].id > 0) {
	sprintf(command,"]\\[%d}|{%d}|{%d]", dccTurnouts[i].address, dccTurnouts[i].id, TURNOUT_UNKNOWN);
	doPrint(command);
      }
    }
    /*
    Output *pt = Output::firstOutput;
    while (pt != NULL) {
      sprintf(command, "]\\[%d|%d|%d]", pt->data.id, pt->data.id,
	      (pt->data.oStatus == 0 ? TURNOUT_CLOSED : TURNOUT_THROWN));
      INTERFACE.print(command);
      pt = pt->nextOutput;
    } // while
    */
    doPrintln("]"); // Send close bracket and EOL
  } // if
}
				   
void WiThrottle::handleTurnout(char *s) {
  // Substring of PTAxxx command, starting with byte 3
  int addr = strtol(s+1, NULL, 10);
  for (int i = 0; i < MAX_DCC_TURNOUTS; i++) {
    if (dccTurnouts[i].address == addr) {
      sprintf(command, "a %d 0 %d", addr, (s[0] == 'C' ? 0 : 1));
      SerialCommand::parse(command);
      dccTurnouts[i].tStatus = (s[0] == 'C'? 0 : 1);
      return;
    }
  } // end for loop.
  // If we got here, then it's not a defined DCC turnout.  Might be
  // an Output.  Handle that differently.
  // err... for now don't handle it at all.
}

void WiThrottle::doPrint(char *x) {
  Serial.print(x);
  INTERFACE.print(x);
}

void WiThrottle::doPrintln(char *x) {
  Serial.println(x);
  INTERFACE.println(x);
}

void WiThrottle::doPrint(StringSumHelper& x) {
  Serial.print(x);
  INTERFACE.print(x);
}

void WiThrottle::doPrintln(StringSumHelper& x) {
  Serial.println(x);
  INTERFACE.println(x);
}
