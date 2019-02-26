/*
DS3231_test.pde
Eric Ayars
4/11

Test/demo of read routines for a DS3231 RTC.

Turn on the serial monitor after loading this to check if things are
working as they should.

*/

#include <DS3231.h>
#include <Wire.h>
#include "U8glib.h"

DS3231 Clock;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // I2C / TWI 
bool Century=false;
bool h12;
bool PM;

void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC);
void setup() {
  // Start the I2C interface
  Wire.begin();
  // Start the serial interface
  Serial.begin(9600);
 /*
 Clock.setSecond(0);
Clock.setMinute(41);
Clock.setHour(22);
Clock.setDate(26);
Clock.setMonth(2);
Clock.setYear(19);
Clock.setDoW(2);
*/}

void loop() {
  // send what's going on to the serial monitor.
  // Start with the year

  
  Serial.print(Clock.getYear(), DEC);
  Serial.print('/');
  // then the month
  Serial.print(Clock.getMonth(Century), DEC);
  Serial.print('/');
  // then the date
  Serial.print(Clock.getDate(), DEC);
  Serial.print('-');
  // and the day of the week
  Serial.print(Clock.getDoW(), DEC);
  Serial.print('-');
  // Finally the hour, minute, and second
  Serial.print(Clock.getHour(h12, PM), DEC);
  Serial.print(':');
  Serial.print(Clock.getMinute(), DEC);
  Serial.print(':');
  Serial.print(Clock.getSecond(), DEC);
  // Add AM/PM indicator
  if (h12) {
    if (PM) {
      Serial.print(" PM ");
    } else {
      Serial.print(" AM ");
    }
  } else {
    Serial.print(" 24h ");
  }
  // Display the temperature
  Serial.print("T=");
  Serial.print(Clock.getTemperature(), 2);
  // Tell whether the time is (likely to be) valid
  if (Clock.oscillatorCheck()) {
    Serial.print(" O+");
  } else {
    Serial.print(" O-");
  }


    
  Serial.print('\n');
  Serial.print('\n');
  
  u8g.firstPage();  
  do {
draw(0,0,0,Clock.getHour(h12, PM),Clock.getMinute(),Clock.getSecond());
} while( u8g.nextPage() );

  delay(1000);
}


void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC){
//x y to pozycja lewego dolnego piksela pisanego tekstu
u8g.setFont(u8g_font_6x12);
u8g.setColorIndex(1);

 int x=0,y=7,xd=64; // wpolowie zaczniemy wartosci wiec x bedzie 64 rowne
u8g.drawStr( x, y, "godzina");
char buf[5];
dtostrf(houR, 4,1,buf);
u8g.drawStr( xd, y, buf);
y+=10;
char buf1[5];
dtostrf(minutE, 4,1,buf1);
u8g.drawStr( xd, y, buf1);
y+=10;
char buf2[5];
dtostrf(seC, 4,1,buf2);
u8g.drawStr( xd, y, buf2);
//zrobic konwersje wszytich
/*
u8g.drawStr( x, y, "tempDHT");
//robimy konwersje floata na string
char buf[5];
dtostrf(tempDHT, 4,1,buf);
u8g.drawStr( xd, y, buf);
y+=10;
u8g.drawStr( x, y, "wilgDHT");
char buf0[5];
dtostrf(wilgDHT, 4,1,buf0);
u8g.drawStr( xd, y, buf0);
y+=10;
u8g.drawStr( x, y, "Temp0/1");
char buf2[4];
sprintf (buf2, "%d",tempan0);//konwersja dypu int na string
char buf3[4];
sprintf (buf3,"%d",tempan1);//konwersja dypu int na string
u8g.drawStr( xd, y, buf2);
u8g.drawStr(xd+22,y,buf3);
y+=10;
u8g.drawStr( x, y, "jasnosc");
char buf4[4];
sprintf (buf4, "%d", swiatl);//konwersja dypu int na string
u8g.drawStr( xd, y, buf4);
y+=10;
u8g.drawStr( x, y, "bateria");
char buf5[6];//konwersja poziomu baterii na string z floata
dtostrf(volty, 5,2,buf5);
u8g.drawStr( xd, y, buf5);
y+=10;
u8g.drawStr( x, y, "czas");
char priodstep[11];
sprintf(priodstep,"%lu",millis());//lu bo unsigned long
u8g.drawStr( 44, y, priodstep);
*/
}
