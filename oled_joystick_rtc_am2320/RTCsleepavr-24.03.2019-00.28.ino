//eeprom function and many digital pins are still not used!
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include <Wire.h>
#include "U8glib.h"
#include <AM2320.h>
#include <DHT.h>

const byte solar_panel_sense_pin=A7;
const byte batt_vcc_sense_pin=A6;//battery level measurment pin
#define sdpin 10//pin sd
#define light_sensor_pin A1 //analog sensor pin for light sensor
 #define yaxis A2 //joystick yaxis pin[analog]
 #define xaxis A3//joystick xaxis pin[analog]
#define dhtpin 9//dht 11 pin remember to use 10k resistor betwwen it and vcc of the sensor
#define DHTTYPE DHT11 // dht 11 ale czy t ojest ten z ujemnymi temp?
#define analog_rain_pin A0 //precise analog rain sensor values from this pin
AM2320 thsensor;//inicjalizacja obiektu sensora am2320
File plikdane;
DS3231 Clock;//inicjalizacjaa obiektu zeara
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // I2C / TWI wyswietlacz oled i2c
DHT dht(dhtpin,DHTTYPE);// dht sensor object inciialisation
 bool Century=false;//nie wiem czy to potrzebne to było do zegara rtc ,tak dokąldnie do daty
 bool h12;//clock varible
 bool PM;//clock variable
bool first_run_condition=true;//variable to make the sensors readouts only at first run of loop in order not to slow down the manu response
bool not_raining=0;//logic value from rain sensor it will be set to 1 wen rain exceeds the value set on the sensor potentiometer
volatile boolean flag=false;//volatile becouse it is only changed by interrupt function

//bool first_run_condition=true;//variable to make the sensors readouts only at first run of loop in order not to slow down the manu response
bool sleep_state=false;

const byte joystick_vcc=8;//vcc from joystick
//byte oled_state=0;//oled state when 0 oled should be off
uint16_t box_x_pos=0;//zmienne do punktu rysownaia wskaznika polozenia
const uint16_t box_y_pos=61;//it can actually be a const couse it doesnt change it is 1 pizel lower than the line y 
uint16_t menu_option=0;//variable to show particular screens
//unsigned int x_pos_prev=0;//to compare previous x position trully not needed any more after adding time delay
unsigned int t_prev=0;//for time delay for joystick to slow it down a little bit
const int joystick_sw=5;///joystick digital switch used to turn oled on and off
const int rain_vcc=7;// 5volts from rain sensor on digital to save energy during sleep
const int rain_sense_pin =6;//digital pin from rain sensor its value determines the not_raining variable
const int SW=2;//wakeup switch connected to arduino D2 is interrupt pin marked as 0
const int light_sensor_vcc=9;//should be high when doing measurment then immediatelly low to save power!
const int batt_in_series_for_power=1;//number of lion cells powering up the arduino 2 for 2s 1 for 1s packs

int rain_density=0;
int light_density=0;//for measurnig light
int hour_r=0,minute_r=0,seconds_r=0,year_r=0,month_r=0,day_r=0;//variables to store clock readouts
float humi=0,temp=0;//to store the am sensor readouts (outside)
float humi_dht=0,temp_dht=0;//tostore dht 11 readouts (inside)
float batt_level=0;
float solar_voltage=0;

void logger(byte day_r,byte month_r,byte year_r,byte hour_r,byte minute_r,
            byte seconds_r,float humi,float temp,float humi_dht,float temp_dht,
            bool not_raining,int rain_density,int light_density,float batt_level,
            float solar_voltage);
void battery_readout(byte sensor_pin_volt,uint16_t batt_in_series,float &batt_level);
bool rain_checker(int &rain_density_cpy);
void joystick_push_to_sleep();//checks wheather to on or off the oled screen

byte position_check();//checks wheather the joystick is tilted or not
int clock_reading(byte what_to_read);//depending on the passed value returns 1-hour, 2-minutes,3-seconds,4-year,5-month,6-day
byte screen_option();//to determine what picture to display
void read_dht(float &humi_dht,float &temp_dht);//function to read from dht sensor
void amsensor_Readout(float &humid,float &tempe);//function to read from am2320 senor
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density,byte box_mov_dir,int light_density_cpy,float &humi_dht,float &temp_dht,float &solar_volatge_org);
void setFlag();//function tht is runned when an innterupt happens
void ArduGoSleep();//function that puts arduino to deep sleep arduino is being held in that function until next interrupt
void read_dht(float &hum_dht,float &tempe_dht);//function to read values from dht 11 sensor

