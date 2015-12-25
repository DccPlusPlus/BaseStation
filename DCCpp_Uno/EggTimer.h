/**********************************************************************
 
EggTimer.h
COPYRIGHT (c) 2013-2015 Gregg E. Berman
 
Part of DCC++ BASE STATION for the Arduino
 
**********************************************************************/

#ifndef EggTimer_h
#define EggTimer_h

class EggTimer{
  boolean set;                    // flag to check whether or not this timer has been set
  long eTime;                     // the end time for this timer, in units of the timing increment
  void (*eFunc)(int);             // the function to be called when timer expires
  int iParam;                     // the integer parameter to use in the funtion call for this timer
  
  static long cTime;              // cumulative number of timing increments since timerConfig was called
  
  public:
    EggTimer(void (*eFunc)(int));      // creates timer that will call function eFunc, which must take a single integer parameter
    static void timerConfig();          // configures interrupt -- must be called before timers are used
    static void update();               // used by interrupt to increment cTime-- should not be called directly by any user code
  
    void setEvent(int csec,int iParam=0);   // sets a timing event such that when check() is called for this instance, eFunc(iParam) will be called after csec timing increments have passed 
    void check();                           // checks to see if this instance of autoTimer has been set, and if so whether the timer has expired
};

#endif

