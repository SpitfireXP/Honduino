/***********************************************************************************************************************
project: <Honduino Version1>
author: <SpitfireXP  -->  http://www.iphpbb.com/board/ftopic-34345126nx14123-11107.html>
description: <I2C Display ansteuern und digitale Temperatursensoren auslesen um diese dann auf dem Display darzustellen.
Öldruck und Öltemperatur über einen analogen Eingang auslesen und Darstellen.
Vtec/ITB anzeigen am Display
***********************************************************************************************************************/

/*-----( Import libraries )-----*/
#include "LiquidCrystal_I2C.h"
#include <tsic.h>
#include "Wire.h"
#include "TimerOne.h"

/*
 Get the LCD I2C Library here:
 https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
 Move any other LCD libraries to another folder or delete them
 See Library "Docs" folder for possible commands etc.

 set the LCD address to 0x27 for a 20 chars 4 line display
 Set the pins on the I2C chip used for LCD connections:
                     addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
*/
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

int needrefresh = 0;
int lastpage = 0;


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


int vtecState = 0;
//int asbState = 0;
//int vtecanzeige = 13;
int vtec = 12;
//int asb = 2;
int licht = 10;
int beleuchtung = 11;

int schalter = 3;
int schalterstatus = 0;

void setup()
{
  pinMode(beleuchtung, OUTPUT);


  // Starte Serielle Datenübertragung
  Serial.begin(9600);

  // initialize the lcd for 20 chars 4 lines
  lcd.begin(20, 4);


  // vtec-light
  //	pinMode(vtecanzeige, OUTPUT);
  pinMode(vtec, INPUT);
  pinMode(schalter, INPUT);
  //	pinMode(asb, INPUT);


  //  Boot
  //	digitalWrite(vtecanzeige, HIGH);
  analogWrite(beleuchtung, 255);
  lcd.setCursor(0, 0);
  lcd.print("####################");
  lcd.setCursor(0, 1);
  lcd.print("#  <<<Carduino>>>  #");
  lcd.setCursor(0, 2);
  lcd.print("#      Version 1.4b#");
  lcd.setCursor(0, 3);
  lcd.print("####################");
  delay(500);
  lcd.clear();
}


void loop()
{
  // Dimmen der Displayhelligkeit per LDR um bei Dunkelheit das Display abzudimmen
  int val = analogRead(sensorhelligkeit);
  val = constrain (val, 100, 700);
  int ledLevel = map(val, 700, 100, 255, 20);
  analogWrite(beleuchtung, ledLevel);

  error = Sensoraussen.getTSicTemp(&aussentemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensorinnen.getTSicTemp(&innentemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensormotorraum.getTSicTemp(&motorraumtemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF
  error = Sensoransaugluft.getTSicTemp(&ansauglufttemperatur);  //turn the TSIC-Sensor ON -> messure -> OFF


  int oeldruck = analogRead(sensoroeldruck);
  oeldruck = map(oeldruck, 80, 900, 0, 100);
  //char oeldruckbuffer [50] ;
  double doubleoeldruck = oeldruck / 10.0;
  dtostrf(doubleoeldruck, 2, 1, oeldruckbuffer);
  oeltemp = analogRead(sensoroeltemp);
  oeltemp = map(oeltemp, 0, 1023, -30, 197);
  boardspannung = analogRead(sensorbordspannung);
  boardspannung = map(boardspannung, 0, 1023, 0, 243);
  double doubleboardspannung = boardspannung / 10.0;
  dtostrf(doubleboardspannung, 2, 1, boardspannungbuffer);



  // VTEC-light anzeige
  vtecState = digitalRead(vtec);

  if (vtecState == HIGH)
  {
    lcd.setCursor(19, 0);
    lcd.print("V");
    lcd.setCursor(19, 1);
    lcd.print("T");
    lcd.setCursor(19, 2);
    lcd.print("E");
    lcd.setCursor(19, 3);
    lcd.print("C");
  }
  else
  {
    lcd.setCursor(19, 0);
    lcd.print(" ");
    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(19, 2);
    lcd.print(" ");
    lcd.setCursor(19, 3);
    lcd.print(" ");
  }

  schalterstatus = digitalRead(schalter);

  if (schalterstatus == HIGH)
  {
    if (lastpage != 1)
    {
      lcd.clear();
    }
    (seite1());
    lastpage = 1;
  }
  else
  {
    if (lastpage != 2)
    {
      lcd.clear();
    }
    (seite2());
    lastpage = 2;
  }
  /*
  //Debugging
  Serial.println("----------------------------------------------------");
  Serial.println("Honduino build by SpitfireXP");
  Serial.println("Honduino@SpitfireXP.de");
  Serial.println("Honduino Version 1.4b");
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

  //	Serial.print(asb);
  //	Serial.println(" Schaltsaugbrücke");
  //	Serial.print(vtec);
  //	Serial.println(" VTEC");

  Serial.print(sensorhelligkeit);
  Serial.print("; ");
  Serial.print(val);
  Serial.print("; ");
  Serial.print(beleuchtung);
  Serial.print("; ");
  Serial.println(" Helligkeit");
  Serial.print(boardspannungbuffer);
  Serial.println(" Volt");
  */
}


// Ausgabe Seite 1
void seite1()
{
  lcd.setCursor(0, 0);
  lcd.print(oeldruckbuffer);
  lcd.print("Bar ");
  lcd.write(239);
  lcd.print("ldruck");

  lcd.setCursor(0, 1);
  lcd.print(oeltemp);
  lcd.print("°C ");
  lcd.write(239);
  lcd.print("ltemperatur ");

  lcd.setCursor(0, 2);
  lcd.print(boardspannungbuffer);
  lcd.print("V Boardspannung");
}

// Ausgabe Seite 2
void seite2()
{
  lcd.setCursor(0, 0);
  lcd.print(aussentemperatur);
  lcd.setCursor(3, 0);
  lcd.print("C Aussentemp.");
  lcd.setCursor(0, 1);
  lcd.print(innentemperatur);
  lcd.setCursor(0, 1);
  lcd.print("C Innentemp.");

  lcd.setCursor(0, 2);
  lcd.print(motorraumtemperatur);
  lcd.setCursor(3, 2);
  lcd.print("C Motorraum");
  lcd.setCursor(0, 3);
  lcd.print(ansauglufttemperatur);
  lcd.setCursor(3, 3);
  lcd.print("C Ansaugklappe");
}
