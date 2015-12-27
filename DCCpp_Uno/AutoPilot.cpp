/**********************************************************************

AutoPilot.cpp
COPYRIGHT (c) 2013 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include "Accessories.h"
#include "AutoPilot.h"
#include "SerialCommand.h"
#include "DCCpp_Uno.h"
#include "RGB.h"
#include "EggTimer.h"
#include "EEStore.h"
#include <EEPROM.h>

// CREATE A LIST OF CABS TO USE FOR AUTOPILOT ROUTINE

Route spiral(9,(int[]){-10,-20,-30,-40,-50,-8,-5,4,-7});

Cab cabs[]={
  Cab(1202,6,144,129,128,28,25,22,16,11,13,6,new Route(5,(int[]){-9,6,-3,-2,1})),
  Cab(8601,3,144,144,128,77,46,34,30,11,12,6,new Route(5,(int[]){9,6,-3,-2,1})),
  Cab(1506,7,133,148,128,61,42,30,25,11,9,6,new Route(4,(int[]){-6,-3,-2,1})),
  Cab(54,5,144,132,132,59,14,11,8,11,8,6,new Route(3,(int[]){3,-2,1})),
  Cab(6021,4,144,129,128,55,25,25,15,11,7,6,new Route(1,(int[]){-1})),
  Cab(2004,1,144,129,128,100,50,50,45,3,14,6,new Route(1,(int[]){10})),
  Cab(622,2,144,129,128,53,30,22,15,4,10,11,new Route(1,(int[]){7}))
};

///////////////////////////////////////////////////////////////////////////////

enum {
  POWER_ON=100, CYCLE_CAB_LIGHTS, SELECTED_CAB_BLINK_OFF, SELECTED_CAB_BLINK_ON, SET_SPIRAL_TURNOUTS,
  START_SELECTED_CAB, INTIALIZE, DEPARTURE_SEQUENCE, ARRIVAL_SEQUENCE, SET_SPIRAL_TURNOUTS_COMPLETED, SET_PARKING_ROUTE_COMPLETED
  };

///////////////////////////////////////////////////////////////////////////////

Cab::Cab(int num, byte reg, byte lights, byte reverseLights, byte noLights, byte fullSpeed, byte slowSpeed, byte reverseSpeed, byte reverseSlowSpeed,
  byte parkingSensor, byte sidingSensor, byte prepareToParkSensor, Route *route){

  this->num=num;
  this->reg=reg;
  this->lights=lights;
  this->reverseLights=reverseLights;
  this->noLights=noLights;
  this->fullSpeed=fullSpeed;
  this->slowSpeed=slowSpeed;
  this->reverseSpeed=reverseSpeed;
  this->reverseSlowSpeed=reverseSlowSpeed;
  this->parkingSensor=parkingSensor;
  this->sidingSensor=sidingSensor;
  this->prepareToParkSensor=prepareToParkSensor;
  this->route=route;  
  nCabs++;
  };
  
///////////////////////////////////////////////////////////////////////////////

Route::Route(int n, int *t){
  this->nTurnouts=n;
  this->t=t;
};
 
///////////////////////////////////////////////////////////////////////////////

void Route::setTurnout(int s){
  Turnout::get(abs(cRoute->t[cTurnout]))->activate((cRoute->t[cTurnout]>0)?s:1-s);
  cTurnout++;
  if(cTurnout<cRoute->nTurnouts)
    routeTimer.setEvent(98,s);      // wait 200 ms between throwing turnouts
  else if(eFunc!=NULL)
    eFunc(callBack);
}

///////////////////////////////////////////////////////////////////////////////

void Route::activate(Route *r, int s, void (*eF)(int), int cB){
  eFunc=eF;
  callBack=cB;
  cTurnout=0;
  cRoute=r;
  setTurnout(s);    // s=1 sets all turnouts in direction of route; s=0 sets all turnouts in opposite direction of route
}

///////////////////////////////////////////////////////////////////////////////

void AutoPilot::process(int snum){
            
  char s[20];
  
  if(!enabled)
    return;
    
  switch(snum){

    case INPUT_BUTTON:
      if(status==0){
        autoPilotTimer.setEvent(490,POWER_ON);               
      } else
      
      if(status==1){
        activeStatus=1;
        status=2;
        for(int i=0;i<Cab::nCabs;i++){
          sprintf(s,"f %d %d",cabs[i].num,cabs[i].noLights);            // turn lights off all cabs
          SerialCommand::parse(s);
        }
        selectedCab=(selectedCab+Cab::nCabs-1)%Cab::nCabs;              // start with last selected cab minus one (will be a valid selectedCab even if random value in EEPROM)
        autoPilotTimer.setEvent(49,CYCLE_CAB_LIGHTS);  
      } else
      
      if(status==2){
        status=3;
        autoPilotTimer.setEvent(49,SELECTED_CAB_BLINK_OFF);
      } else
      
      if(status<14){
        status=20;
        autoPilotTimer.setEvent(49,SET_SPIRAL_TURNOUTS);
      } else
      
      if(status>=40){
        program=1-program;            // if flip program from 1 to 0 or vice versa
        RGBLight::blink(program+1);
      }
      
    break;
    
    case POWER_ON:
      SerialCommand::parse("1");
      autoPilotTimer.setEvent(490,INTIALIZE);
    break;
    
    case INTIALIZE:
        for(int i=0;i<Cab::nCabs;i++){
          sprintf(s,"f %d %d",cabs[i].num,cabs[i].lights);              // turn lights on all cabs (some cabs only change light status on trigger, so start with them all on)
          SerialCommand::parse(s);
          sprintf(s,"f %d %d",cabs[i].num,cabs[i].noLights);            // turn lights off all cabs
          SerialCommand::parse(s);
        }
        sprintf(s,"f 54 161");       // start up cab 54 sound
        SerialCommand::parse(s);
//        soundEffectTimer.setEvent(9800,1);
      status=1;    
    break;
    
    case CYCLE_CAB_LIGHTS:
      sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].noLights);                // turn lights off current cab
      SerialCommand::parse(s);
      selectedCab=(selectedCab+1)%Cab::nCabs;   // select next cab
      EEPROM.put(eeNum,selectedCab);
      sprintf(s,"t %d %d 0 1",cabs[selectedCab].reg,cabs[selectedCab].num);              // set direction to forward
      SerialCommand::parse(s);
      sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].lights);  // turn cab light on
      SerialCommand::parse(s);
      autoPilotTimer.setEvent(1470,CYCLE_CAB_LIGHTS);                                 // start timer to select next cab
    break;  
    
    case SELECTED_CAB_BLINK_OFF:
      sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].noLights);                // turn lights off current cab
      SerialCommand::parse(s);
      autoPilotTimer.setEvent(245,SELECTED_CAB_BLINK_ON);                                 // start timer to turn back on this cab's light
    break;  

    case SELECTED_CAB_BLINK_ON:
      sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].lights);  // turn cab light on
      SerialCommand::parse(s);
      status++;
      if(status<14){
        autoPilotTimer.setEvent(245,SELECTED_CAB_BLINK_OFF);                                 // start timer to turn back off this cab's light
      }else{
        status=2;
        autoPilotTimer.setEvent(49,CYCLE_CAB_LIGHTS);  
      }        
    break;  
    
    case SET_SPIRAL_TURNOUTS:
      Route::activate(&spiral,1,process,SET_SPIRAL_TURNOUTS_COMPLETED);
    break;     

    case SET_SPIRAL_TURNOUTS_COMPLETED:
      special(DEPARTURE_SEQUENCE,cabs[selectedCab].num);
    break;     

    case START_SELECTED_CAB:
      sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].lights);  // turn cab light on
      SerialCommand::parse(s);
      sprintf(s,"t %d %d %d 1",cabs[selectedCab].reg,cabs[selectedCab].num,cabs[selectedCab].fullSpeed);     // set full speed
      SerialCommand::parse(s);
      status=40;
    break;  
        
    case SET_PARKING_ROUTE_COMPLETED:
        sprintf(s,"t %d %d %d 0",cabs[selectedCab].reg,cabs[selectedCab].num,cabs[selectedCab].reverseSpeed);     // set reverse speed
        SerialCommand::parse(s);
        status=70;       
    break;        

    default:
      if(status==40 && snum==cabs[selectedCab].prepareToParkSensor){
        status=50;
      } else
      if(status==50 && snum==cabs[selectedCab].parkingSensor){
        special(ARRIVAL_SEQUENCE,cabs[selectedCab].num);
        sprintf(s,"t %d %d %d 1",cabs[selectedCab].reg,cabs[selectedCab].num,cabs[selectedCab].slowSpeed);     // set slow speed
        SerialCommand::parse(s);
        status=51;
      } else
      if(status==51 && snum==cabs[selectedCab].parkingSensor){
        sprintf(s,"t %d %d 0 0",cabs[selectedCab].reg,cabs[selectedCab].num);     // stop cab, set reverse direction
        SerialCommand::parse(s);
        sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].reverseLights);  // turn reverse cab light on
        SerialCommand::parse(s);
        status=60;
        Route::activate(cabs[selectedCab].route,1,process,SET_PARKING_ROUTE_COMPLETED);
      } else
      if(status==70 &&  snum==cabs[selectedCab].sidingSensor){
        sprintf(s,"t %d %d %d 0",cabs[selectedCab].reg,cabs[selectedCab].num,cabs[selectedCab].reverseSlowSpeed);     // set reverse slow speed
        SerialCommand::parse(s);
        status=71;
      } else
      if(status==71 &&  snum==cabs[selectedCab].sidingSensor){
        sprintf(s,"t %d %d -1 1",cabs[selectedCab].reg,cabs[selectedCab].num);     // stop cab, set forward direction
        SerialCommand::parse(s);
        if(program==1){
          status=2;
          autoPilotTimer.setEvent(49,CYCLE_CAB_LIGHTS);
        } else
        if(program==0){
          sprintf(s,"f %d %d",cabs[selectedCab].num,cabs[selectedCab].noLights);                // turn lights off current cab
          SerialCommand::parse(s);
          selectedCab=(selectedCab+random(1,Cab::nCabs))%Cab::nCabs;   // select random next cab, but do not repeat this one
          EEPROM.put(eeNum,selectedCab);
          status=20;
          autoPilotTimer.setEvent(49,SET_SPIRAL_TURNOUTS);
        }
      }
    break;

  } // switch
  
} // AutoPilot::process
  
///////////////////////////////////////////////////////////////////////////////

void AutoPilot::special(int s, int cab){

  switch(s){
    
    case DEPARTURE_SEQUENCE:
      if(cab==54){
        SerialCommand::parse("f54 180");
        SerialCommand::parse("f54 176");        
        SerialCommand::parse("f54 222 4");
        SerialCommand::parse("f54 222 0");
        autoPilotTimer.setEvent(1471,START_SELECTED_CAB);
      } else {
        autoPilotTimer.setEvent(98,START_SELECTED_CAB);
      }
    break;

    case ARRIVAL_SEQUENCE:
      if(cab==54){
        SerialCommand::parse("f54 222 4");
        SerialCommand::parse("f54 222 0");
      }
    break;
    
  } // switch
} // special

///////////////////////////////////////////////////////////////////////////////

void AutoPilot::load(){

  EEPROM.get(EEStore::pointer(),selectedCab);  
  eeNum=EEStore::pointer();                         // address of EEPROM to store selectedCab
  EEStore::advance(sizeof(selectedCab));
}

///////////////////////////////////////////////////////////////////////////////

void AutoPilot::store(){

  EEPROM.put(EEStore::pointer(),selectedCab);  
  eeNum=EEStore::pointer();
  EEStore::advance(sizeof(selectedCab));
}

///////////////////////////////////////////////////////////////////////////////

int AutoPilot::enabled=1;
int AutoPilot::activeStatus=0;
int AutoPilot::status=0;
int AutoPilot::program=0;
int AutoPilot::selectedCab;
int AutoPilot::eeNum;
int Cab::nCabs=0;
EggTimer AutoPilot::autoPilotTimer(AutoPilot::process);  
EggTimer Route::routeTimer(Route::setTurnout);  
int Route::cTurnout;
Route *Route::cRoute;
int Route::callBack;
void (* Route::eFunc)(int);



