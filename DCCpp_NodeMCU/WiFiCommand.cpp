/**********************************************************************

WiFiCommand.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

// DCC++ BASE STATION COMMUNICATES VIA WIFI USING SINGLE-CHARACTER TEXT COMMANDS
// WITH OPTIONAL PARAMTERS, AND BRACKETED BY < AND > SYMBOLS.  SPACES BETWEEN PARAMETERS
// ARE REQUIRED.  SPACES ANYWHERE ELSE ARE IGNORED.  A SPACE BETWEEN THE SINGLE-CHARACTER
// COMMAND AND THE FIRST PARAMETER IS ALSO NOT REQUIRED.

// See WiFiCommand::parse() below for defined text commands.

#include <ESP8266WiFi.h>
#include "Config.h"
#include "DCCpp_NodeMCU.h"
#include "WiFiCommand.h"
#include "Sensor.h"
#include "EEStore.h"

WiFiServer   server(ETHERNET_PORT);
WiFiClient   client[MAX_CLIENTS];

///////////////////////////////////////////////////////////////////////////////

char WiFiCommand::commandString[MAX_CLIENTS][MAX_COMMAND_LENGTH+1];

///////////////////////////////////////////////////////////////////////////////

void WiFiCommand::init(){
  Serial.print("\n\nConnecting to ");
  Serial.println(_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin( _SSID, _PASSWORD );
  WiFi.hostname( _HOSTNAME );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.disconnect();
  
  Serial.println( WiFi.softAP( _HOSTNAME, _PASSWORD, 6) ? "AP Started": "AP failed" );
  Serial.print("\n\nConnecting to ");
  Serial.println( _SSID );
  WiFi.mode(WIFI_AP);
  WiFi.begin( _SSID, _PASSWORD );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }
  Serial.println("\nWiFi connected");
  
  server.begin();
}


void WiFiCommand::process(){
  char c;

  if (server.hasClient()) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!client[i] || !client[i].connected()) {
        if (client[i]) {
          client[i].stop();
        }
        client[i] = server.available();
        continue;
      }
    }
    server.available().stop();
  }
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (client[i] && client[i].connected()) {
      if( client[i].connected() ) {
        if( client[i].available()){        // while there is data on the network
          c=client[i].read();
          if(c=='<')                    // start of new command
            sprintf(commandString[i],"");
          else if(c=='>') {              // end of new command
//            Serial.print(commandString);
            parse(commandString[i]);
          }
          else if(strlen(commandString[i])<MAX_COMMAND_LENGTH)    // if comandString still has space, append character just read from network
            sprintf(commandString[i],"%s%c",commandString[i],c);     // otherwise, character is ignored (but continue to look for '<' or '>')
        }
//        Sensor::check();
      } // while
    }
} // WiFiCommand:process
   
///////////////////////////////////////////////////////////////////////////////

void WiFiCommand::parse(char *com){
  
  switch(com[0]){
    case 'T': // process turnouts
    case 'Z': // process outputs
      WiFiCommand::print("<X>");     // nothing defined for now
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
     WiFiCommand::print("<p0>");
     break;
          
/***** TURN OFF POWER FROM MOTOR SHIELD TO TRACKS  ****/    

    case '0':     // <0>
/*   
 *    disables power from the motor shield to the main operations and programming tracks
 *    
 *    returns: <p0>
 */
     WiFiCommand::print("<p0>");
     break;

/***** READ STATUS OF DCC++ BASE STATION  ****/    

    case 's':      // <s>
/*
 *    returns status messages containing track power status, throttle status, turn-out status, and a version number
 *    NOTE: this is very useful as a first command for an client to send to this sketch in order to verify connectivity and update any GUI to reflect actual throttle and turn-out settings
 *    
 *    returns: series of status messages that can be read by an client to determine status of DCC++ Base Station and important settings
 */
      WiFiCommand::print("<p0>");
      WiFiCommand::print("<iDCC++ BASE STATION FOR ARDUINO ");
      WiFiCommand::print(ARDUINO_TYPE);
      WiFiCommand::print(": V-");
      WiFiCommand::print(VERSION);
      WiFiCommand::print(" / ");
      WiFiCommand::print(__DATE__);
      WiFiCommand::print(" ");
      WiFiCommand::print(__TIME__);
      WiFiCommand::print(">");
      WiFiCommand::print("<N");
      WiFiCommand::print("1");
      WiFiCommand::print(": ");
      WiFiCommand::print(WiFi.localIP().toString().c_str());
      WiFiCommand::print(">");          
                      
      break;

/***** STORE SETTINGS IN EEPROM  ****/    

    case 'E':     // <E>
/*
 *    stores settings for turnouts and sensors EEPROM
 *    
 *    returns: <e nTurnouts nSensors>
*/
     
    EEStore::store();
    WiFiCommand::print("<e ");
    WiFiCommand::print(EEStore::eeStore->data.nTurnouts);
    WiFiCommand::print(" ");
    WiFiCommand::print(EEStore::eeStore->data.nSensors);
    WiFiCommand::print(" ");
    WiFiCommand::print(EEStore::eeStore->data.nOutputs);
    WiFiCommand::print(">");
    break;
    
/***** CLEAR SETTINGS IN EEPROM  ****/    

    case 'e':     // <e>
/*
 *    clears settings for Turnouts in EEPROM
 *    
 *    returns: <O>
*/
     
    EEStore::clear();
    WiFiCommand::print("<O>");
    break;

/***** PRINT CARRIAGE RETURN IN SERIAL MONITOR WINDOW  ****/    
                
    case ' ':     // < >                
/*
 *    simply prints a carriage return - useful when interacting with Ardiuno through serial monitor window
 *    
 *    returns: a carriage return
*/
      WiFiCommand::print("\r");
      break;  

  } // switch
}; // WiFiCommand::parse

///////////////////////////////////////////////////////////////////////////////

void WiFiCommand::print( char * string ) {
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (client[i] && client[i].connected()) {
      client[i].print(string);
      delay(1);
    }
}

  
void WiFiCommand::print(const char * string ) {
  char buffer[30];
  strcpy( buffer, string );
  WiFiCommand::print( buffer );
}

    
void WiFiCommand::print(int  num) {
  char buffer[30];
  itoa( num, buffer, 10 );  
  WiFiCommand::print( buffer );
}

