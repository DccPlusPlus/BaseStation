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

void DccServer::upload(Sensor *tt){

  setMaster();                // convert to WIRE MASTER 
  Wire.beginTransmission(8);  // 8 is always the WIRE address of DCC++ MASTER
  Wire.write(tt->active?"Q":"q");
  Wire.write(highByte(tt->data.snum));
  Wire.write(lowByte(tt->data.snum));
  Wire.write(0);                              // dummy byte -- must always have 4 bytes so all transmissions look alike (needed for proper WIRE arbitration)
  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);        // revert back to WIRE SERVER
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::upload(Output *tt){

  setMaster();                // convert to WIRE MASTER 
  Wire.beginTransmission(8);  // 8 is always the WIRE address of DCC++ MASTER
  Wire.write(tt->data.oStatus==0?"y":"Y");          // relay status of local output to DCC++ MASTER
  Wire.write(highByte(tt->data.id));
  Wire.write(lowByte(tt->data.id));
  Wire.write(serverID);
  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);        // revert back to WIRE SERVER
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::upload(RemoteOutput *tt){
   
  setMaster();                                      // convert to WIRE MASTER 
  Wire.beginTransmission(tt->serverID+8);           // use serverID of DCC++ BOOSTER
  Wire.write("Z");                                  // transmit remote OUTPUT command
  Wire.write(highByte(tt->snum));
  Wire.write(lowByte(tt->snum));
  Wire.write(tt->activeDesired?1:0);               // send desired state, but don't set tt->active; wait for return instead
  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);                             // revert back to WIRE SERVER      
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::receiveWire(int nBytes){
  
  char c;
  byte i,j,k;
  int w;

  c=Wire.read();
  i=Wire.read();
  j=Wire.read();
  k=Wire.read();
  
  switch(c){
   
    case 'Q':                      // sensor activated
    case 'q':                      // sensor de-activated    
      RemoteSensor *tt;
      w=word(i,j);      
      tt=RemoteSensor::get(w);     // get remoteSensor
      
      if(tt!=NULL) {                // remoteSensor exists
        tt->active=(c=='Q');       // set active status
      } else {
        tt=RemoteSensor::create(w);   // create remoteSensor
        tt->active=(c=='Q');       // set active status
        if(!tt->active)             // not active, and was just created
          return;                 // do not report inactive sensors when just created
      }
                                 
      INTERFACE.print("<");
      INTERFACE.print(c);
      INTERFACE.print(w);
      INTERFACE.print(">");
    break;

    case 'Y':                      // output activated
    case 'y':                      // output de-activated    
      RemoteOutput *ss;
      w=word(i,j);      
      ss=RemoteOutput::get(w);     // get remoteOutput

      if(ss==NULL)                      // remoteOutput does not yet exist
        ss=RemoteOutput::create(w,k);   // create with output ID and serverID
      
      ss->active=(c=='Y');       // set active status
                                 
      INTERFACE.print("<Y");
      INTERFACE.print(w);
      INTERFACE.print(ss->active?" 1>":" 0>");
    break;

    case 'Z':                     // activate output
      Output *rr;
      w=word(i,j);
      rr=Output::get(w);
      if(rr!=NULL)
        rr->activate(k);
    break;
       
  }
    
}

///////////////////////////////////////////////////////////////////////////////

RemoteSensor *RemoteSensor::create(int snum){
  RemoteSensor *tt;
  
  if(firstSensor==NULL){
    firstSensor=(RemoteSensor *)calloc(1,sizeof(RemoteSensor));
    tt=firstSensor;
  } else if((tt=get(snum))==NULL){
    tt=firstSensor;
    while(tt->nextSensor!=NULL)
      tt=tt->nextSensor;
    tt->nextSensor=(RemoteSensor *)calloc(1,sizeof(RemoteSensor));
    tt=tt->nextSensor;
  }
  
  tt->snum=snum;
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

RemoteSensor *RemoteSensor::get(int n){
  RemoteSensor *tt;
  for(tt=firstSensor;tt!=NULL && tt->snum!=n;tt=tt->nextSensor);
  return(tt); 
}

///////////////////////////////////////////////////////////////////////////////

void RemoteSensor::status(){
  RemoteSensor *tt;
    
  for(tt=firstSensor;tt!=NULL;tt=tt->nextSensor){
    INTERFACE.print(tt->active?"<Q":"<q");
    INTERFACE.print(tt->snum);
    INTERFACE.print(">");
  }
}

///////////////////////////////////////////////////////////////////////////////

RemoteOutput *RemoteOutput::create(int snum, int serverID){
  RemoteOutput *tt;
  
  if(firstOutput==NULL){
    firstOutput=(RemoteOutput *)calloc(1,sizeof(RemoteOutput));
    tt=firstOutput;
  } else if((tt=get(snum))==NULL){
    tt=firstOutput;
    while(tt->nextOutput!=NULL)
      tt=tt->nextOutput;
    tt->nextOutput=(RemoteOutput *)calloc(1,sizeof(RemoteOutput));
    tt=tt->nextOutput;
  }
  
  tt->snum=snum;
  tt->serverID=serverID;
  tt->uploaded=true;
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

RemoteOutput *RemoteOutput::get(int n){
  RemoteOutput *tt;
  for(tt=firstOutput;tt!=NULL && tt->snum!=n;tt=tt->nextOutput);
  return(tt); 
}

///////////////////////////////////////////////////////////////////////////////

void RemoteOutput::status(){
  RemoteOutput *tt;
    
  for(tt=firstOutput;tt!=NULL;tt=tt->nextOutput){
    INTERFACE.print("<Y");
    INTERFACE.print(tt->snum);
    INTERFACE.print(tt->active?" 1>":" 0>");
  }
}

///////////////////////////////////////////////////////////////////////////////

void RemoteOutput::activate(int s){
  activeDesired=(s>0);
  uploaded=false;
}

///////////////////////////////////////////////////////////////////////////////
  
void RemoteOutput::check(){    

  if(DccServer::serverID>0){                                   // this is a DCC++ SERVER - must upload LOCAL output definitions to DCC++ MASTER
    
    for(Output *tt=Output::firstOutput;tt!=NULL;tt=tt->nextOutput){
      if(!tt->uploaded)                                   
        DccServer::upload(tt);
    } 
  } else {                                                     // this is a DCC++ MASTER - must upload any REMOTE OUTPUT commands to DCC++ SERVER 

    for(RemoteOutput *tt=RemoteOutput::firstOutput;tt!=NULL;tt=tt->nextOutput){
      if(!tt->uploaded)                                   
        DccServer::upload(tt);
    }
  }
          
}

///////////////////////////////////////////////////////////////////////////////

byte DccServer::serverID;
RemoteSensor *RemoteSensor::firstSensor=NULL;
RemoteOutput *RemoteOutput::firstOutput=NULL;


