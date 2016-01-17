/**********************************************************************

Accessories.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

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
invoke the required <a> command as needed, first define/edit/delete such turnouts using the following
variations of the "T" command:

  <T ID ADDRESS SUBADDRESS>:   creates a new turnout ID, with specified ADDRESS and SUBADDRESS
                               if turnout ID already exists, it is updated with specificed ADDRESS and SUBADDRESS
                               returns: <O> if successful and <X> if unsuccessful (e.g. out of memory)

  <T ID>:                      deletes definition of turnout ID
                               returns: <O> if successful and <X> if unsuccessful (e.g. ID does not exist)

  <T>:                         lists all defined turnouts
                               returns: <H ID ADDRESS SUBADDRESS THROW> for each defined turnout or <X> if no turnouts defined
  
where

  ID: the numeric ID (0-32767) of the turnout to control
  ADDRESS:  the primary address of the decoder controlling this turnout (0-511)
  SUBADDRESS: the subaddress of the decoder controlling this turnout (0-3)

Once all turnouts have been properly defined, use the <E> command to store their definitions to EEPROM.
If you later make edits/additions/deletions to the turnout definitions, you must invoke the <E> command if you want those
new definitions updated in the EEPROM.  You can also clear everything stored in the EEPROM by invoking the <e> command.

To "throw" turnouts that have been defined use:

  <T ID THROW>:                sets turnout ID to either the "thrown" or "unthrown" position
                               returns: <H ID THROW>, or <X> if turnout ID does not exist

where

  ID: the numeric ID (0-32767) of the turnout to control
  THROW: 0 (unthrown) or 1 (thrown)

When controlled as such, the Arduino updates and stores the direction of each Turnout in EEPROM so
that it is retained even without power.  A list of the current directions of each Turnout in the form <H ID THROW> is generated
by this sketch whenever the <s> status command is invoked.  This provides an efficient way of initializing
the directions of any Turnouts being monitored or controlled by a separate interface or GUI program.

**********************************************************************/

#include "Accessories.h"
#include "SerialCommand.h"
#include "DCCpp_Uno.h"
#include "EEStore.h"
#include <EEPROM.h>
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

void Turnout::activate(int s){
  char c[20];
  data.tStatus=(s>0);                                    // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
  sprintf(c,"a %d %d %d",data.address,data.subAddress,data.tStatus);
  SerialCommand::parse(c);
  if(num>0)
    EEPROM.put(num,data.tStatus);
  INTERFACE.print("<H");
  INTERFACE.print(data.id);
  if(data.tStatus==0)
    INTERFACE.print(" 0>");
  else
    INTERFACE.print(" 1>"); 
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
    INTERFACE.print("<X>");
    return;
  }
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);

  INTERFACE.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::show(int n){
  Turnout *tt;

  if(firstTurnout==NULL){
    INTERFACE.print("<X>");
    return;
  }
    
  for(tt=firstTurnout;tt!=NULL;tt=tt->nextTurnout){
    INTERFACE.print("<H");
    INTERFACE.print(tt->data.id);
    if(n==1){
      INTERFACE.print(" ");
      INTERFACE.print(tt->data.address);
      INTERFACE.print(" ");
      INTERFACE.print(tt->data.subAddress);
    }
    if(tt->data.tStatus==0)
       INTERFACE.print(" 0>");
     else
       INTERFACE.print(" 1>"); 
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
        INTERFACE.print("<X>");
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
      INTERFACE.print("<X>");
    return(tt);
  }
  
  tt->data.id=id;
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  if(v==1)
    INTERFACE.print("<O>");
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::firstTurnout=NULL;


