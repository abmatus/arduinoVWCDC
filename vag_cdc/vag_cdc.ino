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
*/
#define U_INT unsigned int
#define U_LONG unsigned long


#define SCAN      0x00
#define SHFFL     0x55
#define PLAY      0xFF


#define F_CPU 8000000UL
#define UART_BAUD_RATE 9600

#define CLOCKPIN  PD6
#define DATAPIN   PD7
#define PWR_LED   PD5
#define RADIO_PIN PD2

#define RADIOHISTART  9000
#define RADIOLOSTART  4500
#define RADIOLO_1     1500
#define RADIOLO_0     1


byte send_package(byte,byte,byte,byte,byte,byte,byte,byte);
byte get_radiocmd(U_LONG);
void radio_interrupt();


//Variablen deklarieren
//Standart Packet daten
byte cd = 0xBE; //CD Auswahl
byte tr = 0xFF; //Track Auswahl
byte mode = 0xFF; //Mode Auswahl

//Radio -> CDC Kommunikation
byte radio = 0; //Neuer Radiobefehl erhalten
U_LONG cmd = 0; //Radiobefehl daten
int captimehi = 0;  //High Zeiten erfassen
int captimelo = 0;  //Low Zeiten erfassen
byte cmdbit = 0;  //Empfangene Bits zählen
byte capstart = 0;  //Empfang Starten
byte capbyte = 0; //Empfange Bits

//CDC -> Radio Kommunikation
byte data = 0;  //Aktuelles Byte zum Radio
byte p = 0; //Bytenummer
byte status = 0;  //Byte vollständig versendet
byte new_package = 1;  //Neues Paket versenden

/*
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
     status = 1;
     data = 0;
   }
}
*/

void setup() {  
  //Pins einrichten
  //Vorbereitungen für Bitbanging SPI
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  //PWR_LED und RADIO_PIN
  pinMode(PWR_LED, OUTPUT);
  pinMode(RADIO_PIN, INPUT);
  
  //Serielle Komunikation
  //Beginne Serielle Kommunikation
  Serial.begin(UART_BAUD_RATE);
  
  //Sende Hallo Welt!
  Serial.println("Hallo Welt!");
  
  //Pin Interrupt einrichten
  attachInterrupt(0, radio_interrupt, CHANGE);
  
  //Warten auf das Start signal vom Radio
  while(!(radio));
  
  /*
  //Timer einrichten
  noInterrupts(); //keine Interupts
  TCCR0A |= (1<<CS00);
  TCNT0 = 89; //Timer vorbelegen
  interrupts(); //erlaube Interupts
  */
  
  //PWR_LED an und ersten Daten senden
  digitalWrite(PWR_LED, 1); //LED einschalten
  
  while(send_package(0x74,0xBE,0xFE,0xFF,0xFF,0xFF,0x8F,0x7C));  //idle
  
  while(send_package(0x34,0xFF,0xFE,0xFE,0xFE,0xFF,0xFA,0x3C));  //load disc
  
  while(send_package(0x74,0xBE,0xFE,0xFF,0xFF,0xFF,0x8F,0x7C));  //idle
  
}

void loop() {
  int r = 0;
  
  //Neues Zeichen verfügbar
  if(Serial.available()) {
    r = Serial.read();
    if(r <= 0xFF) { //Gültiges Zeichen?
      if((r & 0xC0) == 0xC0) {  //Mode ,CD Wechsel oder Track Wechsel
        if(r == 0xCA) //Mode Scan
          mode = SCAN;
        else if(r == 0xCB)  //Mode Schuffle
          mode = SHFFL;
        else if(r = 0xCC) //Mode Play
          mode = PLAY;
        else
          cd = 0xFF^(r & 0x0F); //CD Wechsel
      }
      else {  //Track Wechsel
        tr = 0xFF^r;
      }
    }
  }
  
  send_package(0x34,cd,tr,0xFF,0xFF,mode,0xCF,0x3C);  //Zusammengebautes Paket senden
  
  //Neuer Befehl vom Radio
  if(radio) {
    radio = 0;
    Serial.println(get_radiocmd(cmd));
  }
}

byte send_package(byte c0,byte c1,byte c2,byte c3,byte c4,byte c5,byte c6,byte c7) {
  byte package[] = { c0, c1, c2, c3, c4, c5, c6, c7};
  
  if(p < 8)
  {
    if(status && new_package)
    {
      data = package[p];
      p++;
      status = 0;
    }
  }
  else
  {
    p = 0;
    new_package = 0;
    
    return 0;
  }
  
  return 1;
}

byte get_radiocmd(U_LONG radio_cmd) {
  byte radio_befehl = 0;  //Befehl vom Radio zum CDC
  radio_befehl = (radio_cmd>>8) & 0xFF;
  
  return radio_befehl;
}

void radio_interrupt() {
  //Prüfen ob pin High ist, wenn ja, dann Lowzeit messen
  if(digitalRead(RADIO_PIN)) {
    
    if(capstart || capbyte) {
      captimelo = TCNT1;
    }
    
    else {
      capstart = 1;
    }
    
    TCNT1 = 0;
    
    //Zeiten
    if(captimehi > RADIOHISTART && captimelo > RADIOLOSTART) {
      capbyte = 1;
      capstart = 0;
      Serial.println("Start gefunden!");
    }
    
    else if(capbyte && captimelo > RADIOLO_1) {
      
      cmd = (cmd<<1) | 0x00000001;
      cmdbit++;
      
      Serial.println("bit1");
    }
    
    else if(capbyte && captimelo > RADIOLO_0) {
      
      cmd = (cmd<<1);
      cmdbit++;
      
      Serial.println("bit0");
    }
    
    else {
      
      Serial.println("Nüscht");
    }
    
    if(cmdbit == 32) {
      Serial.println("Neuer Radiobefehl:");
      radio = 1;
      cmdbit = 0;
      capbyte = 0;
    }
  }
  //Highzeit messen
  else {
    captimehi = TCNT1;
    TCNT1 = 0;
  }
}
