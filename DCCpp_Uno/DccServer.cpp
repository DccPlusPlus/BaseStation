/**********************************************************************

DccServer.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************

DccServer COMMENTS - TBD

**********************************************************************/

#include "DCCpp_Uno.h"
#include "DccServer.h"
#include "EEStore.h"
#include <EEPROM.h>
#include "Comm.h"
#include "Sensor.h"

///////////////////////////////////////////////////////////////////////////////
  
void DccServer::check(){    
  DccServer *tt;
  byte x[2];

  if(serverID>0)
    return;

  if(millis()-checkTime<SERVER_SAMPLE_TIME)     // only check once every few seconds --- no need to clog I2C bus.
    return;

  checkTime=millis();
  
  for(tt=firstDccServer;tt!=NULL;tt=tt->nextDccServer){

    if(!masterStarted){                               // there is at least one server (since we are in this loop), but master not yet started
      Wire.begin();
      masterStarted=true;
    }

    Wire.beginTransmission(tt->data.snum+7);         // check if server is is active
    Wire.write("J");
    Wire.endTransmission();
    Wire.requestFrom(tt->data.snum+7,2);
    x[0]=Wire.read();
    x[1]=Wire.read();

    if(tt->data.snum==x[0]){                         // server found
      tt->reset=(x[1]!=1);                           // check if it was reset (any value except for 1)
      if(tt->reset)                                  // if it was reset, set upLoaded flag to false
        tt->upLoaded=false;                         
      
      if(!tt->active){                               // was previously inactive
        tt->active=true;
        INTERFACE.print("<J");
        INTERFACE.print(tt->data.snum);
        INTERFACE.print(">");
      }
    } else {                                        // server not found
      if(tt->active){                               // was previously active
        tt->active=false;
        INTERFACE.print("<j");
        INTERFACE.print(tt->data.snum);
        INTERFACE.print(">");
      }
    }

    if(tt->active && !tt->upLoaded){               // server is active and there are still objects to upload  
      tt->upLoaded=Sensor::upload(tt);             // upload any new sensor definitions
    }
  } // loop over all DccServers
    
} // DccServer::check

///////////////////////////////////////////////////////////////////////////////

DccServer *DccServer::create(int snum, int v){
  DccServer *tt;
  
  if(firstDccServer==NULL){
    firstDccServer=(DccServer *)calloc(1,sizeof(DccServer));
    tt=firstDccServer;
  } else if((tt=get(snum))==NULL){
    tt=firstDccServer;
    while(tt->nextDccServer!=NULL)
      tt=tt->nextDccServer;
    tt->nextDccServer=(DccServer *)calloc(1,sizeof(DccServer));
    tt=tt->nextDccServer;
  }

  if(tt==NULL){       // problem allocating memory
    if(v==1)
      INTERFACE.print("<X>");
    return(tt);
  }
  
  tt->data.snum=snum;
  tt->active=false;
  tt->upLoaded=false;
  tt->reset=true;

  if(v==1)
    INTERFACE.print("<O>");
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

DccServer* DccServer::get(int n){
  DccServer *tt;
  for(tt=firstDccServer;tt!=NULL && tt->data.snum!=n;tt=tt->nextDccServer);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void DccServer::remove(int n){
  DccServer *tt,*pp;
  
  for(tt=firstDccServer;tt!=NULL && tt->data.snum!=n;pp=tt,tt=tt->nextDccServer);

  if(tt==NULL){
    INTERFACE.print("<X>");
    return;
  }
  
  if(tt==firstDccServer)
    firstDccServer=tt->nextDccServer;
  else
    pp->nextDccServer=tt->nextDccServer;

  free(tt);

  INTERFACE.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::status(){
  DccServer *tt;

  INTERFACE.print("<I");
  INTERFACE.print(serverID);
  INTERFACE.print(">");
    
  for(tt=firstDccServer;tt!=NULL;tt=tt->nextDccServer){
    INTERFACE.print(tt->active?"<J":"<j");
    INTERFACE.print(tt->data.snum);
    INTERFACE.print(">");
  }
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::parse(char *c){
  int n;
  
  switch(sscanf(c,"%d",&n)){

    case 1:                      // argument is serverID where 0=local, [1-120]=server
      if(n>=0 && n<=120){
        INTERFACE.print("<O>");
        init(n);
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

void DccServer::init(int id){

  if(id>=0)       // this is a valid new ID
    serverID=id;    // set new Server ID

  for(DccServer *tt=firstDccServer;tt!=NULL;tt=tt->nextDccServer)      // de-activate any current servers (only has an affect if currently a started master)
    tt->active=false;
  
  Wire.end();     // end current Wire session

  if(serverID==0){          // DCC++ Master
    masterStarted=false;
  } else {                  // DCC++ Server
    serverReset=true;
    Wire.begin(serverID+7);
    Wire.onReceive(receiveWire);
    Wire.onRequest(serverQuery);
  }
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::receiveWire(int nBytes){

  int i,j,k;

  switch(Wire.read()){
   
    case 'S':                      // create a sensor      
      i=Wire.read();
      j=Wire.read();
      k=Wire.read();       
      Sensor::create(i,j,k,0);      // expected 3 bytes: sensor ID, pin, pullup
    break;

    case 'Q':                     // set-up for sensor queries
      Wire.onRequest(Sensor::sensorQuery);
    break;
    
    case 'q':                     // query a specific sensor
      i=Wire.read();
      Sensor::get(i);             // this will set Sensor::lastQueried value
    break;

    case 'J':                     // query server status
      Wire.onRequest(serverQuery);
    break;
    
  }
    
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::serverQuery(){
  byte x[2];

  x[0]=serverID;
  x[1]=serverReset?99:1;
  Wire.write(x,2);
  serverReset=false;
}

///////////////////////////////////////////////////////////////////////////////

DccServer *DccServer::firstDccServer=NULL;
long int DccServer::checkTime=0;
boolean DccServer::masterStarted=false;
byte DccServer::serverID=0;
boolean DccServer::serverReset=true;


