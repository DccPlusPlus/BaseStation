/**********************************************************************

DccServer.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************

DCC++ BASE STATION supports the use of multiple Arduinos communicating
via the Arduino's built-in TWI/I2C Interface, utilizing Arduino's WIRE library.
Each Arduino requires a unique SERVER ID from 0 through 119.  This address
is set interactively with the <J ID> command, and saved to EEPROM when
the <E> command is issued.

When multiple Arduinos are used, one acts as the DCC++ MASTER and all others
are DCC++ BOOSTERS.

* The DCC++ MASTER must have a SERVER ID of 0.  
* DCC++ BOOSTERS must have a unique SERVER ID from 1 through 119 

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

void DccServer::init(){
  
  digitalWrite(SCL,HIGH);       // set TWI lines to utilize internal pull-up resistors
  digitalWrite(SDA,HIGH);

  bitClear(TWSR,TWPS0);          // set TWI pre-scaler to 1
  bitClear(TWSR,TWPS1);
  TWBR=72;                       // with pre-scaler set to 1 above, this yields a TWI frequency of 100kHz

  state=READY;                   // set TWI state
  
  TWAR=(serverID+1) << 1;        // set WIRE ADDRESS = SERVER ID + 1

  TWCR=(1<<TWEA) | (1<<TWEN) | (1<<TWIE);   // start SLAVE RECEIVER mode
  
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
  
//  Wire.end();
//  Wire.onReceive(receiveWire);
//  Wire.begin(serverID+1);         // set as WIRE SERVER with ID+1 (yielding address between 1 and 120)
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::setMaster(){

//  Wire.end();
//  Wire.begin();                   // set as WIRE MASTER
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::upload(Sensor *tt){

  setMaster();                // convert to WIRE MASTER 
//  Wire.beginTransmission(1);  // 1 is always the WIRE address of DCC++ MASTER (SERVER ID = 0)
//  Wire.write(tt->active?"Q":"q");
//  Wire.write(highByte(tt->data.snum));
//  Wire.write(lowByte(tt->data.snum));
//  Wire.write(0);                              // dummy byte -- must always have 4 bytes so all transmissions look alike (needed for proper WIRE arbitration)
//  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);        // revert back to WIRE SERVER
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::upload(Output *tt){

  setMaster();                // convert to WIRE MASTER 
//  Wire.beginTransmission(1);  // 1 is always the WIRE address of DCC++ MASTER (SERVER ID = 0)
//  Wire.write(tt->data.oStatus==0?"y":"Y");          // relay status of local output to DCC++ MASTER
//  Wire.write(highByte(tt->data.id));
//  Wire.write(lowByte(tt->data.id));
//  Wire.write(serverID);
//  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);        // revert back to WIRE SERVER
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::upload(RemoteOutput *tt){
   
  setMaster();                                      // convert to WIRE MASTER 
//  Wire.beginTransmission(tt->serverID+1);           // use WIRE ADDRESS of DCC++ BOOSTER (= SERVER ID + 1)
//  Wire.write("Z");                                  // transmit remote OUTPUT command
//  Wire.write(highByte(tt->snum));
//  Wire.write(lowByte(tt->snum));
//  Wire.write(tt->activeDesired?1:0);               // send desired state, but don't set tt->active; wait for return instead
//  tt->uploaded=(Wire.endTransmission()==0);
  setServer(serverID);                             // revert back to WIRE SERVER      
  
}

///////////////////////////////////////////////////////////////////////////////

void DccServer::refresh(){

  if(serverID>0)                              // only applicable for the DCC++ MASTER
    return;
    
  setMaster();                                // convert to WIRE MASTER 
//  Wire.beginTransmission(0);                  // use "general call" WIRE ADDRESS of zero (all SERVERS will respond)
//  Wire.write("G");                            // transmit status refresh command
//  Wire.write(0);                              // dummy byte -- must always have 4 bytes so all transmissions look alike (needed for proper WIRE arbitration)
//  Wire.write(0);                              // dummy byte -- must always have 4 bytes so all transmissions look alike (needed for proper WIRE arbitration)
//  Wire.write(0);                              // dummy byte -- must always have 4 bytes so all transmissions look alike (needed for proper WIRE arbitration)
//  Wire.endTransmission();
  setServer(serverID);                        // revert back to WIRE SERVER      
  
}
///////////////////////////////////////////////////////////////////////////////

void DccServer::check(){

  if(state!=DATA_RECEIVED)        // check if data has been received
    return;
    
  int w;
  
  switch(data[0]){
   
    case 'Q':                      // sensor activated
    case 'q':                      // sensor de-activated    
      RemoteSensor *tt;
      w=word(data[1],data[2]);      
      tt=RemoteSensor::get(w);     // get remoteSensor
      
      if(tt!=NULL) {                // remoteSensor exists
        tt->active=(data[0]=='Q');       // set active status
      } else {
        tt=RemoteSensor::create(w);   // create remoteSensor
        tt->active=(data[0]=='Q');       // set active status
        if(!tt->active)             // not active, and was just created
          break;                 // do not report inactive sensors when just created
      }
                                 
      INTERFACE.print("<");
      INTERFACE.print(char(data[0]));
      INTERFACE.print(w);
      INTERFACE.print(">");
    break;

    case 'Y':                      // output activated
    case 'y':                      // output de-activated    
      RemoteOutput *ss;
      w=word(data[1],data[2]);      
      ss=RemoteOutput::get(w);     // get remoteOutput

      if(ss==NULL)                            // remoteOutput does not yet exist
        ss=RemoteOutput::create(w,data[3]);   // create with output ID and serverID
      
      ss->active=(data[0]=='Y');       // set active status
                                 
      INTERFACE.print("<Y");
      INTERFACE.print(w);
      INTERFACE.print(ss->active?" 1>":" 0>");
    break;

    case 'Z':                     // activate output requested
      Output *rr;
      w=word(data[1],data[2]);
      rr=Output::get(w);
      if(rr!=NULL)
        rr->activate(data[3]);
    break;

   case 'G':                     // status refresh requested - will cause a DCC++ SERVER to re-send all OUTPUTS and SENSORS to DCC++ MASTER

    for(Output *tt=Output::firstOutput;tt!=NULL;tt=tt->nextOutput)      // set upload status for all local OUTPUTS to false
      tt->uploaded=false;

    for(Sensor *tt=Sensor::firstSensor;tt!=NULL;tt=tt->nextSensor)      // set upload status for all local SENSORS to false
      tt->uploaded=false;   
   
   break;
       
  } // switch

  state=READY;
  TWCR=(1<<TWEA) | (1<<TWEN) | (1<<TWIE);   // re-start SLAVE RECEIVER mode

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
volatile byte DccServer::iData;
volatile byte DccServer::data[TWI_BUF_SIZE];
volatile byte DccServer::state;
RemoteSensor *RemoteSensor::firstSensor=NULL;
RemoteOutput *RemoteOutput::firstOutput=NULL;

///////////////////////////////////////////////////////////////////////////////

ISR(TWI_vect){
  switch(TWSR){

    case 0x08:        // MASTER TRANSMITTER
      Serial.println(TWSR,HEX);
      TWDR=1<<1;
      TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWIE);   // send SLA+W
    break;

    case 0x18:
      Serial.println("::18::");
    break;

          
    case 0x60:        // received and acknowledged own SLA+W
    case 0x70:        // received and acknowledged general call    
      DccServer::state=READING;
      DccServer::iData=0;    
      TWCR=(1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);     // set up to receive and acknowledge first byte
      break;

    case 0x80:       // received and acknowledged a byte as Slave Receiver at SLA+W
    case 0x90:       // received and acknowledged a byte as Slave Receiver ar general call address    
      DccServer::data[DccServer::iData++]=TWDR;
      
      if(DccServer::iData<TWI_BUF_SIZE)
        TWCR=(1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);   // set up to receive and acknowledge next byte
      else
        TWCR=(1<<TWINT) | (0<<TWEA) | (1<<TWEN) | (1<<TWIE);   // set up to continue receiving data but do not acknowledge any additional bytes
    break;

    case 0x88:       // received but did not acknowledge a byte as Slave Receiver at SLA+W (buffer full)
    case 0x98:       // received but did not acknowledge a byte as Slave Receiver ar general call address (buffer full)
    case 0xA0:       // STOP received while in Slave Receiver mode    
      TWCR=(1<<TWINT);    // switch to non-addressable Slave mode (data byte must be processed before re-starting Slave Receiver mode)
      DccServer::state=DATA_RECEIVED;
    break;

    default:
      Serial.println(TWSR,HEX);
      TWCR=(1<<TWINT);    // switch to non-addressable Slave mode (data byte must be processed before re-starting Slave Receiver mode)
    break;
  }
}

