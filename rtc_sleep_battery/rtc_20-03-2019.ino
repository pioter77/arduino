//eeprom function and many digital pins are still not used!
#include <avr/sleep.h>
#include <DS3231.h>
#include <Wire.h>
#include "U8glib.h"
#include <AM2320.h>
#include <DHT.h>

#define dhtpin 8
#define DHTTYPE DHT11 // dht 11 ale czy t ojest ten z ujemnymi temp?
#define analog_rain_pin A0 //precise analog rain sensor values from this pin
AM2320 thsensor;//inicjalizacja obiektu sensora am2320
DS3231 Clock;//inicjalizacjaa obiektu zeara
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // I2C / TWI wyswietlacz oled i2c
DHT dht(dhtpin,DHTTYPE);
bool Century=false;//nie wiem czy to potrzebne
bool h12;//clock varible
bool PM;//clock variable

bool not_raining=0;//logic value from rain sensor it will be set to 1 wen rain exceeds the value set on the sensor potentiometer
volatile boolean flag=false;//volatile becouse it is only changed by interrupt function

//const int clock_vcc=12;//vcc for clock module
const int rain_vcc=7;// 5volts from rain sensor on digital to save energy during sleep
const int rain_sense_pin =6;//digital pin from rain sensor its value determines the not_raining variable
const int SW=2;//wakeup switch connected to arduino D2 is interrupt pin marked as 0

int rain_density=0;//wartosc do analogowego odcztu deszczu imn mniejsza tym wiekszy
float humi=0,temp=0;//to store the am sensor readouts (outside)
float humi_in=0,temp_in=0;//tostore dht 11 readouts (inside)

void read_dht(float &hum_in,float &tempe_in);
void amsensor_readout(float &humid,float &tempe);
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density);
void setFlag();
void ArduGoSleep();

void setup() {
  // Start the I2C interface
  Wire.begin();
  dht.begin();
  // Start the serial interface
  Serial.begin(115200);
  pinMode(rain_vcc,OUTPUT);
  digitalWrite(rain_vcc,LOW);//off immediatelly, shoud be pulled high only during measurment
 // pinMode(clock_vcc,OUTPUT);
  //digitalWrite(clock_vcc,LOW);//off immediatelly, shoud be pulled high only during measurment
  pinMode(SW,INPUT_PULLUP);
  pinMode(rain_sense_pin,INPUT);
  
 // amsensor_Readout(humi,temp);
  //delay(2000);
 // float testingvalue=26.54;
 // Serial.println(float_to_char(testingvalue));
  u8g.sleepOn();//turn off oled
  ArduGoSleep();//arduino goes to sleep and waits for an interrupt to occur

// uncomment this only once to set the rtc values to correct ones eg.after relacing cr2320 battery of the clock
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
    
amsensor_Readout(humi,temp);//we are reading the values from am2320 sensor
read_dht(humi_in,temp_in);//reading inside temperature values
not_raining=rain_checker(rain_density);//checking weather the mesured level of rain is greater than the defined one
//at the same time getting preecise rain value to the global variable rain_density
//not_raining=1 when no rain, 0 when it rain

    u8g.sleepOff();//waking up the oled screen
  //  digitalWrite(clock_vcc,HIGH);
    //amsensor_Readout(humi,temp);
  u8g.firstPage();  
   do {
draw(0,0,0,Clock.getHour(h12, PM),Clock.getMinute(),Clock.getSecond(),humi,temp,not_raining,rain_density);
} while( u8g.nextPage() );//end of the picture loop
//amsensor_Readout(humi,temp);
  delay(5000);//5s oled working time after that it goes to sleep
 // digitalWrite(clock_vcc,LOW);
  u8g.sleepOn();//after 5 sec oled goes to sleee
   flag=false;//flag is set to 0 and it will be changed when the next interrupt on D2 occurs
  ArduGoSleep();//going for sleep function
  }
  
 
}


//this function needs modifying still, to make it more readable and to make its oled output look more representative
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density)
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
y+=10;
u8g.drawStr(0,y,"no rain?: ");
u8g.setPrintPos(xd,y); u8g.print(not_raining_cpy);
y+=10;
u8g.drawStr(0,y,"rain density: ");
u8g.setPrintPos(xd+30,y); u8g.print(rain_density);//+30 to prevent both columns from meeting

}

void setFlag()//flag changed to 1 when interrupt occurs 
//when on 1 everything in the main loop can run
{
  flag=true;
}

void ArduGoSleep()//function to manage arduino sleep mode
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0,setFlag,LOW);//D2 pin jako switch
  sleep_mode();//tu wchodzi w sleep
  //wywaola zmieni flage wejdzie w setup i loop akutalizacja oleda 5 sek i flaga na 0 i wywola ta funkcje od poczatku]
  sleep_disable();
  detachInterrupt(0);
}


void amsensor_Readout(float &humid,float &tempe)//function to read values from am2320 i2c sensor
{
  byte case_holder;//ma tzymac warunek ktory zwraca funkcja odczytujaca
  do{
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
    Serial.println("1");
  }else if(case_holder==2){
    humid=0.0;
    tempe=0.0;
    Serial.println("2");
  }
  }while(case_holder!=0);

}


bool rain_checker(int &rain_density_cpy)
{
  digitalWrite(rain_vcc,HIGH);//power on to rain sensor
  bool temp_var=digitalRead(rain_sense_pin);
rain_density_cpy=analogRead(analog_rain_pin);//analog rain ddensity measurment 
//the greater the number the weaker the rain is becouse greater resistance and higher voltage drop 
digitalWrite(rain_vcc,LOW);//power savings
  return temp_var;//returns 0 when not raining, 1 when it rains
}

void read_dht(float &hum_in,float &tempe_in)
{
  humi_in=dht.readHumidity();
  tempe_in=dht.readTemperature();
}

void battery_readout(byte batt_anlog_pin)
{
  
}

int light_sensor_readout(byte readout_pin)
{//it will only work until battery volatage is single cell one
//with 2s pack you will get 8.2v and that wil kill arduino
//needs then a volatage divider
  return analogRead(readout_pin);
}
