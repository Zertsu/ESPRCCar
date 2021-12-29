#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

int LM1=4;	//D2
int LM2=14;	//D5
int RM1=12;	//D6
int RM2=15;	//D8
int SP1=13;	//D7
int SP2=5;	//D1


long lastpacket;
bool isON=false;

const char* ssid = "SSID";
const char* password = "PASS";
const char* Mssid="asdf8743";

WiFiUDP Udp;
unsigned int localUdpPort = 5556;
char incomingPacket[255];




void setMot(int LP,int RP,int val) {
  if(val>0) {
    analogWrite(RP,0);
    analogWrite(LP,val);
  } else if(val<0) {
    analogWrite(LP,0);
    analogWrite(RP,abs(val));
  } else {
    analogWrite(LP,0);
    analogWrite(RP,0);
  }
}



void zeroout() {
    setMot(LM1,LM2,0);
    setMot(RM1,RM2,0);
    digitalWrite(SP1,LOW);
    digitalWrite(SP2,LOW);
}


void demo(int F1,int F2, int F3) {
	zeroout();
	analogWrite(LM1,F1);
	analogWrite(RM1,F1);
	delay(333);
	analogWrite(LM1,F2);
	analogWrite(RM1,F2);
	delay(333);
	analogWrite(LM1,F3);
	analogWrite(RM1,F3);
	delay(333);
	zeroout();
}


void setup()
{
  analogWriteRange(255);
  pinMode(LM1,OUTPUT);
  pinMode(LM2,OUTPUT);
  pinMode(RM1,OUTPUT);
  pinMode(RM2,OUTPUT);
  pinMode(SP1,OUTPUT);
  pinMode(SP2,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,0);
  demo(20,30,20);
  //Serial.begin(115200);
  //Serial.println();
  
  //Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  WiFi.softAP(Mssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    //Serial.print(".");
  }
  digitalWrite(LED_BUILTIN,1);
  demo(20,40,0);
  //Serial.println(" connected");

  Udp.begin(localUdpPort);
  //Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}





void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize) {
  int len = Udp.read(incomingPacket, 255);
  if (len > 0)
    {
      incomingPacket[len] = 0;
    }
  int RM=-255;
  int LM=-255;
  int SM=-1;
  LM=LM+(incomingPacket[0]-48)*100;
  LM=LM+(incomingPacket[1]-48)*10;
  LM=LM+(incomingPacket[2]-48);
  RM=RM+(incomingPacket[3]-48)*100;
  RM=RM+(incomingPacket[4]-48)*10;
  RM=RM+(incomingPacket[5]-48);
  SM=SM+(incomingPacket[6]-48);
  lastpacket=millis();
  isON=true;
  
  //Serial.println();
  //Serial.println(LM);
  //Serial.println(RM);
  //Serial.println(SM);
  setMot(LM1,LM2,LM);
  setMot(RM1,RM2,RM);
  digitalWrite(SP1,constrain(SM,0,1));
  digitalWrite(SP2,constrain(-SM,0,1));
  
  } else if(lastpacket+1000<millis() and isON) {
    isON=false;
    zeroout();
  }
}