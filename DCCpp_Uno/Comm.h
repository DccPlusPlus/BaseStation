/**********************************************************************

Comm.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#if COMM_TYPE == 1
  #include ETHERNET_LIBRARY
  extern EthernetServer INTERFACE;
#endif  

