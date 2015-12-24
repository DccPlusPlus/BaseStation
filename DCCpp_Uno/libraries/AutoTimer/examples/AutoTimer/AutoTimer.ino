
#include "Arduino.h"
#include "AutoTimer.h"

struct TestFunc{
  static void f1(int s);
  static void f2(int s);
  static void f3(int s);
};

AutoTimer autoTimer[]={AutoTimer(TestFunc::f1),AutoTimer(TestFunc::f2)};

void setup() {
  
  Serial.begin(115200);
    
  Serial.print("f0 ");
  Serial.println(millis());
  
  AutoTimer::timerConfig();
  
  autoTimer[0].setEvent(500);
    
}

void loop() {
  
  autoTimer[0].check();
  
}

void TestFunc::f1(int s){
  autoTimer[0].setEvent(100,millis()/1000);
  Serial.print("f1 ");
  Serial.print(s);
  Serial.print(" ");
  Serial.println(millis());
}

void TestFunc::f2(int s){
  Serial.print("f2 ");
  autoTimer[0].setEvent(500);
  Serial.println(millis());
  autoTimer[1].setEvent(100);

}

void TestFunc::f3(int s){
  Serial.print("f3 ");
  Serial.println(millis());
  autoTimer[1].setEvent(100);
}


