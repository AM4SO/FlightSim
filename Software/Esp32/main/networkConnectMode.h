#include <WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <esp_wifi.h>
#include "StreamString.h"
#include "WString.h"

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED


class NetworkConnectProgram{
  const char* ssid;
  const char* password;
  const char* webPageP0;
  const char* webPageP1;
  String networkListString;

  WiFiClass wifiHandler;

  WiFiServer* server;

  String readRequestLine(WiFiClient* client);
  void doPost(WiFiClient* client, int bodyLength);
  void doNetScan();

  public:
    NetworkConnectProgram();
    void setup();
    void loop();
};