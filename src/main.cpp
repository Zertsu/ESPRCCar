#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "LittleFS.h"


class digitalMotor {
  protected:
    uint8_t pin1;
    uint8_t pin2;
  public:
    void attach(uint8_t p1, uint8_t p2) {
      pin1 = p1;
      pin2 = p2;
      pinMode(pin1, OUTPUT);
      digitalWrite(pin1, 0);
      pinMode(pin2, OUTPUT);
      digitalWrite(pin2, 0);
    }

    void set(bool go, bool direction) {
      if (direction)
      {
        digitalWrite(pin1, go);
        digitalWrite(pin2, 0);
      }
      else
      {
        digitalWrite(pin1, 0);
        digitalWrite(pin2, go);
      }
    }

    void stop() {
      digitalWrite(pin1, 0);
      digitalWrite(pin2, 0);
    }
};

class analogMotor : public digitalMotor {
  public:
    void set(int speed, bool direction) {
      if (direction)
      {
        analogWrite(pin1, speed);
        digitalWrite(pin2, 0);
      }
      else
      {
        digitalWrite(pin1, 0);
        analogWrite(pin2, speed);
      }
    }
};


class myUDP : public WiFiUDP {
  public:
    void sendReply(const char *buffer) {
      beginPacket(remoteIP(), remotePort());
      write(buffer);
      endPacket();
    }
    
    void sendReply(const uint8_t *buffer, size_t len) {
      beginPacket(remoteIP(), remotePort());
      write(buffer, len);
      endPacket();
    }
};



analogMotor LeftM;
analogMotor RightM;
digitalMotor TurnM;

unsigned long lastpacket;
bool isON=false;

const char* ssid = "SSID";
const char* password = "PASS";
const char* Mssid="asdf8743";

myUDP Udp;
unsigned int localUdpPort = 5556;
uint8_t incomingPacket[255];
uint8_t sendBuffer[255];


void setup()
{
  LittleFS.begin();
  LeftM.attach(4, 14);   // D2 D5
  RightM.attach(12, 15); // D6 D8
  TurnM.attach(13, 5);   // D7 D1
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);
  for (size_t i = 0; i < 50; i++)
  {
    delay(100);
    if (WiFi.status() == WL_CONNECTED)
    {
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.softAP("ESPCar");
  }
  Udp.begin(localUdpPort);
}





void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    lastpacket=millis();
    isON=true;
    switch (incomingPacket[0])
    {
      case 0:
        memcpy(sendBuffer, incomingPacket, len);
        sendBuffer[0] = 1;
        Udp.sendReply(sendBuffer, len);
        break;
      
      case 2:
        LeftM.set (incomingPacket[1], incomingPacket[3] & 1 << 0);
        RightM.set(incomingPacket[2], incomingPacket[3] & 1 << 1);
        TurnM.set (incomingPacket[3] & 1 << 3, incomingPacket[3] & 1 << 2);
        break;

      default:
        break;
    }
  
  } else if(lastpacket+1000<millis() and isON) {
    isON=false;
    LeftM.stop();
    RightM.stop();
    TurnM.stop();
  }
}