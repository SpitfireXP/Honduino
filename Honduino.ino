/***********************************************************************************************************************
project: <Honduino Version1>
author: <SpitfireXP  -->  http://www.iphpbb.com/board/ftopic-34345126nx14123-11107.html>
description: <I2C Display ansteuern und digitale Temperatursensoren auslesen um diese dann auf dem Display darzustellen.
              Öldruck und Öltemperatur über einen analogen Eingang auslesen und Darstellen.
              Vtec/ITB anzeigen mittels einem Digitalen ports des Rover 45 Tachos>
***********************************************************************************************************************/

/*-----( Import libraries )-----*/
  #include "LiquidCrystal_I2C.h"
  #include <tsic.h>
  #include "Wire.h"
  #include "TimerOne.h"



// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                     addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// definieren Temperatursensor

  int error;  //1 = OK, 0 = parity error    return value of getTSicTemp()
  int aussentemperatur;  // "return" of temperture Aussentemperatur
  int innentemperatur;  // "return" of temperture Innentemperatur
  int motorraumtemperatur;  // "return" of temperture Motorraumtemperatur
  int ansauglufttemperatur;  // "return" of temperture Ansauglufttemperatur

  tsic Sensoraussen(0, 5);  //firstValue = VCC-Pin, secondValue = Signal-Pin
  tsic Sensorinnen(0, 8);  //firstValue = VCC-Pin, secondValue = Signal-Pin
  tsic Sensormotorraum(0, 6);  //firstValue = VCC-Pin, secondValue = Signal-Pin
  tsic Sensoransaugluft(0, 7);  //firstValue = VCC-Pin, secondValue = Signal-Pin

  char oeldruckbuffer [50] ;
  int oeltemp;
  int boardspannung;
  char boardspannungbuffer [50];
  int helligkeit;

  int sensoroeldruck = (A0);
  int sensoroeltemp = (A1);
  int sensorbordspannung = (A2);
  int sensorhelligkeit = (A3);

// definiere VTEC-light
  int vtecState = 0;         // variable for reading the pushbutton status
  int asbState = 0;
  int vtecanzeige = 13;
  int vtec = 12;
  int asb = 2;
  int licht = 10;
  int beleuchtung = 11;
  int alle_x_sekunden=1;

// Timer für VTEC-Light
  long previousMillis = 0;        // speichert den letzten wert wo der LED-Timer aufgefrischt wurde
  long interval = 50;           // long, da millis sehr groß werden kann
  int timerState = LOW;


// Interruptpin
  int Interruptpin = 3;
  int dispSelect = 1;
  int interruptCalled = 1;



// setup
  void setup()
  {
//  pinMode(licht, INPUT);
  pinMode(beleuchtung, OUTPUT);
  
  Timer1.initialize(alle_x_sekunden*100000);
  Timer1.attachInterrupt(blinken);


 // Starte Serielle Datenübertragung
  Serial.begin(9600);

 // initialize the lcd for 20 chars 4 lines
  lcd.begin(20,4);

 
 // vtec-light
  pinMode(vtecanzeige, OUTPUT);
  pinMode(vtec, INPUT);
  pinMode(asb, INPUT);
  
// Interrupt
  attachInterrupt(1, Interruptroutine, FALLING);
 
//  Boot
  digitalWrite(vtecanzeige, HIGH);
  analogWrite(beleuchtung, 255);
  lcd.setCursor(0,0);
  lcd.print("####################");
  lcd.setCursor(0,1);
  lcd.print("#   <<Honduino>>   #");
  lcd.setCursor(0,2);
  lcd.print("#      Version 1.2 #");
  lcd.setCursor(0,3);
  lcd.print("####################");
  delay(2500);
  }

