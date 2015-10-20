/**********************************************************************

DCCpp_Uno.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#ifndef DCCpp_Uno_h
#define DCCpp_Uno_h

#define  BASE_STATION_VERSION  "UNO_1.0"

// Define the Arduino Pins that control various functions

#define SIGNAL_ENABLE_PIN_MAIN 3
#define SIGNAL_ENABLE_PIN_PROG 11

#define CURRENT_MONITOR_PIN_MAIN A0
#define CURRENT_MONITOR_PIN_PROG A1

#define DCC_SIGNAL_PIN_MAIN 10          // Ardunio Uno
#define DCC_SIGNAL_PIN_PROG 5           // Arduino Uno

#define DIRECTION_MOTOR_CHANNEL_PIN_A 12
#define DIRECTION_MOTOR_CHANNEL_PIN_B 13

// Define number of MAIN TRACK Registers

#define MAX_MAIN_REGISTERS 12

// Define slots for EE_PROM storage of key variables.  EE_TURNOUT must be last since it is starting point for a number of turnouts

enum { EE_TURNOUT };

// If SHOW_PACKETS is set to 1, then for select main operations track commands that modify an internal DCC packet register,
// if printFlag for that command is also set to 1, DCC++ BASE STATION will additionally return the 
// DCC packet contents of the modified register in the following format:

//    <* REG: B1 B2 ... Bn CSUM / REPEAT>
//
//    REG: the number of the main operations track packet register that was modified
//    B1: the first hexidecimal byte of the DCC packet
//    B2: the second hexidecimal byte of the DCC packet
//    Bn: the nth hexidecimal byte of the DCC packet
//    CSUM: a checksum byte that is required to be the final byte in any DCC packet
//    REPEAT: the number of times the DCC packet was re-transmitted to the tracks after its iniital transmission
 
#define SHOW_PACKETS  0       // set to zero to disable printing of every packet for select main operations track commands

#endif


