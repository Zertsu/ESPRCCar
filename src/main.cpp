#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include "debug.h"

class digitalMotor
{
protected:
  uint8_t pin1;
  uint8_t pin2;

public:
  void attach(uint8_t p1, uint8_t p2)
  {
    pin1 = p1;
    pin2 = p2;
    pinMode(pin1, OUTPUT);
    digitalWrite(pin1, 0);
    pinMode(pin2, OUTPUT);
    digitalWrite(pin2, 0);
  }

  void set(bool go, bool direction)
  {
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

  void stop()
  {
    digitalWrite(pin1, 0);
    digitalWrite(pin2, 0);
  }
};

class analogMotor : public digitalMotor
{
public:
  void set(int speed, bool direction)
  {
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

class myUDP : public WiFiUDP
{
private:
  unsigned int localUdpPort;
  uint8_t incomingPacket[255];
  uint8_t sendBuffer[255];
  bool serverConnected;
  IPAddress serverIP;
  uint16_t serverPort;
  void (*controlCallback)(const uint8_t *);
  void (*timeoutCallback)();
  unsigned long timeoutAt;
  bool timeouted = false;

  int parsePacketSync(unsigned long timeout = 1500)
  {
    unsigned long end = millis() + timeout;
    int res = 0;
    while (millis() < end)
    {
      res = parsePacket();
      if (res > 0)
      {
        return res;
      }
      delay(100);
    }
    return 0;
  }

public:
  void sendReply(const char *buffer)
  {
    beginPacket(remoteIP(), remotePort());
    write(buffer);
    endPacket();
  }

  void sendReply(const uint8_t *buffer, size_t len)
  {
    beginPacket(remoteIP(), remotePort());
    write(buffer, len);
    endPacket();
  }

  void sendTo(const char *buffer, size_t len, IPAddress ip, uint16_t port)
  {
    beginPacket(ip, port);
    write(buffer, len);
    endPacket();
  }

  int setServer(IPAddress ip, uint16_t port)
  {
    serverIP = ip;
    serverPort = port;
    sendTo("\0", 1, serverIP, serverPort);
    DPRINTF("Connecting to: %s:%d ... ", ip.toString().c_str(), port);
    int res = parsePacketSync(5000);
    if (res > 0)
    {
      DPRINTLN("Succes!");
      serverConnected = true;
      return 0;
    }
    DPRINTLN("Fail!");
    serverConnected = false;
    return 1;
  }

  void setCallbacks(void (*con)(const uint8_t *), void (*t)())
  {
    controlCallback = con;
    timeoutCallback = t;
  }

  void run()
  {
    int packetSize = parsePacket();
    if (packetSize)
    {
      int len = read(incomingPacket, 255);
      incomingPacket[10] = 0;
      timeoutAt = millis() + 1500;
      timeouted = false;
      switch (incomingPacket[0])
      {
      case 0:
        memcpy(sendBuffer, incomingPacket, len);
        sendBuffer[0] = 1;
        sendReply(sendBuffer, len);
        break;

      case 2:
        controlCallback(incomingPacket);
        break;

      default:
        break;
      }
    }
    else if (timeoutAt < millis() && !timeouted)
    {
      timeouted = true;
      timeoutCallback();
    }
  }
};

analogMotor LeftM;
analogMotor RightM;
digitalMotor TurnM;
myUDP Udp;

int waitForWiFi(wl_status_t target, unsigned long timeout = 5000)
{
  unsigned long end = millis() + timeout;
  while (WiFi.status() != target)
  {
    delay(1000);
    if (millis() > end)
    {
      return 1;
    }
  }
  return 0;
}

int networkSetup()
{
  if (LittleFS.exists("/config/wifi.csv"))
  {
    File wifilist = LittleFS.open("/config/wifi.csv", "r");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    waitForWiFi(WL_IDLE_STATUS);
    DPRINTLN("Scanning...");
    int nnetworks = WiFi.scanNetworks();

#if defined(DEBUG)

    Serial.printf("%d network(s) found\n", nnetworks);
    for (int i = 0; i < nnetworks; i++)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.channel(i),
                    WiFi.RSSI(i),
                    WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }

#endif // DEBUG

    int end;
    char SSID_buf[33];
    char pass[64];
    while (wifilist.position() + 1 < wifilist.size())
    {
      end = wifilist.readBytesUntil(',', SSID_buf, 32);
      SSID_buf[end] = 0;
      end = wifilist.readBytesUntil('\n', pass, 63);
      pass[end] = 0;
      for (int i = 0; i < nnetworks; i++)
      {
        if (strcmp(WiFi.SSID(i).c_str(), SSID_buf) == 0)
        {
          DPRINT("Connecting to: ");
          DPRINTLN(SSID_buf);
          WiFi.begin(SSID_buf, pass);
          if (!waitForWiFi(WL_CONNECTED, 15000))
          {
            DPRINTLN("Connected!");
            wifilist.close();
            WiFi.scanDelete();
            return 0;
          }
        }
      }
    }
    wifilist.close();
    WiFi.scanDelete();
  }
  DPRINTLN("Setting up AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP-Car");
  return 1;
}

int connectToServer()
{
  if (!LittleFS.exists("/config/udpser.txt"))
  {
    return 2;
  }
  File serfile = LittleFS.open("/config/udpser.txt", "r");
  char domain[256];
  uint16_t port;
  int end = serfile.readBytesUntil(':', domain, 253);
  domain[end] = 0;
  port = serfile.parseInt();
  IPAddress target;
  WiFi.hostByName(domain, target);
  DPRINTF("Resolved: %s to %s\n", domain, target.toString().c_str());
  return Udp.setServer(target, port);
}

void handleControlPacket(const uint8_t *str)
{
  // DPRINTF("%x %x %x\n", str[1], str[2], str[3]);
  LeftM.set(str[1], str[3] & 1 << 0);
  RightM.set(str[2], str[3] & 1 << 1);
  TurnM.set(str[3] & 1 << 3, str[3] & 1 << 2);
}

void stopMotors()
{
  LeftM.stop();
  RightM.stop();
  TurnM.stop();
}

void setup()
{
  #if defined(DEBUG)
    Serial.begin(115200);
  #endif

  analogWriteResolution(8);
  LeftM.attach(4, 14);   // D2 D5
  RightM.attach(12, 15); // D6 D8
  TurnM.attach(13, 5);   // D7 D1
  pinMode(LED_BUILTIN, OUTPUT);
  Udp.begin(5556);
  LittleFS.begin();
  if (!networkSetup())
  {
    connectToServer();
  }
  else
  {

  }
  Udp.setCallbacks(handleControlPacket, stopMotors);
}

void loop()
{
  Udp.run();
}