/**********************************************************************

WiThrottle.h
COPYRIGHT (c) 2017 Mark S. Underwood

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef WITHROTTLE_H
#define WITHROTTLE_H


class WiThrottle {
private:
  //static char command[MAX_COMMAND_LENGTH+1];
  //static int address; // someday will be an array?

protected:
  static void parseHCommand(char *s);
  static void parseNCommand(char *s);
  static void parseMCommand(char *s);
  static void doThrottleCommand(char *key, char *action);
  static byte getFuncByte1(bool t, int f);
  static byte getFuncByte2(bool t, int f);
  static int getRegisterForCab(int c);
  static int getSpeedForCab(int c);
  static int getDirForCab(int c);
  static void handleRequest(char *s);
public:
  static void parseToDCCpp(char *s);
  static bool isWTCommand(char c);
  static void readCommand(char c);
  static void sendIntroMessage(void);
};

#endif // WITHROTTLE_H
