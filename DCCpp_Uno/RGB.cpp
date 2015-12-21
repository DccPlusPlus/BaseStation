/**********************************************************************

RGB.cpp
COPYRIGHT (c) 2013 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino Uno 

**********************************************************************/

#include "RGB.h"
#include "DCCpp_Uno.h"
//#include "AutoTimer.h"
//#include <EEPROM.h>

//extern AutoTimer rgbBlinker;
//extern AutoTimer soundEffectTimer;

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
      store();                        // write to EEPROM
      
} // RGBLight::parse

///////////////////////////////////////////////////////////////////////////////

void RGBLight::activate(int r, int g, int b, int u){

  analogWrite(RED_LED,r);
  analogWrite(GREEN_LED,g);
  analogWrite(BLUE_LED,b);

  if(u==1){         // if u=1, update static values
    rValue=r;
    gValue=g;
    bValue=b;    
  }
}
 
///////////////////////////////////////////////////////////////////////////////

void RGBLight::store(){

//  EEPROM.write(EE_R_VALUE,rValue);
//  EEPROM.write(EE_G_VALUE,gValue);
//  EEPROM.write(EE_B_VALUE,bValue);

}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::load(){

//  rValue=EEPROM.read(EE_R_VALUE);
//  gValue=EEPROM.read(EE_G_VALUE);
//  bValue=EEPROM.read(EE_B_VALUE);

}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::init(){
  pinMode(RED_LED,OUTPUT);
  pinMode(BLUE_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  
  load();
  activate(rValue,gValue,bValue);
}

///////////////////////////////////////////////////////////////////////////////

void RGBLight::show(){

  INTERFACE.print("<L");
  INTERFACE.print(rValue);
  INTERFACE.print(" ");
  INTERFACE.print(gValue);
  INTERFACE.print(" ");
  INTERFACE.print(bValue);  
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
/*
void RGBLight::blink(int n){
  if(n>0){
    setRGB(0,0,0);              // turn RGB off
    rgbBlinker.setEvent(20,-n);          // set timer for 500ms and call this routine again with parameter = -n
  } else
  if(n<0){
    setRGB();                   // turn RGB back on using stored values
    n++;                        // increment count (noting n is negative)
    if(n<0)                     // if n has not yet reached zero
      rgbBlinker.setEvent(20,-n);             // set timer for another 500ms and call this routing again with parameter = -n (which should be a positive number)
  }
}
*/
///////////////////////////////////////////////////////////////////////////////

int RGBLight::rValue=255;
int RGBLight::gValue=255;
int RGBLight::bValue=255;

