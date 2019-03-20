 #include <Wire.h>
#include "U8glib.h"
 
 #define yaxis A2
 #define xaxis A3

byte joystick_vcc=8;//vcc from joystick
byte oled_state=0;//oled state when 0 oled should be off
uint16_t box_x_pos=0;//zmienne do punktu rysownaia wskaznika polozenia
uint16_t box_y_pos=61;//it can actually be a const couse it doesnt change it is 1 pizel lower than the line y 
uint16_t menu_option=0;//variable to show particular screens
unsigned int x_pos_prev=0;//to compare previous x position trully not needed any more after adding time delay
unsigned int t_prev=0;//for time delay for joystick to slow it down a little bit
const int encoder_sw=5;//encoder digital switch used to turn oled on and off



void encoder_push();//checks wheather to on or off the oled screen
void draw(void);//draw function mandatory for u8glib
byte position_check();//checks wheather the joystick is tilted or not
byte screen_option();//to determine what picture to display

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // I2C / TWI wyswietlacz oled i2c

 void setup() { 
  
   pinMode(joystick_vcc,OUTPUT);
   digitalWrite(joystick_vcc,LOW);
   pinMode(encoder_sw,INPUT_PULLUP);
  u8g.sleepOn();//oled sleep at he power on
   
   //Serial.begin (115200);
 } 

 void loop() { 
    encoder_push();
  if(oled_state==1) {digitalWrite(joystick_vcc,HIGH);}else {digitalWrite(joystick_vcc,LOW);}
  position_check();
 
 u8g.firstPage();  
  do {
    draw(position_check());
  } while( u8g.nextPage() );
 }

 
 void encoder_push()
{
  digitalWrite(joystick_vcc,HIGH);
  if(digitalRead(encoder_sw)==LOW)
  {
    //Serial.println("funkcja");
  oled_state=!oled_state;
        }
  if(oled_state==1) {u8g.sleepOff();}else
  { u8g.sleepOn();}
  digitalWrite(joystick_vcc,LOW);
}

byte position_check()
{
            unsigned int t_now=millis();
  if((t_now-t_prev)>200) {//200 miliseconds

             unsigned int reading_posx= analogRead(xaxis);
 
  if(reading_posx<412)//&&(reading_posx!=x_pos_prev)) 
    {
      x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
      t_prev=millis();
      return 2;//left
  }
  if(reading_posx>612)//&&(reading_posx!=x_pos_prev)) 
  {
    x_pos_prev=reading_posx;//zapamietuje poprzednia pozycje jesli sie roznia to zrobi krok
    t_prev=millis();
    return 1;//right
  }
                        }
  return 0;
}

void draw(byte box_mov_dir) {
  byte box_length=16;//pixels
  byte box_width=3;//pixels
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 0, 22, "Hello World!");
  u8g.drawLine(0,62,128,62);
  
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
  switch(screen_option()){
    case 1:
    u8g.drawStr(box_x_pos,40,"1");
    break;

    case 2:
    u8g.drawStr(box_x_pos,40,"2");
    break;

    case 3:
    u8g.drawStr(box_x_pos,40,"3");
    break;

    case 4:
    u8g.drawStr(box_x_pos,40,"4");
    break;

    case 5:
    u8g.drawStr(box_x_pos,40,"5");
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
