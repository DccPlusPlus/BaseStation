/**********************************************************************

RGB.cpp
COPYRIGHT (c) 2013 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include "RGB.h"
#include "DCCpp_Uno.h"
#include <EEPROM.h>
#include "EEStore.h"
#include "Comm.h"
#include "EggTimer.h"

///////////////////////////////////////////////////////////////////////////////

void RGBLight::parse(char *c){
  int x,y,z,s;
  char type[4];

  if(sscanf(c,"%3s %d %d %d %d",type,&x,&y,&z,&s)<5)        // type="HSV" or "RGB"; s=1 requests store to EEPROM; any other value results in no store
    return;

  if(strcasecmp(type,"HSV")==0)       // H from 0-359, SV each from 0-100
    HSV2RGB(x,y,z);                   // convert from HSV to RGB

    activate(x,y,z,1);                // activate RGB and update static values

    if(s==1)
      EEPROM.put(num,data);           // write new color to EEPROM
      
} // RGBLight::parse

///////////////////////////////////////////////////////////////////////////////

void RGBLight::activate(int r, int g, int b, int u){

  analogWrite(RED_LED,r);
  analogWrite(GREEN_LED,g);
  analogWrite(BLUE_LED,b);

  if(u==1){         // if u=1, update static values
    data.R=r;
    data.G=g;
    data.B=b;    
  }
}
 
///////////////////////////////////////////////////////////////////////////////

void RGBLight::store(){

  EEPROM.put(EEStore::pointer(),data);  
  num=EEStore::pointer();
  EEStore::advance(sizeof(data));
}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::load(){

  EEPROM.get(EEStore::pointer(),data);  
  num=EEStore::pointer();
  EEStore::advance(sizeof(data));
  activate(data.R,data.G,data.B);
      
  pinMode(RED_LED,OUTPUT);
  pinMode(BLUE_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::show(){

  INTERFACE.print("<L");
  INTERFACE.print(data.R);
  INTERFACE.print(" ");
  INTERFACE.print(data.G);
  INTERFACE.print(" ");
  INTERFACE.print(data.B);  
  INTERFACE.print(">");
}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::HSV2RGB(int &x, int& y, int& z){

  float H,S,V;
  float f,p,q,t;

  H=x%360;
  S=y/100.0;
  V=z/100.0;
  
  f=((int)H%60)/60.0;
  p=V*(1.0-S);
  q=V*(1.0-f*S);
  t=V*(1.0-(1.0-f)*S);
          
  switch((x%360)/60){            
    case 0:
      x=V*255.0; y=t*255.0; z=p*255.0; break;
    case 1:
      x=q*255.0; y=V*255.0; z=p*255.0; break;
    case 2:
      x=p*255.0; y=V*255.0; z=t*255.0; break;
    case 3:
      x=p*255.0; y=q*255.0; z=V*255.0; break;
    case 4:
      x=t*255.0; y=p*255.0; z=V*255.0; break;
    case 5:
      x=V*255.0; y=p*255.0; z=q*255.0; break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::blink(int n){

  if(n>0){
    activate(0,0,0,0);                      // turn RGB off, but don't update static values
    timer.setEvent(98,-n);                // set timer for 200ms and call this routine again with parameter = -n
  } else
  if(n<0){
    activate(data.R,data.G,data.B);       // turn RGB back on using known static values
    n++;                                  // increment count (noting n is negative)
    if(n<0)                               // if n has not yet reached zero
      timer.setEvent(98,-n);              // set timer for another 200ms and call this routing again with parameter = -n (which should be a positive number)
  }
}

///////////////////////////////////////////////////////////////////////////////

struct RGBData RGBLight::data;
int RGBLight::num;
EggTimer RGBLight::timer(RGBLight::blink);  