void setup() {
  // Start the I2C interface
  Wire.begin();
  dht.begin();//start dht sensor
    
SD.begin(sdpin);
  // Start the serial interface
 //Serial.begin(115200);
 pinMode(joystick_vcc,OUTPUT);
  digitalWrite(joystick_vcc,LOW);
   pinMode(joystick_sw,INPUT_PULLUP);
  pinMode(rain_vcc,OUTPUT);
  digitalWrite(rain_vcc,LOW);//off immediatelly, shoud be pulled high only during measurment
 // pinMode(clock_vcc,OUTPUT);
  //digitalWrite(clock_vcc,LOW);//off immediatelly, shoud be pulled high only during measurment
  pinMode(SW,INPUT_PULLUP);
  pinMode(rain_sense_pin,INPUT);
  
 // while(SD.begin(sdpin)==0) {
   // Serial.println("card error");
   
   
    //delay(5000);
  //}
    bool istnienie=0;
    
 if(SD.exists("dane.txt")) istnienie=1;
 
  
if(istnienie==1) {
   plikdane=SD.open("dane.txt",FILE_WRITE);
 // Serial.println("reset zasilania byl");
  plikdane.print("PRZERWA");
  plikdane.println();
  plikdane.close();
}

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

  if(flag){
      
    if(first_run_condition==true){
     digitalWrite(joystick_vcc,HIGH);
    battery_readout(batt_vcc_sense_pin,batt_in_series_for_power,batt_level);//checks battery level
    battery_readout(solar_panel_sense_pin,3,solar_voltage);//checks solar voltage level
    amsensor_Readout(humi,temp);//we are reading the values from am2320 sensor
    read_dht(humi_dht,temp_dht);//reading inside temperature values
    not_raining=rain_checker(rain_density);//checking weather the mesured level of rain is greater than the defined one
    light_density=light_sensor_readout(light_sensor_pin,light_sensor_vcc);
    
    hour_r=clock_reading(1);//gethour
    minute_r=clock_reading(2);//get miutes
    seconds_r=clock_reading(3);//get seconds value
    year_r=clock_reading(4);//getyear
    month_r=clock_reading(5);//get month
    day_r=clock_reading(6);//get day
    logger(day_r,month_r,year_r,hour_r,minute_r,seconds_r,humi,temp,humi_dht,temp_dht,not_raining,rain_density,light_density,batt_level,solar_voltage);
    }
    u8g.sleepOff();//waking up the oled screen
     joystick_push_to_sleep();
//at the same time getting preecise rain value to the global variable rain_density
//not_raining=1 when no rain, 0 when it rain

    
      position_check();
        // Serial.println("pozycja");
  //  digitalWrite(clock_vcc,HIGH);
    //amsensor_Readout(humi,temp);
  u8g.firstPage();  
   do {
draw(day_r,month_r,year_r,hour_r,minute_r,seconds_r,humi,temp,not_raining,rain_density,position_check(),light_density,humi_dht,temp_dht,batt_level,solar_voltage);
//draw(0,0,0,0,0,0,humi,temp,not_raining,rain_density,position_check());
} while( u8g.nextPage() );//end of the picture loop
//amsensor_Readout(humi,temp);
  //delay(5000);//5s oled working time after that it goes to sleep
 // digitalWrite(clock_vcc,LOW);

   first_run_condition=false;
   
   if(sleep_state==true){
    sleep_state=false;
   // Serial.println("funkcja2");
    first_run_condition=true;
    flag=false;//flag is set to 0 and it will be changed when the next interrupt on D2 occurs
    u8g.sleepOn();//after switch from joystick pushed oled goes to sleeep
    digitalWrite(joystick_vcc,LOW);
  ArduGoSleep();//going for sleep function
   }
  }
  
 
}


