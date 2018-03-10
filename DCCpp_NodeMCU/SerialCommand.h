/**********************************************************************

SerialCommand.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef SerialCommand_h
#define SerialCommand_h

#define  MAX_COMMAND_LENGTH         30

struct SerialCommand{
  static char commandString[MAX_COMMAND_LENGTH+1];
  static void parse(char *);
  static void process();
}; // SerialCommand
  
#endif




