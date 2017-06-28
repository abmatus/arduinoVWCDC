/*
Testlauf, bin mal gespannt
*/

#define F_CPU 16000000UL
#define UART_BAUD_RATE 9600

byte start = 0;
byte runde = 0;

// timer interrupt service routine
ISR(Timer0_COMPA_vect)
{
  if(!start)
   {
     PORTD ^= (1<<PD5);
     start = 1;
   }
   if else(start == 1)
   {
     PORTD ^= (1<<PD7);
     start = 0;
   }
   else
   {
     PORTD &= B01011111; //Setze 0
   }
}



void setup() {
  //Pins einrichten
  //Vorbereitungen für Bitbanging SPI
  pinMode(PD5, OUTPUT);
  pinMode(PD7, OUTPUT);
  
  
  //Serielle Komunikation
  //Beginne Serielle Kommunikation
  Serial.begin(UART_BAUD_RATE);
  
  //Sende Hallo Welt!
  Serial.println("Hallo Welt!");
  
  
  //Timer einrichten
  noInterrupts(); //keine Interupts
  TCCR0A |= (1<<CS00);
  TCNT0 = 89; //Timer vorbelegen
  interrupts(); //erlaube Interupts
}

void loop() {
}
