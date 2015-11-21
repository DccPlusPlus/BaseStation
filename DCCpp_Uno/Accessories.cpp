/**********************************************************************

Accessories.cpp
COPYRIGHT (c) 2013-2015 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/
/**********************************************************************

DCC++ BASE STATION can keep track of the direction of any turnout that is controlled
by a DCC stationary accessory decoder.  All turnouts, as well as any other DCC accessories
connected in this fashion, can always be operated using the DCC BASE STATION Accessory command:

  <a ADDRESS SUBADDRESS ACTIVATE>

However, this general command simply sends the appropriate DCC instruction packet to the main tracks
to operate connected accessories.  It does not store or retain any information regarding the current
status of that accessory.

UPDATE THIS SECTION !!!!!!!!!!!

To have this sketch store and retain the direction of DCC-connected turnouts, as well as automatically
invoke the required <a> command as needed, define such turnouts in a single global array declared
in DCCpp_Uno.ino using the format:

  Turnout(ID, ADDRESS, SUBADDRESS)

  ID: a unique integer ID (0-32767) for this Turnout
  ADDRESS:  the primary address of the decoder (0-511) used to control this Turnout
  SUBADDRESS: the subaddress of the decoder (0-3) used to control this Turnout

Changes to the direction of Turnouts declared in this fashion can be controlled by the Turnout command:

  <T ID THROW>

When controlled as such, the Arduino updates and stores the direction of each Turnout in EEPROM so
that it is retained even without power.  A list of the current directions of each Turnout is generated
by this sketch whenever the <s> status command is invoked.  This provides an efficient way of initializing
the directions of any Turnouts being monitored or controlled by a separate interface or GUI program.

**********************************************************************/

#include "Accessories.h"
#include "SerialCommand.h"
#include "DCCpp_Uno.h"
#include "EEStore.h"
#include <EEPROM.h>

///////////////////////////////////////////////////////////////////////////////

void Turnout::activate(int s){
  char c[20];
  data.tStatus=(s>0);                                    // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
  sprintf(c,"a %d %d %d",data.address,data.subAddress,data.tStatus);
  SerialCommand::parse(c);
  if(num>0)
    EEPROM.put(num,data.tStatus);
  Serial.print("<H");
  Serial.print(data.id);
  if(data.tStatus==0)
    Serial.print(" 0>");
  else
    Serial.print(" 1>"); 
}

///////////////////////////////////////////////////////////////////////////////

Turnout* Turnout::get(int n){
  Turnout *tt;
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;tt=tt->nextTurnout);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void Turnout::remove(int n){
  Turnout *tt,*pp;
  
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextTurnout);

  if(tt==NULL){
    Serial.print("<X>");
    return;
  }
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);

  Serial.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::show(int n){
  Turnout *tt;

  if(firstTurnout==NULL){
    Serial.print("<X>");
    return;
  }
    
  for(tt=firstTurnout;tt!=NULL;tt=tt->nextTurnout){
    Serial.print("<H");
    Serial.print(tt->data.id);
    if(n==1){
      Serial.print(" ");
      Serial.print(tt->data.address);
      Serial.print(" ");
      Serial.print(tt->data.subAddress);
    }
    if(tt->data.tStatus==0)
       Serial.print(" 0>");
     else
       Serial.print(" 1>"); 
  }
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::parse(char *c){
  int n,s,m;
  Turnout *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
      t=get(n);
      if(t!=NULL)
        t->activate(s);
      else
        Serial.print("<X>");
      break;

    case 3:                     // argument is string with id number of turnout followed by an address and subAddress
      create(n,s,m,1);
    break;

    case 1:                     // argument is a string with id number only
      remove(n);
    break;
    
    case -1:                    // no arguments
      show(1);                  // verbose show
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::load(){
  struct TurnoutData data;
  Turnout *tt;

  for(int i=0;i<EEStore::eeStore->data.nTurnouts;i++){
    EEPROM.get(EEStore::pointer(),data);  
    tt=create(data.id,data.address,data.subAddress);
    tt->data.tStatus=data.tStatus;
    tt->num=EEStore::pointer();
    EEStore::advance(sizeof(tt->data));
  }  
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::store(){
  Turnout *tt;
  
  tt=firstTurnout;
  EEStore::eeStore->data.nTurnouts=0;
  
  while(tt!=NULL){
    tt->num=EEStore::pointer();
    EEPROM.put(EEStore::pointer(),tt->data);
    EEStore::advance(sizeof(tt->data));
    tt=tt->nextTurnout;
    EEStore::eeStore->data.nTurnouts++;
  }
  
}
///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::create(int id, int add, int subAdd, int v){
  Turnout *tt;
  
  if(firstTurnout==NULL){
    firstTurnout=(Turnout *)calloc(1,sizeof(Turnout));
    tt=firstTurnout;
  } else if((tt=get(id))==NULL){
    tt=firstTurnout;
    while(tt->nextTurnout!=NULL)
      tt=tt->nextTurnout;
    tt->nextTurnout=(Turnout *)calloc(1,sizeof(Turnout));
    tt=tt->nextTurnout;
  }

  if(tt==NULL){       // problem allocating memory
    if(v==1)
      Serial.print("<X>");
    return(tt);
  }
  
  tt->data.id=id;
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  if(v==1)
    Serial.print("<O>");
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::firstTurnout=NULL;


