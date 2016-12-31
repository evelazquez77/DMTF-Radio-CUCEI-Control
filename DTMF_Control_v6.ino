////////////////////////////////////////////////////////////////////
//  Radio Receiver Controlled by DMTF Commands
//  This program controls the operation of a radio receiver by DMTF commands sent through the same receive radio channel.
//  The system is performed by an Arduino Nano that controls a set of ICs as an MT8870 DTMF Decoder, an FM radio receiver module
//  based on the Si4703 and a MC4066 as a base of digital volume control.
//  The main functions are:
//  1.- The selective and general power on and off of the audio power system
//        Command: #MMSSP#
//            MM=[ Corresponding classroom module number (00~99 where 00 is for General School) ]
//            SS=[ Corresponding classroom number (00~99 where 00 is for General Module) ]
//            P=[ A: General Power ON, B: General Power OFF, C: Selective Power ON, D: Selective Power OFF ]
//  2.- 4-level audio volume control
//        Command: #MMSSV#
//            MMSS= Same as above.
//            V=[ 0: Mute, 1~2:Intermediate Levels of Sound Volume 3: Full Sound Volume ]
//  3.- General power on in emergency mode with and w/o alarm sound.
//            Command: #**#
//
///////////////////////////////////////////////////////////////////

#include "Si4703_Breakout.h"
#include <Wire.h>

//Delayed Steering (Output).Presents a logic high when a received tone-pair has been
//registered and the output latch updated
const int   StD = 0;          //int 0 on digital pin 2
const char  key_ctrl[16] = "D1234567890*#ABC";
// inputs to arduino from dtmf -MT8870- Q1,Q2,Q2,Q4 and a mirror of StD
const int   pins[5] = { 8, 9, 10, 11, 2 };
const char  Id_Gen[] = "00";  // Constant characters for general activation
const char  Id_MM[] = "21";   // Letter T, as the "T" module, has the 21th place in Alphabet
const char  Id_SS[] = "08";   // 08 for the classroom "T8"

/////////////////// Variables for command handler
char        key_rcvd;
char        two_key[] = "@@"; // Accumulative two char
int         key;              //key represents the pressed key
boolean     Id_AmI;
boolean     YesNo;

/////////////////// Variables for Radio Module
int resetPin = 12;
int SDIO = A4;
int SCLK = A5;
Si4703_Breakout radio(resetPin, SDIO, SCLK);
float channel;
int volume;

