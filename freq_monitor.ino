#include <ESP8266WiFi.h>
#include "Arduino.h"      
#define MainPeriod 5000

long previousMillis = 0; 
volatile unsigned long duration=0; 
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
float Freq = 60;

void ICACHE_RAM_ATTR myinthandler ();

class comm {
  public:
  comm()
  {

  } 
  
  const char* ssid     = "errans";
  const char* password = "zamb0rah";
  const char* host = "192.168.1.143";
  const int httpPort = 80;
  int failCount = 0;
  bool amiconnected = 0;
  bool started = 0;
  
  bool connect() {
    if (started == 0) {
      Serial.print("First start. Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      unsigned long connectMillis = millis();
       while (WiFi.status() != WL_CONNECTED) {
        if (connectMillis + 30000 <= millis()) {
          //we failed--reset and try again next time.
          WiFi.disconnect();
          amiconnected = 0;
          Serial.println("failed during start");
          return 1;
        };
        delay(500);
        Serial.print(".");
      };
      
      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      amiconnected = 1;
      Serial.print("End start. amiconnected=");
      Serial.println(amiconnected);
      return 0;
    } else {
      Serial.print("begin started == 1. started is ");
      Serial.println(started);
      if (amiconnected == 0) {
        Serial.println(amiconnected);
        Serial.println("reconnecting");
        unsigned long connectMillis = millis();
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED) {
          if (connectMillis + 30000 <= millis()) {
            //we failed--reset and try again next time.
            WiFi.disconnect();
            amiconnected = 0;
            return 1;
          };
          delay(500);
          Serial.print(".");
        };
        
        Serial.println("");
        Serial.println("WiFi connected");  
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        amiconnected = 1;
        return 0;
      };
    };
    started = 1;
  };

  bool push() {
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
      Serial.println("inside no client connect");
      failCount++;
      Serial.println(failCount);
      delay(1000);
      amiconnected = 0;
      connect();
      failCount = 0;
      return 1;
      Serial.println(failCount);
      return 1;
    };

    String url = "/api.php?device_api_pushing=true&freq=";
    url = url + Freq;
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    bool available = 0;
    unsigned int timer = millis();
    while(available == 0) {
      if(client.available() || (millis() >= timer + 15000)) {
        available = 1;

      };
    };
    return 0;
  };
};

comm cc = comm();

void setup(){
  Serial.begin(9600);
  delay(100);
  cc.connect();
  attachInterrupt(digitalPinToInterrupt(14), myinthandler, RISING);
  Serial.println("end setup.");
}
 
void loop(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MainPeriod){
    detachInterrupt(digitalPinToInterrupt(14));
    previousMillis = currentMillis;   
    // need to bufferize to avoid glitches
    unsigned long _duration = duration;
    unsigned long _pulsecount = pulsecount;
    duration = 0; 
    pulsecount = 0;
    Freq = 1e6 / float(_duration);    
    Freq *= _pulsecount;
    Serial.println(Freq);
    cc.push();
    attachInterrupt(digitalPinToInterrupt(14), myinthandler, RISING);
  }  
}

void myinthandler(){
  unsigned long currentMicros = micros();
  duration += currentMicros - previousMicros;
  previousMicros = currentMicros;
  pulsecount++;
}
