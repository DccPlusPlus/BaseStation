

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

  bitClear(TCCR5B,WGM33);   // set Timer 5 to CTC, TOP=OCR5A
  bitSet(TCCR5B,WGM32);
  bitClear(TCCR5A,WGM31);
  bitClear(TCCR5A,WGM30);

  bitSet(TCCR5B,CS32);    // set Timer 5 prescale=1024
  bitClear(TCCR5B,CS31);
  bitSet(TCCR5B,CS30);

  OCR5A=156;              // trigger every 10 milliseconds
  TCNT5=0;                // reset counter
  bitSet(TIFR5,OCF5A);    // clear any current interrupt flag (note this is done by writing a logical one)
  bitSet(TIMSK5,OCIE5A);  // enable interrupt vector for Timer 5 Output Compare A Match (OCR5A)
}

///////////////////////////////////////////////////////////////////////////////

ISR(TIMER5_COMPA_vect){   // set interrupt service for OCR5A of TIMER-5
  AutoTimer::update();
}

///////////////////////////////////////////////////////////////////////////////

long AutoTimer::cTime;


