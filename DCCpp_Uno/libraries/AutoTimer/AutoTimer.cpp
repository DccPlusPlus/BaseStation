/**********************************************************************
 
 AutoTimer.cpp
 COPYRIGHT (c) 2013-2015 Gregg E. Berman
 
 Part of DCC++ BASE STATION for the Arduino
 
 **********************************************************************/
/**********************************************************************
 
 DCC++ BASE STATION OPTIONAL COMPONENT
 
 The AutoTimer Library utilizes OCR4A of Timer-4 of the Arduino Mega
 to implement an automatic countdown timer that calls a specified function
 or method when the countdown reaches zero.  Multiple instances of the
 countdown timer can be created at the same time, providing for parallel
 countdown locks, each with each own countdown value, and starting
 independently.
 
 ** This library CANNOT be used with Arduino UNO and will fail to compile
    (the UNO does not have a Timer-4)
 
 ** This library does not interfere with the use of digital pin 6, 7, or 8,
    each of which are tied to Timer-4.  These pins can be used for normal
    digital reads and write.   However, these pins should not be used for PWM
    output since AutoTimer interferes with the PWM timing of those pins
 
**********************************************************************/

#include "Arduino.h"
#include "AutoTimer.h"

///////////////////////////////////////////////////////////////////////////////

AutoTimer::AutoTimer(void (*eFunc)(int)){
  set=false;
  this->eFunc=eFunc;
}

///////////////////////////////////////////////////////////////////////////////

void AutoTimer::setEvent(int csec, int iParam){
 eTime=cTime+csec;
 this->iParam=iParam;
 set=true;
}

///////////////////////////////////////////////////////////////////////////////

void AutoTimer::check(){
  if(set && cTime>eTime){
    set=false;
    eFunc(iParam);
  }
}

///////////////////////////////////////////////////////////////////////////////

void AutoTimer::update(){
  cTime++;
}

///////////////////////////////////////////////////////////////////////////////

void AutoTimer::timerConfig(){

  cTime=0;

  bitClear(TCCR4B,WGM43);   // set Timer 4 to CTC, TOP=OCR4A
  bitSet(TCCR4B,WGM42);
  bitClear(TCCR4A,WGM41);
  bitClear(TCCR4A,WGM40);

  bitSet(TCCR4B,CS42);    // set Timer 4 prescale=1024
  bitClear(TCCR4B,CS41);
  bitSet(TCCR4B,CS40);

  OCR4A=156;              // trigger every 10 milliseconds
  TCNT4=0;                // reset counter
  bitSet(TIFR4,OCF4A);    // clear any current interrupt flag (note this is done by writing a logical one)
  bitSet(TIMSK4,OCIE4A);  // enable interrupt vector for Timer 4 Output Compare A Match (OCR4A)
}

///////////////////////////////////////////////////////////////////////////////

ISR(TIMER4_COMPA_vect){   // set interrupt service for OCR4A of TIMER-4
  AutoTimer::update();
}

///////////////////////////////////////////////////////////////////////////////

long AutoTimer::cTime;


