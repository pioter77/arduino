//eeprom function and many digital pins are still not used!
#include <avr/sleep.h>
#include <DS3231.h>
#include <Wire.h>
#include "U8glib.h"
#include <AM2320.h>
#include <DHT.h>

 #define yaxis A2
 #define xaxis A3
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
bool first_run_condition=true;//variable to make the sensors readouts only at first run of loop in order not to slow down the manu response
bool not_raining=0;//logic value from rain sensor it will be set to 1 wen rain exceeds the value set on the sensor potentiometer
volatile boolean flag=false;//volatile becouse it is only changed by interrupt function

//bool first_run_condition=true;//variable to make the sensors readouts only at first run of loop in order not to slow down the manu response
bool sleep_state=false;

byte joystick_vcc=8;//vcc from joystick
//byte oled_state=0;//oled state when 0 oled should be off
uint16_t box_x_pos=0;//zmienne do punktu rysownaia wskaznika polozenia
uint16_t box_y_pos=61;//it can actually be a const couse it doesnt change it is 1 pizel lower than the line y 
uint16_t menu_option=0;//variable to show particular screens
unsigned int x_pos_prev=0;//to compare previous x position trully not needed any more after adding time delay
unsigned int t_prev=0;//for time delay for joystick to slow it down a little bit
const int joystick_sw=5;///joystick digital switch used to turn oled on and off
const int rain_vcc=7;// 5volts from rain sensor on digital to save energy during sleep
const int rain_sense_pin =6;//digital pin from rain sensor its value determines the not_raining variable
const int SW=2;//wakeup switch connected to arduino D2 is interrupt pin marked as 0

int rain_density=0;//wartosc do analogowego odcztu deszczu imn mniejsza tym wiekszy
float humi=0,temp=0;//to store the am sensor readouts (outside)
float humi_in=0,temp_in=0;//tostore dht 11 readouts (inside)

bool rain_checker(int &rain_density_cpy);
void joystick_push_to_sleep();//checks wheather to on or off the oled screen
void draw(void);//draw function mandatory for u8glib
byte position_check();//checks wheather the joystick is tilted or not
byte screen_option();//to determine what picture to display
void read_dht(float &hum_in,float &tempe_in);
void amsensor_Readout(float &humid,float &tempe);
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density,byte box_mov_dir);
void setFlag();
void ArduGoSleep();

void setup() {
  // Start the I2C interface
  Wire.begin();
  dht.begin();
  // Start the serial interface
 Serial.begin(115200);
 pinMode(joystick_vcc,OUTPUT);
  digitalWrite(joystick_vcc,LOW);
   pinMode(joystick_sw,INPUT_PULLUP);
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

  if(flag){
      
    if(first_run_condition==true){
     digitalWrite(joystick_vcc,HIGH);
    amsensor_Readout(humi,temp);//we are reading the values from am2320 sensor
    read_dht(humi_in,temp_in);//reading inside temperature values
    not_raining=rain_checker(rain_density);//checking weather the mesured level of rain is greater than the defined one
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
draw(0,0,0,Clock.getHour(h12, PM),Clock.getMinute(),Clock.getSecond(),humi,temp,not_raining,rain_density,position_check());
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
void draw(byte daY,byte montH,byte yeaR,byte houR,byte minutE,byte seC,float &humi_1,float &temp_1,bool not_raining_cpy,int &rain_density,byte box_mov_dir)
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
        u8g.drawStr(box_x_pos,40,"1");
        u8g.setPrintPos(0,y);
        u8g.print(houR);
        u8g.drawStr(11,y,":");
        u8g.setPrintPos(15,y);
        u8g.print(minutE);
        u8g.drawStr(27,y,":");
        u8g.setPrintPos(31,y);
        u8g.print(seC);
    break;

    case 2:
    char buf[5];
      dtostrf(humi_1, 6,2,buf);//zm do konwersji,ilosc miejsc calosci ze znakiem,ile po przecinku i bufor string do zapisu
      u8g.drawStr(0,y,"wilg: "); u8g.drawStr(xd,y,buf);
    break;

    case 3:
    char buf_t[6];//bo znak minus
    dtostrf(temp_1,6,2,buf_t);
    u8g.drawStr(0,y,"temp: "); u8g.drawStr(xd,y,buf_t);
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
    u8g.drawStr(box_x_pos,40,"6");
    break;

    case 7:
    u8g.drawStr(box_x_pos,40,"7");
    break;

    case 8:
    u8g.drawStr(box_x_pos,40,"8");
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


byte position_check()
{
            unsigned int t_now=millis();
  if((t_now-t_prev)>200) {//200 miliseconds

             unsigned int reading_posx= analogRead(xaxis);
 
  if(reading_posx<412)
    {
      x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
      t_prev=millis();
      return 2;//left
  }
  if(reading_posx>612)
  {
    x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
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
