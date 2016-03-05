/**********************************************************************

DccServer.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************


**********************************************************************/

#include "Dccpp_Uno.h"
#include "DccServer.h"
#include "EEStore.h"

///////////////////////////////////////////////////////////////////////////////

byte DccServer::serverID;

///////////////////////////////////////////////////////////////////////////////
  
void DccServer::status(){

  INTERFACE.print("<J");
  INTERFACE.print(serverID);
  INTERFACE.print(">");
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::parse(char *c){
  int n;
  
  switch(sscanf(c,"%d",&n)){

    case 1:                      // argument is serverID (0=INACTIVE, 1=DCC++ MASTER, 2-255=DCC++ BOOSTER)
      serverID=n;
    break;
        
    case -1:                    // no arguments
      status();
    break;

  }
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::load(){
  DccServer::serverID=EEStore::eeStore->data.serverID;

  #ifdef ARDUINO_AVR_MEGA2560
   if(serverID==1){
     Serial1.begin(115200);
     Serial1.flush();
     }
  #endif 
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::store(){
  EEStore::eeStore->data.serverID=DccServer::serverID;  
}


