/*
* VAG_CDC.c
*
* Created: 30.03.2017 20:00:51
* Author: Abrell Markus
*
* BC127  //ovc3860
*
* http://www.14core.com/wiring-the-xs3868-ovc3860-stereo-bluetooth-2-0-edr-module/
*
* https://learn.sparkfun.com/tutorials/understanding-the-bc127-bluetooth-module#introduction
*
*
*/

#define F_CPU 8000000UL
#define UART_BAUD_RATE 9600

#define CLOCKPIN  D6
#define DATAPIN   D7
#define PWR_LED   D5
#define RADIO_PIN D2

#include <inttypes.h>
#include <SPI.h>

int send_package(uint8_t p[8]);

int count = 0, status = 0, s = 0, package = 0;
int data = 0;
uint8_t daten[] = {0x74,0xBE,0xFE,0xFF,0xFF,0xFF,0x8F,0x7C};

// timer interrupt service routine
ISR(Timer0_COMPA_vect)
{
   //Setze vorlauf
   TCNT0 = 89;
   
   //Prüfe ob 8 Bits gesendet
   if(count < 8)
   {
     
     //Wenn Clock low dann lege daten an
     if(!(digitalRead(CLOCKPIN)))
     {
       digitalWrite(DATAPIN, (data>>count)%2);//Schreibe Daten
     }
     else
     {
       count++;
     }
     
     digitalWrite(CLOCKPIN, digitalRead(CLOCKPIN) ^ 1); // toggle CLOCK pin
   }
   //Setze den Zähler zurück
   else
   {
     count = 0;
     status = 0;
   }
}

void setup() {
   //Beginne Serielle Kommunikation
   Serial.begin(UART_BAUD_RATE);
   
   //Sende Hallo Welt!
   Serial.println("Hallo Welt!");
   
   
   //Vorbereitungen für Bitbanging SPI
   pinMode(CLOCKPIN, OUTPUT);
   pinMode(DATAPIN, OUTPUT);
   
   //Timer einrichten
   noInterrupts(); //keine Interupts
   TCCR0A |= (1<<CS00);
   TCNT0 = 89;
   interrupts(); //erlaube Interupts
   
   /*initialisiere CDC
   send_package(0x74,0xBE,0xFE,0xFF,0xFF,0xFF,0x8F,0x7C); //idle
   _delay_ms(10);
   send_package(0x34,0xFF,0xFE,0xFE,0xFE,0xFF,0xFA,0x3C); //load disc
   _delay_ms(100);
   send_package(0x74,0xBE,0xFE,0xFF,0xFF,0xFF,0x8F,0x7C); //idle
   _delay_ms(10);*/
}

void loop() {
    //Prüfen ob daten angekommen sind zum verarbeiten
    if (Serial.available())
    {
      //Serielles Zeichen einlesen
      char ch = Serial.read();
      
      if(ch == 's' || ch == 'S')
      {
        Serial.println("Start!");
        s = 1;
      }
      else
      {
        Serial.println("Stop!");
        s = 0;
      }
    }
    
    if(s)
    {
      send_package(daten);
    }
}

int send_package(uint8_t p[8])
{
  if(package < 8)
  {
    if(status == 0)
    {
      data = p[package];
      package++;
      status = 1;
    }
  }
  else
  {
    package = 0;
  }
}
