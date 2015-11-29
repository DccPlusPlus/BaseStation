/**********************************************************************

Comm.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include <SPI.h>
#include <EthernetV2_0.h>                      // FOR SEEED STUDIO ETHERNET SHIELD V2.0

#if COMM_TYPE == 1
  extern EthernetServer INTERFACE;
#endif  

