/**********************************************************************

DCC++ BASE STATION
COPYRIGHT (c) 2013-2016 Gregg E. Berman

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses

**********************************************************************/
/**********************************************************************
      
DCC++ BASE STATION is a C++ program written for the Arduino Uno and Arduino Mega
using the Arduino IDE 1.6.6.

It allows a standard Arduino Uno or Mega with an Arduino Motor Shield (as well as others)
to be used as a fully-functioning digital command and control (DCC) base station
for controlling model train layouts that conform to current National Model
Railroad Association (NMRA) DCC standards.

This stripped-down version of DCC++ BASE STATION only supports sensor inputs connected
to the NodeMCU.

DCC++ BASE STATION is controlled with simple text commands received via
the NodeMCU WiFi interface.

Neither DCC++ BASE STATION nor DCC++ CONTROLLER use any known proprietary or
commercial hardware, software, interfaces, specifications, or methods related
to the control of model trains using NMRA DCC standards.  Both programs are wholly
original, developed by the author, and are not derived from any known commercial,
free, or open-source model railroad control packages by any other parties.

However, DCC++ BASE STATION and DCC++ CONTROLLER do heavily rely on the IDEs and
embedded libraries associated with Arduino and Processing.  Tremendous thanks to those
responsible for these terrific open-source initiatives that enable programs like
DCC++ to be developed and distributed in the same fashion.

REFERENCES:

  NMRA DCC Standards:          http://www.nmra.org/index-nmra-standards-and-recommended-practices
  Arduino:                     http://www.arduino.cc/
  Processing:                  http://processing.org/
  GNU General Public License:  http://opensource.org/licenses/GPL-3.0

BRIEF NOTES ON THE THEORY AND OPERATION OF DCC++ BASE STATION:


DCC++ BASE STATION in split into multiple modules, each with its own header file:

  DCCpp_NodeMCU:    declares required global objects and contains initial Arduino setup()
                    and Arduino loop() functions, as well as and optional array of Sensors 

  WiFiCommand:    contains methods to read and interpret text commands from the WiFi interface,
                    process those instructions.

  Sensor:           contains methods to monitor and report on the status of optionally-defined infrared
                    sensors embedded in the Main Track and connected to various pins on the NodeMCU

  EEStore:          contains methods to store, update, and the sensor settings in the EEPROM for
                    recall after power-up

DCC++ BASE STATION is configured through the Config.h file that contains all user-definable parameters                    

**********************************************************************/

// BEGIN BY INCLUDING THE HEADER FILES FOR EACH MODULE

#include <ESP8266WiFi.h>
#include "Config.h"
#include "DCCpp_NodeMCU.h"
#include "Sensor.h"
#include "EEstore.h"
#include "WiFiCommand.h"

///////////////////////////////////////////////////////////////////////////////
// MAIN ARDUINO LOOP
///////////////////////////////////////////////////////////////////////////////

void loop(){
  
  WiFiCommand::process();   // check for, and process, and new WiFi commands
  Sensor::check();          // check sensors for activate/de-activate
  
} // loop

///////////////////////////////////////////////////////////////////////////////
// INITIAL SETUP
///////////////////////////////////////////////////////////////////////////////

void setup(){  

  Serial.begin(115200);            // configure serial interface
  Serial.flush();

  EEStore::init();                                          // initialize and load Turnout and Sensor definitions stored in EEPROM
  WiFiCommand::init();
  
  Serial.print("<iDCC++ BASE STATION FOR ESP8266 ");      // Print Status to Serial Line regardless of COMM_TYPE setting so use can open Serial Monitor and check configurtion 
  Serial.print(ARDUINO_TYPE);
  Serial.print(" / ");
  Serial.print(VERSION);
  Serial.print(" / ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.print(">");
  Serial.print("<N");
  Serial.print("1");
  Serial.print(": ");
             
  Serial.print(WiFi.localIP());
  Serial.println(">");
} // setup
