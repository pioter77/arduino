
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "DHT.h"
//#include <Time.h>

#include "U8glib.h"

#define dhtp 2
#define dhtyp DHT22
#define sdpin 10

File plikdane;

unsigned long zaptime1=0;
unsigned long actime=0;
unsigned long czas=60000;//60 sek odstepu


bool warun1=0;//warunk zmienia sie na 1 po 1szym zapisie i potem tak zostaje,poczebny zrby wyswietlacz nie wyswietlal czegos jak jeszcze nie bylo pomiaru iz eby odplil sie na 15 sekund po wykonaniu pomiaru
float wilg=0,temp=0;//na to referencje beda przekazywac sie wartosci w funkcjach
bool warunek=0;
int opcja=0;
int powtarzalnosc=1;
int odcz0=0,odcz1=0,odcz2,odcz3=0;//odcz0 A0 termistor0,odcz1 A1 termistor1,odcz2 A2 fotorezystor,odcz3 A3 plus baterii
DHT dht(dhtp,dhtyp);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 

//funkcja mierzenie dostaje referencje na parametry z funkcji odczyt 
void zapis(int a);
void mierzenie(unsigned long odstep,float &wilgmierz,float &tempmierz,bool &warun);//bedzie po prostu mierzenie(czas); i to dostanie oryginal do zmiany wartosci żeby nie tworzyc nowej zmiennejo pamiec chodzi
void wyswietlanie(float &wilgodcz,float &tempodcz,bool &warun);
void logger();
void odczytywanie();
//void draw(int opcja1,float temP,float wilG,int temP0,int temP1,int jasn);
void draw();

void aktualizacja();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  dht.begin();//zaczynamy transmisje z czyjnika dht

    

   Serial.println("gotowy");
  Serial.println("szukam karte");


while(SD.begin(sdpin)==0){ //sprawdzamy komunikacje z karta SD
  
    Serial.println("nie wykryto karty");
   // opcja=1;
   
  
       delay (500);
  // opcja=0;
    //nie zacznie programu az nie znajdzie karty
    
}


  Serial.println("karta wykryta, zaczynam");
  //opcja=2;
  
//opcja=0;
  
   bool istnienie=0;
 if(SD.exists("dane.txt")) istnienie=1;
 
  
if(istnienie==1) {
   plikdane=SD.open("dane.txt",FILE_WRITE);
 // Serial.println("reset zasilania byl");
  plikdane.print("PRZERWA");
  plikdane.println();
  plikdane.close();
}
  

}

void loop() {
  // make a string for assembling the data to log:
  mierzenie(czas,wilg,temp,warunek);
  wyswietlanie(wilg,temp,warunek);
aktualizacja();
if(warun1==1)//zalozenie jest takie ze bedzie przez 15 sekund po wykonaniu zapisu sie ta petla wyswietlania robic
{
u8g.firstPage();  
  do {
    draw(temp,wilg,odcz0,odcz1,odcz2,odcz3);
  } while( u8g.nextPage() );
}
//  Serial.println(wilgmierz);
 // Serial.println(tempmierz);
  
}



void mierzenie(unsigned long odstep,float &wilgmierz,float &tempmierz,bool &warun)
{
  actime=millis();
 unsigned long differ=actime-zaptime1;

  if(differ>=odstep)
  {
  //  Serial.println(differ);
wilgmierz=dht.readHumidity();
tempmierz=dht.readTemperature();
    if(isnan(tempmierz)||isnan(wilgmierz))
    {
     Serial.println("cos nie tak poszlo i odczyt bledny");
      warun=0;
    }else{
       warun=1;
    zaptime1=millis();//czas od 
    }
  }
}

//funkcja zapisujaca na SD oraz odczytujaca analogi
void wyswietlanie(float &wilgodcz,float &tempodcz,bool &warunek1)
{
 if(warunek1==1)
 {
  Serial.println("funkcja drukujaca");
   Serial.print("Humidity: "); 
    Serial.print(wilgodcz);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
   Serial.print(tempodcz);
    Serial.println(" *C");;
  warunek1 = !warunek1;
 // Serial.println(warunek1);
 // Serial.println(zaptime1);
 //odczyt analogów
  odczytywanie(odcz0,odcz1,odcz2,odcz3);
  logger(wilgodcz,tempodcz,odcz0,odcz1,odcz2,odcz3);
  //zmienia warunek na pozytywany, rozpoczyna aktualizacje wyswietlacza
  //mozna funkcji draw dac te same wartosci jak do funkcji logger
  warun1=1;
 
  
  //odczytywanie();
 }
}


void logger(float &wilgotnosc,float &temperatura,int &odczy0, int &odczy1,int &odczy2,int &odczy3)
{
  plikdane=SD.open("dane.txt",FILE_WRITE);
  plikdane.print(zaptime1);
  plikdane.print(";");
  plikdane.print(wilgotnosc);
   plikdane.print(";");
  plikdane.print(temperatura);
   plikdane.print(";");
  plikdane.print(odczy0);
   plikdane.print(";");
  plikdane.print(odczy1);
   plikdane.print(";");
  plikdane.print(odczy2);
  plikdane.print(";");
  plikdane.print(odczy3);
   plikdane.println();
  plikdane.close();
 // Serial.println("zapisano czas i danex2!");
 // Serial.println(odczy0);
 // Serial.println(odczy1);
 // Serial.println(odczy2);
}

void odczytywanie(int &odczyt0,int &odczyt1,int &odczyt2,int &odczyt3)
{
   odczyt0=analogRead(A0);//termistor0
   odczyt1=analogRead(A1);//termistor1
   odczyt2=analogRead(A2);//fotorezystor
   odczyt3=analogRead(A3);//poziom baterii 0d 0 d0 1023
 // Serial.println(odczyt0);
  //Serial.println(odczyt1);
 // Serial.println(odczyt2);
  
}

//void draw(int opcja1,float temP,float wilG,int temP0,int temP1,int jasn) {
void draw(float tempDHT,float wilgDHT,int tempan0,int tempan1,int swiatl,int battlev){
//x y to pozycja lewego dolnego piksela pisanego tekstu
u8g.setFont(u8g_font_6x12);
u8g.setColorIndex(1);

 int x=0,y=7,xd=64; // wpolowie zaczniemy wartosci wiec x bedzie 64 rowne
 float volty=(5.0*battlev)/1024.0;//chce,my miec poziom bateri we woltach a nie w 0-1023

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
u8g.drawStr( xd, y, "czas");

}

void aktualizacja()
{
  actime=millis();
   unsigned long differ=actime-zaptime1;
   //przesuniueta opozniona wzgledem tamtej o 30 sekund
if(warun1==1){
  if(differ<=(czas/4))
  {
          warun1=1;
         
    }else{
       warun1=0;
    
    }
  }
}


