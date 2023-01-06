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
#include "ThingSpeak.h"
#include <ArduinoJson.h>

const char* host = "maker.ifttt.com";
int yil,ay;
const char* auth  = "eXOv51EFYSIT_h7H00ERfSCIdFg27NH8";  // Blynk uygulaması tarafından, mailinize gelen token key    
const char* ssid = "OPPO Reno4";  // Bağlantı yapacağınız Wi‐Fi adı 
const char* pass  = "iremirem12"; 
int saat, dakika ;
int time_blynk,data;
int gun;
int calibration_factor=400;
unsigned long channelID =1996937;             // Thingspeak channel ID 
unsigned int field_no=1; 
const char* writeAPIKey = "HR1NLRPL5DSKU5FY";   // Thingspeak write API Key 
const char* readAPIKey = "WYNZU9A2AJZXQASF";   // Thingspeak read API Key 

time_t local, utc;
WiFiUDP ntpUDP;
HX711 scale;
Servo servo;
NTPClient timeClient(ntpUDP, "tr.pool.ntp.org", NTP_OFFSET, NTP_INTERVAL);
BlynkTimer timer;
WiFiClient client;



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
  
}
}

BLYNK_WRITE(V5){
  time_blynk=param.asInt();
  Serial.print(time_blynk);
}


void setup() {
  Serial.begin(9600);
  scale.begin(D5,D6);
  scale.set_scale(calibration_factor); // this value is obtained by calibrating the scale with known weights

  delay(100);
  servo.attach(5);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  Blynk.begin (auth, ssid, pass, "blynk.cloud", 80);
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  ThingSpeak.begin(client);

  timeClient.begin();
 

 

  
}

void loop() {
  
  
    

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



  

  int final_time = 3600*saat + 60*dakika;
  
  if (time_blynk == final_time){
   
  servo.write(180);
  delay(2000);
  servo.write(0);
  delay(2000);
  
  }
  

  //Serial.println(dakika);
  if (data == 1){
  servo.write(180);
  delay(1000);
  servo.write(0);
  delay(1000);
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



static unsigned int lastMeasurement =0;
if(millis()-lastMeasurement>5000){

lastMeasurement=millis();
float weight = scale.get_units(calibration_factor); // Ağırlığı ölçün
if(weight<0){
  weight=0;
}
if(weight>0){
  weight=(weight-898)*2;
}
int i=1;
Serial.print("Agirlik:");
Serial.println(weight);
if(weight<750 && i==1){

           const int httpPort = 80;  
            if (!client.connect(host, httpPort)) {  
                  Serial.println("connection failed");  
            return;}

  String url = "/trigger/mama_bitti/with/key/pnyzO2VyW8j6W3WkZyg5X3z-UxtQAWsv1e2KmN6RIuS"; 

          
                    Serial.print("Requesting URL: ");
                    Serial.println(url);
                 
                     client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                                    "Host: " + host + "\r\n" +   
                                           "Connection: close\r\n\r\n");
  i=0;
  
}
Blynk.virtualWrite(V3,weight);
ThingSpeak.setField (1, weight);
ThingSpeak.writeField (channelID, field_no, weight, writeAPIKey);





}

timer.run();
Blynk.run();
}//End of the lllloooopppp

void sendIFTTTRequest() {
  // IFTTT isteği oluştur
WiFiClient client; 
           const int httpPort = 80;  
            if (!client.connect(host, httpPort)) {  
                  Serial.println("connection failed");  
            return;}

  String url = "/trigger/mama_bitti/with/key/pnyzO2VyW8j6W3WkZyg5X3z-UxtQAWsv1e2KmN6RIuS"; 

          
                    Serial.print("Requesting URL: ");
                    Serial.println(url);
                 
                     client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                                    "Host: " + host + "\r\n" +   
                                           "Connection: close\r\n\r\n");    
                                    }