/////////////////
void setup()
{
  Serial.begin(9600);
  attachInterrupt(StD, dtmf_decode, RISING); //Detect StD when goes from 0 to 1
  for (int i = 0; i < 5; i++)  // Initializing 5 inputs Q1,Q2,Q2,Q4 and a mirror of StD
    pinMode(pins[i], INPUT);

  radio.powerOn();            // Initializing radio module to power on
  radio.setVolume(10);        // Selecting to volumen level
  radio.setChannel(987);      // Setting radio module receiving channel in Radio CUCEI

  Serial.println("Send me a command letter.");
}
///////////////////
void loop()
{
  // Radio Control by DMTF
  if (digitalRead(2) == HIGH )
  {
    Id_AmI = false;
    // Serial.print(key_rcvd);
    Wait_NextCharacter ();    // Wait a high level in StD
    //Serial.println(" was received");

    //////////////////////////// DETECT # ////////////////////////////
    if (key == 12 )           // Is # received?
    {
      Serial.println("Header # Received");
      Wait_Low ();            // Wait a low level in StD

      //////////////////////////// DETECT MM OR ** ////////////////////////////
      wait_ACharacter (0);                      // Capturing next two characters
      wait_ACharacter (1);

      if ( !strcmp(two_key, "**" ))             // Defines whether the code is an alarm signal
      {
        if ( Is_FinalHash )                     // Defines whether the character is the end of the command string.
        {
          Serial.println("ES Alarma general. Debe de sonar");                                         // OK OK OK
          // Here goes an action to turn on alarm
          // End of hash verification
          // Closing detection cicle
        }
      }
      else
      {
        Serial.println("NO es Alarma general");

        if ( !strcmp(two_key, Id_Gen ))         // Defines whether the code is for General CUCEI call
        {
          Serial.println("ES difusion general para todo CUCEI");                                      // OK OK OK
          Compare_Ids(Id_SS);         // Classroom code is Captured only to finish protocol
          Is_FinalHash();             // And the final hash to close detection cicle
          Serial.println("Cerrando ciclo de detección");
          // Here goes an action to turn on alarm
          // End of hash verification
          // Closing detection cicle
        }
        else
        {
          if ( !strcmp(two_key, Id_MM) )        // Defines whether the code is for the same building
          {
            Serial.println("El codigo de modulo es el que corresponde");                                      // OK OK OK

            //////////////////////////// DETECT SS ////////////////////////////
            wait_ACharacter (0);                // Capturing next two characters
            wait_ACharacter (1);                // to check Classroom code

            if ( !strcmp(two_key, Id_Gen ))     // Defines whether the code is for general building call
            {
              Serial.println("ES difusion general para todo el modulo");                                      // OK OK OK
              Is_FinalHash();             // And the final hash to close detection cicle
              // Here goes an action to General Power ON / OFF
              Serial.println("Cerrando ciclo de detección");
              // Here goes an action to turn on alarm
              // End of hash verification
              // Closing detection cicle

            }
            else
            {

              if ( !strcmp(two_key, Id_SS) )    // Defines whether the code is for the same building
              {
                Serial.println("El codigo de salon es el que corresponde");                                      // OK OK OK
                Is_FinalHash();             // And the final hash to close detection cicle
                // Here goes an action to Selective Power ON / OFF
                Serial.println("Cerrando ciclo de detección");
                // Here goes an action to turn on alarm
                // End of hash verification
                // Closing detection cicle
              }
              else
              {
                Serial.println("No es el salón");                                                               // OK OK OK
                Is_FinalHash();             // And the final hash to close detection cicle
              }
            }
          }
          else
          {
            Serial.println("No es el modulo");                                                               // OK OK OK
            Compare_Ids(Id_SS);         // Classroom code is Captured only to finish protocol
            Is_FinalHash();             // And the final hash to close detection cicle
            Serial.println("Cerrando ciclo de detección sin encontrar nada");
            // There no action
            // End of hash verification
            // Closing detection cicle
          }


        }


      }
      Wait_Low ();
    }

  }
}
///////////////////
void dtmf_decode()
{
  key = 0;
  key_rcvd = "";
  for (int i = 0; i < 4; i++) // decoding value of Q1,Q2,Q2,Q4 as key
    key += digitalRead(pins[i]) << i;
  key_rcvd = key_ctrl[key];
}
//////////////////
void displayInfo()
{
  float canal = channel / 10;
  Serial.print("Channel:"); Serial.print(canal);
  Serial.print(" Volume:"); Serial.print(volume);
  Serial.print(" Strength:"); Serial.println(radio.getStrength());
}
//////////////////
void Wait_Low()
{ while ( digitalRead(2) == HIGH )
  {  } // Do Nothing!
}
/////////////////
void Wait_NextCharacter()
{
  while ( digitalRead(2) != HIGH )
  {   } // Do Nothing!
}
/////////////////
void wait_ACharacter(int Pos)
{
  Wait_NextCharacter();
  Wait_Low();
  two_key[Pos] = key_rcvd;
  Serial.println(two_key[Pos]);
  //Serial.println(" was received");
}
/////////////////
boolean Compare_Ids(char ID2[3])
{
  wait_ACharacter (0);
  wait_ACharacter (1);
  if ( !strcmp(two_key, ID2))
  {
    return true;
  }
  else
  {
    return false;
  }
}
/////////////////
boolean Is_FinalHash()
{
  Wait_NextCharacter();
  Wait_Low();
  if ( key == 12 )            // Is # received?
  {
    return true;
  }
  else
  {
    return false;
  }
}
/////////////////
void Alarm_tone1()
{
  Serial.print(" Running Alarm Tone 1");
}
/////////////////

