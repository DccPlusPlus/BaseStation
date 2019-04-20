/**********************************************************************

RemoteSensor.cpp
COPYRIGHT (c) 2018 Dan Worth

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************

DCC++ BASE STATION supports remote RemoteSensor inputs that are connected via a
WiFi connection. Remote RemoteSensors are dynamically created during startup or by a
remote RemoteSensor reporting its state.

During startup, the base station scans for Access Points that have a name
starting with REMOTE_RemoteSensorS_PREFIX defined in Config.h, ie: "RemoteSensor01". If no
Access Points are found matching this prefix during startup they will be created
automatically when the RemoteSensor reports its state to the base station.

Note: Remote RemoteSensors should not maintain a persistent connection. Instead they
should connect when a change occurs that should be reported. It is not necessary
for Remote RemoteSensors to report when they are INACTIVE. If a Remote RemoteSensor does not
report within REMOTE_RemoteSensorS_DECAY milliseconds the base station will
automatically transition the Remote RemoteSensor to INACTIVE state if it was
previously ACTIVE.

To have this sketch monitor one or more for remote RemoteSensor triggers, first define/edit/delete
RemoteSensor definitions using the following variation of the "RS" command:

  <RS ID STATE>:      Informs the base station of the status of a remote RemoteSensor.
  <RS ID>:            Deletes remote RemoteSensor ID.
  <RS>:               Lists all defined remote RemoteSensors.
                      returns: <RS ID STATE> for each defined remote RemoteSensor or
                      <X> if no remote RemoteSensors have been defined/found.
where

  ID:     the numeric ID (0-32667) of the remote RemoteSensor.
  STATE:  State of the RemoteSensors, zero is INACTIVE, non-zero is ACTIVE.
          Usage is remote RemoteSensor dependent.

If a Remote RemoteSensor Pin is found to have transitioned from one state to another, one of the following messages are generated:

  <Q ID>     - for transition of RemoteSensor ID from HIGH state to LOW state (i.e. the RemoteSensor is triggered)
  <q ID>     - for transition of RemoteSensor ID from LOW state to HIGH state (i.e. the RemoteSensor is no longer triggered)

Depending on whether the physical RemoteSensor is acting as an "event-trigger" or a "detection-RemoteSensor," you may
decide to ignore the <q ID> return and only react to <Q ID> triggers.

**********************************************************************/
#include <ESP8266WiFi.h>
#include "Config.h"
#include "DCCpp_NodeMCU.h"
#include "RemoteSensor.h"
#include "EEStore.h"
#include "WiFiCommand.h"
#include <EEPROM.h>

///////////////////////////////////////////////////////////////////////////////
  
void RemoteSensor::check(){    
  RemoteSensor *tt;
  Sensor  *stt;

  for(tt=firstRemoteSensor;tt!=NULL;tt=tt->nextRemoteSensor)
    if( millis() > tt->lastUpdate + REMOTE_SENSORS_DECAY )
      if( stt = Sensor::get( tt->data.snum + REMOTE_SENSORS_FIRST_SENSOR ) )
        stt->signal = 1.0;  // set to inactive on the next sensor check
      else
        Serial.println( "sensor not found after timeout" );

} // RemoteSensor::check

///////////////////////////////////////////////////////////////////////////////

RemoteSensor *RemoteSensor::create(int snum, int value ){
  RemoteSensor *tt;
  Sensor *stt;
  
  if(firstRemoteSensor==NULL){
    firstRemoteSensor=(RemoteSensor *)calloc(1,sizeof(RemoteSensor));
    tt=firstRemoteSensor;
  } else if((tt=get(snum))==NULL){
    tt=firstRemoteSensor;
    while(tt->nextRemoteSensor!=NULL)
      tt=tt->nextRemoteSensor;
    tt->nextRemoteSensor=(RemoteSensor *)calloc(1,sizeof(RemoteSensor));
    tt=tt->nextRemoteSensor;
  }

  if(tt==NULL){       // problem allocating memory
    WiFiCommand::print("<X>");
    return(tt);
  }

  tt->lastUpdate = millis();
  tt->data.snum=snum;
  tt->data.value=value;
  if( !(stt = Sensor::get( snum + REMOTE_SENSORS_FIRST_SENSOR ) ) )
    stt = Sensor::create(snum + REMOTE_SENSORS_FIRST_SENSOR, -1, 0, 0 );
  
  if( stt ) {
    stt->active=value?false:true;  // force a state change on the next sensor check
    stt->signal=value?0.0:1.0;
  }
  else
    Serial.println( "no sensor created or found" );
  WiFiCommand::print("<O>");
  return(tt);
}

///////////////////////////////////////////////////////////////////////////////

RemoteSensor* RemoteSensor::get(int n){
  RemoteSensor *tt;
  for(tt=firstRemoteSensor;tt!=NULL && tt->data.snum!=n;tt=tt->nextRemoteSensor);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void RemoteSensor::remove(int n){
  RemoteSensor *tt,*pp;
  
  for(tt=firstRemoteSensor;tt!=NULL && tt->data.snum!=n;pp=tt,tt=tt->nextRemoteSensor);

  if(tt==NULL){
    WiFiCommand::print("<X>");
    return;
  }
  Sensor::remove( tt->data.snum + REMOTE_SENSORS_FIRST_SENSOR ); 
  
  if(tt==firstRemoteSensor)
    firstRemoteSensor=tt->nextRemoteSensor;
  else
    pp->nextRemoteSensor=tt->nextRemoteSensor;


  free(tt);

  WiFiCommand::print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void RemoteSensor::show(){
  RemoteSensor *tt;

  if(firstRemoteSensor==NULL){
    WiFiCommand::print("<X>");
    return;
  }
    
  for(tt=firstRemoteSensor;tt!=NULL;tt=tt->nextRemoteSensor){
    WiFiCommand::print("<RS");
    WiFiCommand::print(tt->data.snum);
    WiFiCommand::print(" ");
    WiFiCommand::print(tt->data.value);
    WiFiCommand::print(">");
  }
}

///////////////////////////////////////////////////////////////////////////////

void RemoteSensor::parse(char *c){
  int n,s,m;
  RemoteSensor *t;
  
  switch(sscanf(c,"%d %d",&n,&s)){
    
    case 2:                     // argument is string with id number of RemoteSensor followed by a value
      create(n,s);
    break;

    case 1:                     // argument is a string with id number only
      remove(n);
    break;
    
    case -1:                    // no arguments
      show();
    break;

    default:                     // invalid number of arguments
      WiFiCommand::print("<X>");
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

RemoteSensor *RemoteSensor::firstRemoteSensor=NULL;

