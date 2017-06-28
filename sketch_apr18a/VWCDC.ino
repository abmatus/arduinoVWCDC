#define F_CPU 16000000UL

byte senden(byte, byte, byte, byte, byte, byte, byte, byte);
byte transmite(byte);

void setup() {
  //Pins einrichten
  //Vorbereitungen für SPI
  pinMode(PD5, OUTPUT);
  pinMode(PD7, OUTPUT);
}

void loop() {
    senden(0x34, 0xBE, 0xFE, 0xFF, 0xFE, 0xFF, 0xCF, 0x3C);//PLAY
    delay(40);
}

byte senden(byte c0, byte c1, byte c2, byte c3, byte c4, byte c5, byte c6, byte c7){
  byte paket[8] = {c0, c1, c2, c3, c4, c5, c6, c7};
  byte bytes = 0;
  for(bytes = 0; bytes < 8; bytes++){
    transmite(paket[bytes]);
    delay(1);
  }
  
  PORTD &= B1011111; //Setze Daten Low
}

byte transmite(byte data){
  for(byte i = 0; i < 8; i++){
    PORTD |= B00100000;//Setze Clock High
    delayMicroseconds(6);
    PORTD = (PORTD & B01111111) | ((data & B10000000) << i);//Daten anlegen
    delayMicroseconds(2);
    PORTD &= B11011111; //Setze Clock Low
    delayMicroseconds(9);
  }
}
