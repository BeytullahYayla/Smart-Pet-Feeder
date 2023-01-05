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
#include <ESP_Mail_Client.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* host = "maker.ifttt.com";
int yil,ay;
const int DOUT_PIN = D6; // HX711 DOUT pini (veri çıkışı) NodeMCU GPIO12 piniyle bağlanır
const int CLK_PIN = D5; // HX711 CLK pini (saat çıkışı) NodeMCU GPIO13 piniyle bağlanır
const char* auth  = "eXOv51EFYSIT_h7H00ERfSCIdFg27NH8";  // Blynk uygulaması tarafından, mailinize gelen token key    
const char* ssid = "AndroidAP";  // Bağlantı yapacağınız Wi‐Fi adı 
const char* pass  = "iremirem12"; 
unsigned long last_second;
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
  Serial.begin(115200);
  //ThingSpeak.begin(client);
  scale.begin(D5,D6);
  scale.set_scale(calibration_factor); // this value is obtained by calibrating the scale with known weights
  scale.tare(); // reset the scale to 0
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

  int final_time = 3600*saat + 60*dakika;
  
  if (time_blynk == final_time){
   
  servo.write(180);
  delay(2000);
  servo.write(0);
  delay(2000);
  
  }
  Serial.println(gun);

  //Serial.println(dakika);
  if (data == 1){
  servo.write(180);
  delay(1000);
servo.write(0);
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




float weight = scale.get_units(calibration_factor); // Ağırlığı ölçün
if(weight<0){
  weight=0;
}
int i=0;
if(weight>0 && i<3){

  
  sendIFTTTRequest();
  i++;
  
}

Serial.println(weight); // Ölçülen ağırlığı seri porta yazdırın//
Blynk.virtualWrite(V3,weight);
/*
ThingSpeak.setField (1, weight);       // 1 nolu field ı kur 
ThingSpeak.writeFields(channelID, writeAPIKey);  
*/

Blynk.run();
timer.run();

}//End of the lllloooopppp

void sendIFTTTRequest() {
  // IFTTT isteği oluştur
WiFiClient client; 
           const int httpPort = 80;  
            if (!client.connect(host, httpPort)) {  
                  Serial.println("connection failed");  
            return;}
/*
HTTPClient http;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 60;
DynamicJsonDocument doc(capacity);
  http.begin(client,"https://maker.ifttt.com/trigger/mama_bitti/with/key/pnyzO2VyW8j6W3WkZyg5X3z-UxtQAWsv1e2KmN6RIuS");
  http.addHeader("Content-Type", "application/json");

  // İstek verilerini oluştur
  
  doc["value1"] = 0;
  doc["value2"] = "gram";
  doc["value3"]="";

  // İstek verilerini JSON'a dönüştür
  String json;
  serializeJson(doc, json);

  // İstek gönder
  int httpCode = http.POST(json);
  Serial.println(httpCode); // İstek cevabını yazdır

  http.end(); // İstek sonlandır
  */
  String url = "/trigger/mama_bitti/with/key/pnyzO2VyW8j6W3WkZyg5X3z-UxtQAWsv1e2KmN6RIuS"; 

          
                    Serial.print("Requesting URL: ");
                    Serial.println(url);
                 
                     client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                                    "Host: " + host + "\r\n" +   
                                           "Connection: close\r\n\r\n");    
                                    }
