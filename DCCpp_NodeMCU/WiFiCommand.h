/**********************************************************************

WiFiCommand.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef WiFiCommand_h
#define WiFiCommand_h

#define MAX_COMMAND_LENGTH    30
#define MAX_CLIENTS           8

struct WiFiCommand{
  static char commandString[MAX_CLIENTS][MAX_COMMAND_LENGTH+1];
  static void init();
  static void parse(char *);
  static void process();
  static void print(char *);
  static void print(const char * );
  static void print(int);
}; // WiFiCommand

#endif
