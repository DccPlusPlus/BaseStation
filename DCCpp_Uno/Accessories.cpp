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
#include <EEPROM.h>

///////////////////////////////////////////////////////////////////////////////

Turnout::Turnout(int id, int add, int subAdd){
  this->id=id;
  this->address=add;
  this->subAddress=subAdd;
  this->num=nTurnouts;
  if(EEPROM.read(EE_TURNOUT+num)==0)
    tStatus=0;
  else
    tStatus=1;
  nTurnouts++;
} // Turnout::Turnout

///////////////////////////////////////////////////////////////////////////////

void Turnout::activate(int s){
  char c[20];
  tStatus=(s>0);                                    // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
  sprintf(c,"a %d %d %d",address,subAddress,tStatus);
  SerialCommand::parse(c);
  EEPROM.write(EE_TURNOUT+num,tStatus);
  Serial.print("<H");
  Serial.print(id);
  if(tStatus==0)
    Serial.print(" 0>");
  else
    Serial.print(" 1>"); 
}

///////////////////////////////////////////////////////////////////////////////

Turnout* Turnout::get(int n){
  for(int i=0;i<nTurnouts;i++){
    if(turnouts[i].id==n)
      return(turnouts+i);
    }
  return(NULL);
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::parse(char *c){            // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
  int n,s;
  Turnout *t;
  if(sscanf(c,"%d %d",&n,&s)==2){
    t=get(n);
    if(t!=NULL)
      t->activate(s);
  }
}

///////////////////////////////////////////////////////////////////////////////

int Turnout::nTurnouts=0;

