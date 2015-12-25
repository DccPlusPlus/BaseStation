/**********************************************************************

Config.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE MOTOR_SHIELD_TYPE ACCORDING TO THE FOLLOWING TABLE:
//
//  0 = ARDUINO MOTOR SHIELD          (MAX 18V/2A PER CHANNEL)
//  1 = POLOLU MC33926 MOTOR SHIELD   (MAX 28V/3A PER CHANNEL)

#define MOTOR_SHIELD_TYPE   0

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE NUMBER OF MAIN TRACK REGISTER

#define MAX_MAIN_REGISTERS 12

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE COMMUNICATIONS INTERFACE TYPE
//
//  0 = Built-in Serial Port
//  1 = Arduino Ethernet/SD Card Shield

#define COMM_TYPE   1

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE NAME OF ETHERNET LIBRARY TO INCLUDE (DIFFERENT SHIELDS MAY USE THEIR OWN LIBRARIES)

//#define ETHERNET_LIBRARY  <EthernetV2_0.h>        // https://github.com/Seeed-Studio/Ethernet_Shield_W5200
#define ETHERNET_LIBRARY  <Ethernet2.h>        // https://github.com/arduino-org/Arduino

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE PORT TO USE FOR ETHERNET COMMUNICATIONS INTERFACE
//

#define ETHERNET_PORT 2560

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE MAC ADDRESS ARRAY FOR ETHERNET COMMUNICATIONS INTERFACE
//

#define MAC_ADDRESS {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF }

/////////////////////////////////////////////////////////////////////////////////////

