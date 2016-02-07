/**********************************************************************

DccServer.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp.h"
#include "Comm.h"
#include "DccServer.h"
#include "EEStore.h"

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

    case 1:                      // argument is serverID (1-120)
      if(n>=1 && n<=120){
        INTERFACE.print("<O>");
        setServer(n);
      } else {
        INTERFACE.print("<X>");
      }
    break;
        
    case -1:                    // no arguments
      status();
    break;

  }
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::load(){
   DccServer::serverID=EEStore::eeStore->data.serverID;  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::store(){
  EEStore::eeStore->data.serverID=DccServer::serverID;  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::setServer(int id){

  serverID=id;
  
  Wire.end();
  Wire.onReceive(receiveWire);
  Wire.begin(serverID+7);         // set as SERVER with ID+7 (yielding address between 8 and 127
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::setMaster(){

  Wire.end();
  Wire.begin();                   // set as MASTER
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::receiveWire(int nBytes){

  int i,j,k;

  switch(Wire.read()){
   
    case 'S':                      // create a sensor      
      i=Wire.read();
      j=Wire.read();
      k=Wire.read();       
    break;
    
  }
    
}

///////////////////////////////////////////////////////////////////////////////

byte DccServer::serverID=1;