// loop
  void loop()
  {
 
 

// Dimmen der Displayhelligkeit per LDR um bei Dunkelheit das Display abzudimmen

int val = analogRead(sensorhelligkeit);
  
  val = constrain (val, 100, 700);
  int ledLevel = map(val, 700, 100, 255, 30);
  
  analogWrite(beleuchtung, ledLevel);


  if(interruptCalled == 1){
  lcd.clear();
  interruptCalled = 0;
  }

// switchcase
  switch(dispSelect)
  {
  case 1:
  seite1();
  break;

  case 2:
  seite2();
  break;
  }


// Auslesen der Temperaturwerte und Wiedergabe auf dem Display


  error = Sensoraussen.getTSicTemp(&aussentemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensorinnen.getTSicTemp(&innentemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensormotorraum.getTSicTemp(&motorraumtemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensoransaugluft.getTSicTemp(&ansauglufttemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF


// Auslesen und Anzeigen von Öldruck und Öltemperatur auf dem Display
  int oeldruck = analogRead(sensoroeldruck);
  oeldruck = map(oeldruck, 0, 1023, 0, 100);
  
  //char oeldruckbuffer [50] ;

  double doubleoeldruck = oeldruck/10.0;
  dtostrf(doubleoeldruck,2,1,oeldruckbuffer);

  oeltemp = analogRead(sensoroeltemp);
  oeltemp = map(oeltemp, 0, 1023, -20, 150);

//  drosselklappe = analogRead(sensordrosselklappe);
//  drosselklappe = map(drosselklappe, 95, 875, 0, 100);

  boardspannung = analogRead(sensorbordspannung);
  boardspannung = map(boardspannung, 0, 1023, 0, 220);
  
  double doubleboardspannung = boardspannung/10.0;
  dtostrf(doubleboardspannung,2,1,boardspannungbuffer);



// VTEC-light anzeige

  vtecState = digitalRead(vtec);
  asbState = digitalRead(asb);

  if  (asbState == HIGH & vtecState == LOW)
  {
  digitalWrite(vtecanzeige, HIGH);
  } 
  else
  {  
  digitalWrite(vtecanzeige, LOW);
  }


//Debugging
  Serial.println("----------------------------------------------------");
  Serial.println("Honduino build by SpitfireXP");
  Serial.println("Honduino@SpitfireXP.de");
  Serial.println("Honduino Version 1.2");
  Serial.println("----------------------------------------------------");
  Serial.print(aussentemperatur);
  Serial.println(" C Außentemperatur");
  Serial.print(innentemperatur);
  Serial.println(" C Innentemperatur");

  Serial.print(motorraumtemperatur);
  Serial.println(" C Kühlwassertemperatur");
  Serial.print(ansauglufttemperatur);
  Serial.println(" C Ansdauglufttemperatur");

  Serial.print(oeltemp);
  Serial.println(" C Öltemperatur");
  Serial.print(oeldruckbuffer);
  Serial.println(" Bar Öldruck");

  Serial.print(asb);
  Serial.println(" Schaltsaugbrücke");
  Serial.print(vtec);
  Serial.println(" VTEC");

  Serial.print(sensorhelligkeit);
  Serial.print("; ");
  Serial.print(val);
  Serial.print("; ");
  Serial.print(ledLevel);
  Serial.print("; ");
  Serial.println(" Helligkeit");
  Serial.print(boardspannungbuffer);
  Serial.println(" Volt");
  }

// Interrupt
  void Interruptroutine()
  {
    interruptCalled = 1;
  if(dispSelect<2)
  {
  dispSelect++;
  }
  else
  {
  dispSelect = 1;
  }

  }



// Ausgabe der zweiten seite
  void seite1()
  {
    
  // Auslesen und Anzeigen der Aussen und Innentemperatur
  lcd.setCursor(0, 2);
  lcd.print(aussentemperatur);
  lcd.print("C Aussentemp.");
  lcd.setCursor(0, 3);
  lcd.print(innentemperatur);
  lcd.print("C Innentemp.");

  // Ausgabe Öldruck
  lcd.setCursor(0, 0);
  lcd.print(oeldruckbuffer);
  lcd.print("Bar ");
  lcd.write(239);
  lcd.print("ldruck");
  
  // Ausgabe Öltemperatur
  lcd.setCursor(0, 1);
  lcd.print(oeltemp);
  lcd.print("C ");
  lcd.write(239);
  lcd.print("ltemperatur");
  }

// Ausgabe der zweiten Seite
  void seite2()
  {
// Ausgabe Boardspannung
  lcd.setCursor(0, 0);
  lcd.print(boardspannungbuffer);
  lcd.print("V Boardspannung");

// Zeile Löschen
  lcd.setCursor(0,1);
  lcd.print("               ");
  
// Auslesen und Anzeigen der Motorraum und Ansauglufttemperatur
  lcd.setCursor(0, 2);
//  lcd.print("Motorraumtemp.");
  lcd.print(motorraumtemperatur);
  lcd.print("C Wassertemp.");
  lcd.setCursor(0, 3);
  lcd.print(ansauglufttemperatur);
  lcd.print("C Ansaugbrückentemp.");
  }


/*
void disclaimer()
  {
  lcd.setCursor(0,0);
  lcd.print("Honduino@");
  lcd.setCursor(0,1);
  lcd.print("       SpitfireXP.de");
  lcd.setCursor(0,2);
  lcd.print("THX to: Hauke,");
  lcd.setCursor(0,3);
  lcd.print("SRE, Thorben, Patrick");
  }
*/
  void blinken()
  {
    if (asbState == LOW & vtecState == HIGH) 
    {
    digitalWrite(vtecanzeige, digitalRead(vtecanzeige) ^ 1);
  }
}