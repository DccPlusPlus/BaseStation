/**********************************************************************

Comm.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include <SPI.h>
#include <EthernetV2_0.h>

#if (COMM_TYPE == 1) || (COMM_TYPE == 2)
  extern EthernetServer INTERFACE;
#endif  