//this function needs modifying still, to make it more readable and to make its oled output look more representative
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density,byte box_mov_dir,int light_density_cpy,
float &humi_dht,float &temp_dht,float batt_level_cpy,float &solar_voltage_org)
{
//x y to pozycja lewego dolnego piksela pisanego tekstu
u8g.setFont(u8g_font_6x12);
u8g.setColorIndex(1);
int x=0,y=7,xd=64; // wpolowie zaczniemy wartosci wiec x bedzie 64 rowne
byte box_length=16;//pixels
  byte box_width=3;//pixels

  u8g.drawLine(0,62,128,62);//line for slider
  //slider drawing
  if(box_mov_dir==0)
  {
    u8g.drawBox(box_x_pos,box_y_pos,box_length,box_width);
  }
  else if((box_mov_dir==1)&&(box_x_pos<=111))
  {
    //to right
    box_x_pos+=16;
   u8g.drawBox(box_x_pos,box_y_pos,box_length,box_width);
    
  }
  else if((box_mov_dir==2)&&(box_x_pos>=16))
  {
    box_x_pos-=16;
    u8g.drawBox(box_x_pos,box_y_pos,box_length,box_width);
    //to left
  }

  ///////////////////////////////////////////
  switch(screen_option()){
    case 1:
    u8g.drawStr( x, y, "godzina");
    y+=10;
        u8g.setPrintPos(0,y);
        u8g.print(houR);
        u8g.drawStr(11,y,":");
        u8g.setPrintPos(15,y);
        u8g.print(minutE);
        u8g.drawStr(27,y,":");
        u8g.setPrintPos(31,y);
        u8g.print(seC);
        y=40;
             u8g.setPrintPos(0,y);
        u8g.print(daY);
        u8g.drawStr(11,y,"-");
        u8g.setPrintPos(15,y);
        u8g.print(montH);
        u8g.drawStr(27,y,"-");
        u8g.setPrintPos(31,y);
        u8g.print(yeaR);
    break;

    case 2:
    char buf[5];
      dtostrf(humi_1, 6,2,buf);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
      u8g.drawStr(0,y,"wilg: "); u8g.drawStr(xd,y,buf);
        char buf_t[6];//bo znak minus
    dtostrf(temp_1,6,2,buf_t);
    u8g.drawStr(0,y+10,"temp: "); u8g.drawStr(xd,y+10,buf_t);
    break;


    case 3:
    char buf_batt[3];
    dtostrf(batt_level_cpy, 4,2,buf_batt);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
      u8g.drawStr(0,y+10,"batt_level: "); u8g.drawStr(xd,y+10,buf_batt);

      char buf_solar[3];
    dtostrf(solar_voltage_org, 4,2,buf_solar);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
      u8g.drawStr(0,y+20,"solar_lev: "); u8g.drawStr(xd,y+20,buf_solar);
      
    break;

    case 4:
    u8g.drawStr(0,y,"no rain?: ");
    u8g.setPrintPos(xd,y); u8g.print(not_raining_cpy);
    break;

    case 5:
    u8g.drawStr(0,y,"rain density: ");
    u8g.setPrintPos(xd+30,y); u8g.print(rain_density);//+30 to prevent both columns from meeting
    break;

    case 6:
    u8g.drawStr(0,y,"light density: ");
    u8g.setPrintPos(xd+30,y); u8g.print(light_density_cpy);//+30 to prevent both columns from meeting
    break;

    case 7:
    char buf_dht_h[5];
      dtostrf(humi_dht, 6,2,buf_dht_h);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
      u8g.drawStr(0,y,"wilg in: "); u8g.drawStr(xd,y,buf_dht_h);
    break;

    case 8:
    char buf_dht_t[6];//bo znak minus
    dtostrf(temp_dht,6,2,buf_dht_t);
    u8g.drawStr(0,y,"temp in: "); u8g.drawStr(xd,y,buf_dht_t);
    break;
    
  }
}
  ////////////////////////////////
 

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
    // Serial.println(tempe);
     delay(200);
  }else if(case_holder==1) {
    humid=0.0;
    tempe=0.0;
  //  Serial.println("1");
  }else if(case_holder==2){
    humid=0.0;
    tempe=0.0;
    //Serial.println("2");
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

void read_dht(float &humi_dht,float &temp_dht)
{
  //do{
  humi_dht=dht.readHumidity();
  temp_dht=dht.readTemperature();
 // }
}

void battery_readout(byte sensor_pin_volt,uint16_t batt_in_series,float &batt_level)
{
  //it will only work until battery volatage is single cell one
//with 2s pack you will get 8.2v and that wil kill arduino
//needs then a volatage divider
int readout_value_batt=analogRead(sensor_pin_volt);
//Serial.println(readout_value_batt);

switch(batt_in_series){
    case 1:
        //if arduino uses li-ion 1s battery pack,better for lower power consumption projects
        batt_level=(5.0*readout_value_batt)/1024.0;
      break;

    case 2:
        //if ardu uses 2s li-ion battery pack
        //volatge divider 2x 10k resistors in series A6 connected to the middle resistor pins
        //left series resistor pin to battery vcc 9V! right to gnd middle A6
        //note that powering it with 2s and mesuring battery level using this technique will increase power consumption 
        //becouse 2x10k volltage drop of 8.4v will consume additional 0.42mA of current
        batt_level=(float)readout_value_batt*5.0*2.0/1024.0;
      break;

    case 3://if we want to measure 6v solar panel similiar setup for 2s cell
    
            batt_level=(float)readout_value_batt*5.0*2.0/1090.0;//compensation for making true readout
      break;

}
}

int light_sensor_readout(byte readout_pin,byte sensor_vcc)//should be placed in a if statement that ony runs at first loop run
{
  digitalWrite(sensor_vcc,HIGH);//gives 5v to other sensor pin(analog)
  int light=analogRead(readout_pin);
  digitalWrite(sensor_vcc,LOW);//shuts it off to reduce power consumption
  return light;
}


byte position_check()
{
            unsigned int t_now=millis();
  if((t_now-t_prev)>200) {//200 miliseconds

             unsigned int reading_posx= analogRead(xaxis);
 
  if(reading_posx<412)
    {
     // x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
      t_prev=millis();
      return 2;//left
  }
  if(reading_posx>612)
  {
//    x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
    t_prev=millis();
    return 1;//right
  }
                        }
  return 0;
}

byte screen_option()
{ 
  const uint16_t segment_leng=16;
  uint16_t help_var=15;//jak na 8 segnentow jest podzielone to segment ma 16 pikseli
  //wiec paczetek 2giego segmentu to 15 piksel i potem dodajemy 16 po kolejnym sprawdzeniu
  //jak sprawdzamy od najmiejszego to nie trzeba jakisc przedzialow
  if(box_x_pos<help_var) return 1;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 2;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 3;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 4;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 5;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 6;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 7;
  help_var+=segment_leng;
  if(box_x_pos<help_var) return 8;//menu screen no 8
  
  }


 void joystick_push_to_sleep()
{
 //Serial.println("funkcja");
  if(digitalRead(joystick_sw)==LOW)
  {
    //Serial.println("joy wcisniety  ");
  sleep_state=true;
  //Serial.print(sleep_state);
        }
 
}



void logger(byte day_r,byte month_r,byte year_r,byte hour_r,byte minute_r,
            byte seconds_r,float humi,float temp,float humi_dht,float temp_dht,
            bool not_raining,int rain_density,int light_density,float batt_level,
            float solar_voltage)
{
  plikdane=SD.open("dane.txt",FILE_WRITE);
 plikdane.print(day_r);
  plikdane.print(";");
  plikdane.print(month_r);
  plikdane.print(";");
  plikdane.print(year_r);
  plikdane.print(";");
  plikdane.print(hour_r);
  plikdane.print(";");
  plikdane.print(minute_r);
  plikdane.print(";");
  plikdane.print(seconds_r);
  plikdane.print(";");
  plikdane.print(humi);
  plikdane.print(";");
  plikdane.print(temp);
  plikdane.print(";");
  plikdane.print(humi_dht);
  plikdane.print(";");
  plikdane.print(temp_dht);
  plikdane.print(";");
  plikdane.print(not_raining);
  plikdane.print(";");
  plikdane.print(rain_density);
  plikdane.print(";");
  plikdane.print(light_density);
  plikdane.print(";");
  plikdane.print(batt_level);
  plikdane.print(";");
  plikdane.print(solar_voltage);
   
   plikdane.println();
  plikdane.close();
 
}


int clock_reading(byte what_to_read)
{

  switch(what_to_read)
  {
    case 1:
          return Clock.getHour(h12, PM);//hour returned
    break;

    case 2:
          return Clock.getMinute();//minutes vale returned
    break;

    case 3:
          return Clock.getSecond();//secs returned
    break;

    case 4:
          return Clock.getYear();
    break;

    case 5:
          return Clock.getMonth(Century);
    break;
          
    case 6:
          return Clock.getDate();
    break;
//space left for adding date displaying;
        
  }
}

