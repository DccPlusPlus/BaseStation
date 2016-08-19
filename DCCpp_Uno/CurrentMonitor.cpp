/**********************************************************************

CurrentMonitor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp_Uno.h"
#include "CurrentMonitor.h"
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

CurrentMonitor::CurrentMonitor(int sensePin, int enablePin, char *msg){
    this->sensePin=sensePin;
    this->enablePin=enablePin;
    this->msg=msg;
    current=0;
} // CurrentMonitor::CurrentMonitor

boolean CurrentMonitor::checkTime(){
  if(millis()-sampleTime<CURRENT_SAMPLE_TIME)            // no need to check current yet
    return(false);
  sampleTime=millis();                                   // note millis() uses TIMER-0.  For UNO, we change the scale on Timer-0.  For MEGA we do not.  This means CURENT_SAMPLE_TIME is different for UNO then MEGA
  return(true);  
} // CurrentMonitor::checkTime

void CurrentMonitor::check(){
  // compute new exponentially-smoothed current
  current=analogRead(sensePin)*CURRENT_SAMPLE_SMOOTHING+current*(1.0-CURRENT_SAMPLE_SMOOTHING);
  // current overload and enable pin is on disable the output
  if(current > CURRENT_SAMPLE_MAX && digitalRead(enablePin)) {
    digitalWrite(enablePin, LOW);
    // print corresponding error message
    INTERFACE.print(msg);
  } else if(current < CURRENT_SAMPLE_MAX && !digitalRead(enablePin)) {
    digitalWrite(enablePin, HIGH);
  }
} // CurrentMonitor::check  

long int CurrentMonitor::sampleTime=0;

