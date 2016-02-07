/**********************************************************************

DccServer.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************

DCC++ BASE STATION supports the use of multiple Arduinos communicating
via the Arduino's built-in TWI/I2C Interface, utilizing Arduino's WIRE library.
Each Arduino requiures a unique WIRE address from 0 through 119.  This address
is set interactively with the <J ID> command, and saved to EEPROM when
the <E> command is issued.

When multiple Arduinos are used, one acts as the DCC++ MASTER and all others
are DCC++ BOOSTERS.

* The DCC++ MASTER must have a WIRE address of 0.  
* DCC++ BOOSTERS must have a unique WIRE address from 1 through 119 

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

    case 1:                      // argument is serverID (0=DCC++ MASTER, 1-119=DCC++ BOOSTER)
      if(n>=0 && n<=119){
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
  Wire.begin(serverID+8);         // set as WIRE SERVER with ID+8 (yielding address between 8 and 127)
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::setMaster(){

  Wire.end();
  Wire.begin();                   // set as WIRE MASTER
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

