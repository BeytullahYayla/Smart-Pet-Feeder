#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include "Timezone.h"
#include <Servo.h>
#include <HX711.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define USE_NODE_MCU_BOARD
#define motor D1
#define NTP_OFFSET 60*60
#define NTP_INTERVAL 300*1000
#include <String.h>
#include <FirebaseESP8266.h>

int yil,ay;
const int DOUT_PIN = 12; // HX711 DOUT pini (veri çıkışı) NodeMCU GPIO12 piniyle bağlanır
const int CLK_PIN = 13; // HX711 CLK pini (saat çıkışı) NodeMCU GPIO13 piniyle bağlanır
char auth [] = "eXOv51EFYSIT_h7H00ERfSCIdFg27NH8";  // Blynk uygulaması tarafından, mailinize gelen token key    
char ssid [] = "Beytullah";  // Bağlantı yapacağınız Wi‐Fi adı 
char pass [] = "11677840"; 
unsigned long last_second;
int saat, dakika ;
int time_blynk,data;
int gun;
int calibration_factor=807;


time_t local, utc;
WiFiUDP ntpUDP;
HX711 scale;
Servo servo;
NTPClient timeClient(ntpUDP, "tr.pool.ntp.org", NTP_OFFSET, NTP_INTERVAL);
BlynkTimer timer;
BLYNK_WRITE(V0){
   int s0 = param.asInt();
   servo.write(s0);
   Blynk.virtualWrite(V1, s0);
}


BLYNK_WRITE(V1){
data=param.asInt();
if(data==1){
  Serial.print("Motor calisiyor");
  servo.write(180);
  delay(1000);
  servo.write(0);
  delay(1000);
  servo.write(180);
  delay(1000);
  servo.write(0);
  delay(1000);
  servo.write(180);
  delay(1000);
  servo.write(0);
  delay(1000);
  servo.write(0);
  delay(1000);
}
}

BLYNK_WRITE(V5){
  time_blynk=param.asInt();
  Serial.print(time_blynk);
}


void setup() {
  Serial.begin(9600);
  scale.begin(DOUT_PIN, CLK_PIN); // HX711 modülünü başlatın
  scale.set_scale(calibration_factor); // HX711 modülünü ölçeklendirin
  scale.tare(); // HX711 modülünü tarama işlemini yapın
  delay(100);
  servo.attach(5);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  Blynk.begin (auth, ssid, pass, "blynk.cloud", 80);
  WiFi.begin(ssid, pass);
  last_second = millis();
  
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  if (millis() - last_second > 1000)
  {
    last_second = millis();

    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    // convert received time stamp to time_t object

    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = { "EDT", Second, Sun, Mar, 2, +120 };  //Eastern Daylight Time (EDT)... Türkiye: sabit +UTC+3 nedeni ile  +2saat = +120dk ayarlanmalı
    TimeChangeRule usEST = { "EST", First, Sun, Nov, 2, +120  };  //Eastern Time Zone: UTC - 6 hours - change this as needed//Türkiye için değil...
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

    //Aşağıda gün sat ve yılı bu şekilde alabiliriz
    yil=year(local);
    ay=month(local);
    gun=day(local);
    saat = hour(local); //Eğer 24 değil de 12 li saat istenirse :hourFormat12(local); olmalı
    dakika = minute(local);
    
    

  }// End of the millis...
  
  //Serial.print(hours);
  //Serial.print(":");
  
 // Serial.println(timeClient.getMinutes()); 
  int final_time = 3600*saat + 60*dakika;
  Serial.println(final_time);
  if (time_blynk == final_time){
   Serial.print("Timer'da calisiyor");
  servo.write(180);
  delay(2000);
  servo.write(0);
  delay(2000);
  servo.write(180);
  delay(2000);
  servo.write(0);
  delay(2000);
  servo.write(180);
  delay(2000);
  servo.write(0);
  
  delay(2000);
  servo.write(180);
  delay(2000);
  }
  Serial.println(gun);
  
  //Serial.println(dakika);
  if (data == 1){
  digitalWrite(motor, HIGH);
  delay(5000);
  digitalWrite(motor,LOW);
  Serial.print("Opening");
  String strSaat=String(saat);
String strDakika=String(dakika);
String zaman=String("Zaman:"+strSaat+":"+strDakika);
String strGun=String(gun);
String strAy=String(ay);
String strYil=String(yil);
String date=String("Tarih:"+strGun+"."+strAy+"."+strYil);
Blynk.virtualWrite(V2,date);
Blynk.virtualWrite(V4,zaman);
  
} 




float weight = scale.get_units(10); // Ağırlığı ölçün
Serial.println(weight); // Ölçülen ağırlığı seri porta yazdırın
delay(1000);
Blynk.virtualWrite(V3,weight);


Blynk.run();
timer.run();
  
}//End of the lllloooopppp
