
#include <SPI.h>
#include <SD.h>

#include "DHT.h"

#define dhtp 2
#define dhtyp DHT22
#define sdpin 10

File plikdane;

unsigned long zaptime1=0;
unsigned long actime=0;
unsigned long czas=60000;//60 sek odstepu
char nazwa[]="dane.txt";


float wilg=0,temp=0;//na to referencje beda przekazywac sie wartosci w funkcjach
bool warunek=0;
int powtarzalnosc=1;
DHT dht(dhtp,dhtyp);

//funkcja mierzenie dostaje referencje na parametry z funkcji odczyt 
void zapis(int a);
void mierzenie(unsigned long odstep,float &wilgmierz,float &tempmierz,bool &warun);//bedzie po prostu mierzenie(czas); i to dostanie oryginal do zmiany wartosci żeby nie tworzyc nowej zmiennejo pamiec chodzi
void wyswietlanie(float &wilgodcz,float &tempodcz,bool &warun);
void logger();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  dht.begin();
  Serial.println("gotowy");
  Serial.println("szukam karte");

while(SD.begin(sdpin)==0){
  
    Serial.println("nie wykryto karty");
    delay (500);
    //nie zacznie programu az nie znajdzie karty
}

  Serial.println("karta wykryta, zaczynam");
 if(SD.exists(nazwa))
  {//jesli istniej e poprzedni plik to przy uruchamianiu usuwa go
    //zeby nie bylo problemu z osia czasu
  SD.remove("dane.txt");
  }
 
  plikdane=SD.open(nazwa,FILE_WRITE);
  Serial.println("utworzono plik dane.csv");

}

void loop() {
  // make a string for assembling the data to log:
  mierzenie(czas,wilg,temp,warunek);
  wyswietlanie(wilg,temp,warunek);
//  Serial.println(wilgmierz);
 // Serial.println(tempmierz);
  
}



void mierzenie(unsigned long odstep,float &wilgmierz,float &tempmierz,bool &warun)
{
  actime=millis();
 unsigned long differ=actime-zaptime1;

  if(differ>=odstep)
  {
    Serial.println(differ);
wilgmierz=dht.readHumidity();
tempmierz=dht.readTemperature();
    if(isnan(tempmierz)||isnan(wilgmierz))
    {
      Serial.println("cos nie tak poszlo i odczyt bledny");
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
  Serial.println(warunek1);
  Serial.println(zaptime1);
  logger(wilgodcz,tempodcz);
 }
}

//trzeba stworzyc 2 funkcje
//1sza przed calascia funkcji i sprawdza differ i jak ok to warun na 1 czy jest ok
//za nia wszystkie funkcje co maja sie wykonywac co differ wewnatrz ich if(1) to dopiero sie wykonuja
//na koncu 2ga funkcja ktora zmienia warunek na 0 jesli differ obliczony w pierwszej jest zgodny z odstepem
//2ga sprawdzajaca dostaje differ w referencji ?
void logger(float &wilgotnosc,float &temperatura)
{
  plikdane=SD.open("dane.txt",FILE_WRITE);
  plikdane.print(zaptime1);
  plikdane.print(";");
  plikdane.print(wilgotnosc);
   plikdane.print(";");
  plikdane.print(temperatura);
   plikdane.println();
  plikdane.close();
  Serial.println("zapisano czas i danex2!");
}

