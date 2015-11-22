/**********************************************************************

DCCpp_Uno.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#ifndef DCCpp_Uno_h
#define DCCpp_Uno_h

#if defined  ARDUINO_AVR_UNO
#define ARDUINO_TYPE    "UNO"

#define DCC_SIGNAL_PIN_MAIN 10          // Ardunio Uno
#define DCC_SIGNAL_PIN_PROG 5           // Arduino Uno

#elif defined  ARDUINO_AVR_MEGA2560
#define ARDUINO_TYPE    "MEGA"

#define DCC_SIGNAL_PIN_MAIN 12          // Arduino Mega
#define DCC_SIGNAL_PIN_PROG 4           // Arduino Mega

#else

#error CAN'T COMPILE - DCC++ ONLY WORKS WITH AN ARDUINO UNO OR AN ARDUINO MEGA 2560
#endif

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE MOTOR SHIELD PINS - SET MOTOR_SHIELD_TYPE ACCORDING TO THE FOLLOWING TABLE:
//
//  0 = ARDUINO MOTOR SHIELD          (MAX 18V/2A PER CHANNEL)
//  1 = POLOLU MC33926 MOTOR SHIELD   (MAX 28V/3A PER CHANNEL)

#define MOTOR_SHIELD_TYPE   0

/////////////////////////////////////////////////////////////////////////////////////

#if MOTOR_SHIELD_TYPE == 0

#define MOTOR_SHIELD_NAME "ARDUINO MOTOR SHIELD"

#define SIGNAL_ENABLE_PIN_MAIN 3
#define SIGNAL_ENABLE_PIN_PROG 11

#define CURRENT_MONITOR_PIN_MAIN A0
#define CURRENT_MONITOR_PIN_PROG A1

#define DIRECTION_MOTOR_CHANNEL_PIN_A 12
#define DIRECTION_MOTOR_CHANNEL_PIN_B 13

#elif MOTOR_SHIELD_TYPE == 1

#define MOTOR_SHIELD_NAME "POLOLU MC33926 MOTOR SHIELD"

#define SIGNAL_ENABLE_PIN_MAIN 9
#define SIGNAL_ENABLE_PIN_PROG 11

#define CURRENT_MONITOR_PIN_MAIN A0
#define CURRENT_MONITOR_PIN_PROG A1

#define DIRECTION_MOTOR_CHANNEL_PIN_A 7
#define DIRECTION_MOTOR_CHANNEL_PIN_B 8

#else

#error CAN'T COMPILE - PLEASE SELECT A PROPER MOTOR SHIELD TYPE
#endif

// Define number of MAIN TRACK Registers

#define MAX_MAIN_REGISTERS 12

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


