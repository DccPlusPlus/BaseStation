/**********************************************************************

SerialCommand.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

// DCC++ BASE STATION COMMUNICATES VIA THE SERIAL PORT USING SINGLE-CHARACTER TEXT COMMANDS
// WITH OPTIONAL PARAMTERS, AND BRACKETED BY < AND > SYMBOLS.  SPACES BETWEEN PARAMETERS
// ARE REQUIRED.  SPACES ANYWHERE ELSE ARE IGNORED.  A SPACE BETWEEN THE SINGLE-CHARACTER
// COMMAND AND THE FIRST PARAMETER IS ALSO NOT REQUIRED.

// See SerialCommand::parse() below for defined text commands.

#include <ESP8266WiFi.h>
#include "SerialCommand.h"
#include "DCCpp_NodeMCU.h"
#include "Sensor.h"
#include "EEStore.h"

extern WiFiClient   client;
extern WiFiServer   server;

///////////////////////////////////////////////////////////////////////////////

char SerialCommand::commandString[MAX_COMMAND_LENGTH+1];

///////////////////////////////////////////////////////////////////////////////

void SerialCommand::process(){
  char c;
    
    client = server.available();

    if(client){
//      Serial.print("^" );
      while( client.connected() ) {
        if( client.available()){        // while there is data on the network
          c=client.read();
          if(c=='<')                    // start of new command
            sprintf(commandString,"");
          else if(c=='>') {              // end of new command
//            Serial.print(commandString);
            parse(commandString);
          }
          else if(strlen(commandString)<MAX_COMMAND_LENGTH)    // if comandString still has space, append character just read from network
            sprintf(commandString,"%s%c",commandString,c);     // otherwise, character is ignored (but continue to look for '<' or '>')
        }
        Sensor::check();
      } // while
 //     Serial.print( "x" );
    }
} // SerialCommand:process
   
///////////////////////////////////////////////////////////////////////////////

void SerialCommand::parse(char *com){
  
  switch(com[0]){
    case 'T': // process turnouts
    case 'Z': // process outputs
      client.print("<X>");     // nothing defined for now
      break;

    case 'S': 
/*   
 *   *** SEE SENSOR.CPP FOR COMPLETE INFO ON THE DIFFERENT VARIATIONS OF THE "S" COMMAND
 *   USED TO CREATE/EDIT/REMOVE/SHOW SENSOR DEFINITIONS
 */
      Sensor::parse(com+1);
      break;

/***** SHOW STATUS OF ALL SENSORS ****/

    case 'Q':         // <Q>
/*
 *    returns: the status of each sensor ID in the form <Q ID> (active) or <q ID> (not active)
 */
      Sensor::status();
      break;

    case '1':      // <1>
/*   
 *    enables power from the motor shield to the main operations and programming tracks
 *    
 *    returns: <p1>
 */    
     client.print("<p0>");
     break;
          
/***** TURN OFF POWER FROM MOTOR SHIELD TO TRACKS  ****/    

    case '0':     // <0>
/*   
 *    disables power from the motor shield to the main operations and programming tracks
 *    
 *    returns: <p0>
 */
     client.print("<p0>");
     break;

/***** READ STATUS OF DCC++ BASE STATION  ****/    

    case 's':      // <s>
/*
 *    returns status messages containing track power status, throttle status, turn-out status, and a version number
 *    NOTE: this is very useful as a first command for an client to send to this sketch in order to verify connectivity and update any GUI to reflect actual throttle and turn-out settings
 *    
 *    returns: series of status messages that can be read by an client to determine status of DCC++ Base Station and important settings
 */
      client.print("<p0>");
      client.print("<iDCC++ BASE STATION FOR ARDUINO ");
      client.print(ARDUINO_TYPE);
      client.print(": V-");
      client.print(VERSION);
      client.print(" / ");
      client.print(__DATE__);
      client.print(" ");
      client.print(__TIME__);
      client.print(">");
      client.print("<N");
      client.print("1");
      client.print(": ");
      client.print(WiFi.localIP());
      client.print(">");          
                      
      break;

/***** STORE SETTINGS IN EEPROM  ****/    

    case 'E':     // <E>
/*
 *    stores settings for turnouts and sensors EEPROM
 *    
 *    returns: <e nTurnouts nSensors>
*/
     
    EEStore::store();
    client.print("<e ");
    client.print(EEStore::eeStore->data.nTurnouts);
    client.print(" ");
    client.print(EEStore::eeStore->data.nSensors);
    client.print(" ");
    client.print(EEStore::eeStore->data.nOutputs);
    client.print(">");
    break;
    
/***** CLEAR SETTINGS IN EEPROM  ****/    

    case 'e':     // <e>
/*
 *    clears settings for Turnouts in EEPROM
 *    
 *    returns: <O>
*/
     
    EEStore::clear();
    client.print("<O>");
    break;

/***** PRINT CARRIAGE RETURN IN SERIAL MONITOR WINDOW  ****/    
                
    case ' ':     // < >                
/*
 *    simply prints a carriage return - useful when interacting with Ardiuno through serial monitor window
 *    
 *    returns: a carriage return
*/
      client.println("");
      break;  

  } // switch
}; // SerialCommand::parse

///////////////////////////////////////////////////////////////////////////////


