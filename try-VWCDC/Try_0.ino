/*
Testlauf, bin mal gespannt
*/

#define F_CPU 16000000UL
#define UART_BAUD_RATE 9600

byte start = 0;

//byte senden(byte, byte, byte, byte, byte, byte, byte, byte);
//byte transmite(byte);
//void radio_interrupt();


// timer interrupt service routine
ISR(Timer0_COMPA_vect)
{
  
   PORTD ^= (1<<PD5);
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
