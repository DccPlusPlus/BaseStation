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
#include "SerialCommand.h"

///////////////////////////////////////////////////////////////////////////////

byte DccServer::serverID;
boolean DccServer::isMaster=false;

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

void DccServer::busRead(char *c){
  int r;
  char s[MAX_COMMAND_LENGTH];
  
  sscanf(c,"%d %[A-Za-z-0-9 ]",&r,s);

  if(r==serverID)
    SerialCommand::parse(s);
  else if(!isMaster)
    busWrite(r,s);
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::busWrite(int r, char *c){

  if(isMaster){
    #ifdef ARDUINO_AVR_MEGA2560
    Serial1.print("<#");
    Serial1.print(r);
    Serial1.print(" ");
    Serial1.print(c);
    Serial1.print(">");
    #endif
  } else {
    Serial.print("<#");
    Serial.print(r);
    Serial.print(" ");
    Serial.print(c);
    Serial.print(">");
  }
    
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::load(){
  DccServer::serverID=EEStore::eeStore->data.serverID;

  #ifdef ARDUINO_AVR_MEGA2560
   if(serverID==1){
     isMaster=true; 
     Serial1.begin(115200);
     Serial1.flush();
     }
  #endif 
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::store(){
  EEStore::eeStore->data.serverID=DccServer::serverID;  
}


