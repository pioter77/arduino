//#include <Narcoleptic.h>
#include <avr/sleep.h>
#include <DS3231.h>
#include <Wire.h>
#include "U8glib.h"
#include <AM2320.h>

AM2320 thsensor;
DS3231 Clock;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // I2C / TWI 
bool Century=false;
bool h12;
bool PM;
volatile boolean flag=false;
const int SW=2;
float humi=0,temp=0;
byte counter=0;
char float_to_char(float &value);
void clearOLED();
void amsensor_readout(float &humid,float &tempe);
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1);
void setFlag();
void ArduGoSleep();

void setup() {
  // Start the I2C interface
  Wire.begin();
  // Start the serial interface
  Serial.begin(115200);
  pinMode(SW,INPUT_PULLUP);
 // amsensor_Readout(humi,temp);
  //delay(2000);
  float testingvalue=26.54;
  Serial.println(float_to_char(testingvalue));
  u8g.sleepOn();
  ArduGoSleep();
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

 /*
  // Display the temperature
  Serial.print("T=");
  Serial.print(Clock.getTemperature(), 2);
  // Tell whether the time is (likely to be) valid
  if (Clock.oscillatorCheck()) {
    Serial.print(" O+");
  } else {
    Serial.print(" O-");
  }
Serial.println(Clock.getHour(h12, PM));
 
  */
 // //if(digitalRead(SW)==LOW){
// amsensor_Readout(humi,temp);
  if(flag){
    
amsensor_Readout(humi,temp);
counter++;
Serial.println(counter);
    u8g.sleepOff();
    //amsensor_Readout(humi,temp);
  u8g.firstPage();  
   do {
draw(0,0,0,Clock.getHour(h12, PM),Clock.getMinute(),Clock.getSecond(),humi,temp);
} while( u8g.nextPage() );
//amsensor_Readout(humi,temp);
  delay(5000);
  u8g.sleepOn();
   flag=false;
  ArduGoSleep();
  }
  //Narcoleptic.delay(100);
 
}


void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1)
{
//x y to pozycja lewego dolnego piksela pisanego tekstu
u8g.setFont(u8g_font_6x12);
u8g.setColorIndex(1);

 int x=0,y=7,xd=64; // wpolowie zaczniemy wartosci wiec x bedzie 64 rowne
u8g.drawStr( x, y, "godzina");
y+=10;
//char buf[5];
//dtostrf(houR, 4,1,buf);
//char buf=houR;
//u8g.drawStr( 0, y, buf);
u8g.setPrintPos(0,y);
u8g.print(houR);
u8g.drawStr(11,y,":");
u8g.setPrintPos(15,y);
u8g.print(minutE);
u8g.drawStr(27,y,":");
u8g.setPrintPos(31,y);
u8g.print(seC);
y+=10;
char buf[5];
dtostrf(humi_1, 6,2,buf);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
u8g.drawStr(0,y,"wilg: "); u8g.drawStr(xd,y,buf);
y+=10;
char buf_t[6];//bo znak minus
dtostrf(temp_1,6,2,buf_t);
u8g.drawStr(0,y,"temp: "); u8g.drawStr(xd,y,buf_t);
}

void setFlag()
{
  flag=true;
}

void ArduGoSleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0,setFlag,LOW);//D2 pin jako switch
  sleep_mode();//tu wchodzi w sleep
  //wywaola zmieni flage wejdzie w setup i loop akutalizacja oleda 5 sek i flaga na 0 i wywola ta funkcje od poczatku]
  sleep_disable();
  detachInterrupt(0);
}
void amsensor_Readout(float &humid,float &tempe)
{
  byte case_holder;//ma tzymac warunek ktory zwraca funkcja odczytujaca
  case_holder=thsensor.Read();
  if(case_holder==0)
  {
     humid=thsensor.h;
     tempe=thsensor.t;
     Serial.println(tempe);
     delay(200);
  }else if(case_holder==1) {
    humid=0.0;
    tempe=0.0;
  }else if(case_holder==2){
    humid=0.0;
    tempe=0.0;
  }
  
 /*switch (thsensor.Read()==0)
  { case 0:
     humid=thsensor.h;
     tempe=thsensor.t;
     delay(200);
    // delay(200);
    // Serial.println("wilgotno: ");
    // Serial.println(humid);
     //Serial.println("temperatura: ");
     //Serial.println(tempe);
     break;
  case 2: 
  Serial.println("crc failed");
  //humid=0;
  //tempe=0;
  break;
  case 1:
  Serial.println("sensor offline");
  //humid=0;
  //tempe=0;
  break;
  }*/
}

void clearOLED(){
    u8g.firstPage(); 
    do {
    } while( u8g.nextPage() );
}

char float_to_char(float &value)
{
  char buf[6];
  dtostrf(value,6,2,buf);
  return buf[6];
}

