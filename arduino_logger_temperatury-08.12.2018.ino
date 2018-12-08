
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "DHT.h"
#include <Time.h>

#include "U8glib.h"

#define dhtp 2
#define dhtyp DHT22
#define sdpin 10

File plikdane;

unsigned long zaptime1=0;
unsigned long actime=0;
unsigned long czas=60000;//60 sek odstepu



float wilg=0,temp=0;//na to referencje beda przekazywac sie wartosci w funkcjach
bool warunek=0;
int powtarzalnosc=1;
int odcz0=0,odcz1=0,odcz2=0;
DHT dht(dhtp,dhtyp);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 

//funkcja mierzenie dostaje referencje na parametry z funkcji odczyt 
void zapis(int a);
void mierzenie(unsigned long odstep,float &wilgmierz,float &tempmierz,bool &warun);//bedzie po prostu mierzenie(czas); i to dostanie oryginal do zmiany wartosci żeby nie tworzyc nowej zmiennejo pamiec chodzi
void wyswietlanie(float &wilgodcz,float &tempodcz,bool &warun);
void logger();
void odczytywanie();
void draw();
//void oledshow(int &odcz0,int &odcz1,int &odcz2);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  dht.begin();
 // display.begin(SSD1306_SWITCHCAPVCC,0x3C);
//  display.clearDisplay();
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  Serial.println("gotowy");
  Serial.println("szukam karte");
   pinMode(8, OUTPUT);
 
  u8g.setFont(u8g_font_unifont);

 
while(SD.begin(sdpin)==0){
  
    Serial.println("nie wykryto karty");
    u8g.firstPage();
do {
u8g.drawStr( 10, 10, "nie wykryto");
} while( u8g.nextPage() );
     
    delay (500);
    //nie zacznie programu az nie znajdzie karty
}

  Serial.println("karta wykryta, zaczynam");
      u8g.firstPage();
do {
u8g.drawStr( 10, 32, "karta wykryta, zaczynam");
} while( u8g.nextPage() );
  
 /*if(SD.exists(nazwa))
  {//jesli istniej e poprzedni plik to przy uruchamianiu usuwa go
    //zeby nie bylo problemu z osia czasu
  SD.remove("dane.txt");
  }*/
  
 bool istnienie=0;
 if(SD.exists("dane.txt")) istnienie=1;
 
  
if(istnienie==1) {
   plikdane=SD.open("dane.txt",FILE_WRITE);
 // Serial.println("reset zasilania byl");
  plikdane.print("PRZERWA");
  plikdane.println();
  plikdane.close();
}
  
//  Serial.println("utworzono plik dane.csv");

}

void loop() {
  // make a string for assembling the data to log:
  mierzenie(czas,wilg,temp,warunek);
  wyswietlanie(wilg,temp,warunek);
 // oledshow();
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
  odczytywanie(odcz0,odcz1,odcz2);
  logger(wilgodcz,tempodcz,odcz0,odcz1,odcz2);
  //odczytywanie();
 }
}

//trzeba stworzyc 2 funkcje
//1sza przed calascia funkcji i sprawdza differ i jak ok to warun na 1 czy jest ok
//za nia wszystkie funkcje co maja sie wykonywac co differ wewnatrz ich if(1) to dopiero sie wykonuja
//na koncu 2ga funkcja ktora zmienia warunek na 0 jesli differ obliczony w pierwszej jest zgodny z odstepem
//2ga sprawdzajaca dostaje differ w referencji ?
void logger(float &wilgotnosc,float &temperatura,int &odczy0, int &odczy1,int &odczy2)
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
   plikdane.println();
  plikdane.close();
 // Serial.println("zapisano czas i danex2!");
 // Serial.println(odczy0);
 // Serial.println(odczy1);
 // Serial.println(odczy2);
}

void odczytywanie(int &odczyt0,int &odczyt1,int &odczyt2)
{
   odczyt0=analogRead(A0);//termistor0
   odczyt1=analogRead(A1);//termistor1
   odczyt2=analogRead(A2);//fotorezystor
 // Serial.println(odczyt0);
  //Serial.println(odczyt1);
 // Serial.println(odczyt2);
  
}


void draw(int what)
{
  switch(what){
    case 1:u8g.drawStr( 0, 32, "karta niewykryta");
    case 2:u8g.drawStr( 0, 32, "karta wykryta");
  }
}

