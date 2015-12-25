/**********************************************************************
 
EggTimer.cpp
COPYRIGHT (c) 2013-2015 Gregg E. Berman
 
Part of DCC++ BASE STATION for the Arduino
 
**********************************************************************/
/**********************************************************************
 
** OPTIONAL COMPONENT **
 
The EggTimer Class utilizes Timer-2 to implement an automatic countdown
timer that calls a specified function when the countdown reaches zero.
Multiple instances of the countdown timer can be created at the same time,
providing for parallel countdown clocks, each with each own countdown value,
and starting independently.  This class does not interfere with the PWM functionality
of the digital pins tied to Timer-2.

Usage:

EggTimer::EggTimer(FUNCTION)

  - returns an instance of an EggTimer structure where FUNCTION is the name of a
    function that will be called whenever this instance of the EggTimer is set
    and the countdown value reaches zero.  FUNCTION must take one integer parameter
    and not return any values, i.e. void FUNCTION(int) {...}

Eggtimer::timerConfig()

  - static function that should be called one time in setup() to initialize the Egg Timer
    settings.  Can be called before or after any EggTimer instances are declared, but
    must be called before any EggTimer events are set or any EggTimer countdowns are checked

EggTimer::setEvent(COUNTDOWN_TIME, INTEGER_PARAMETER)

  - method that sets and starts the countdown clock for a specific instance of EggTimer.
    COUNTDOWN_TIME is an integer representing the number of units of 2.040 milliseconds
    in the countdown. For example, COUNTDOWN_TIME=490 => 999.6 milliseconds.
    When the countdown reaches zero, the FUNCTION specified when this specific instance
    of EggTimer was created will be called and passed the INTEGER_PARAMETER specified.
    ** EggTimers do not reset automatically --- a new setEvent() must be called every time.

EggTimer::check()

  - method that should be repeatedly called in the main loop() for each instance of
    an EggTimer created.  The logic for calling FUNCTION when an EggTimer reaches zero is
    contained in routine. 
    
  
**********************************************************************/

#include "Arduino.h"
#include "EggTimer.h"

///////////////////////////////////////////////////////////////////////////////

EggTimer::EggTimer(void (*eFunc)(int)){
  set=false;
  this->eFunc=eFunc;
}

///////////////////////////////////////////////////////////////////////////////

void EggTimer::setEvent(int csec, int iParam){
 eTime=cTime+csec;
 this->iParam=iParam;
 set=true;
}

///////////////////////////////////////////////////////////////////////////////

void EggTimer::check(){
  if(set && cTime>eTime){
    set=false;
    eFunc(iParam);
  }
}

///////////////////////////////////////////////////////////////////////////////

void EggTimer::update(){
  cTime++;
}

///////////////////////////////////////////////////////////////////////////////

void EggTimer::timerConfig(){

  // Note we are not changing the default setting for Timer-2 which are:
  // Prescale=64, PWM=Phase Correct (TOP=0xFF)
  // This up-and-down counting of Timer-2 yields an interrupt frequency of
  // 16MHZ / (64*510), or a period of 2.040 milliseconds

  cTime=0;
  bitSet(TIMSK2,TOIE2);  // enable interrupt vector for Timer 2 Overflow
}

///////////////////////////////////////////////////////////////////////////////

ISR(TIMER2_OVF_vect){   // set interrupt service for Overflow of TIMER-2
  EggTimer::update();
}

///////////////////////////////////////////////////////////////////////////////

long EggTimer::cTime;

