////#ifndef <WiFi.h>//
#include <WiFi.h>
#include <WiFiClient.h>
#include "WiFiType.h"
#include <AsyncUDP.h>
//#endif

class JoystickProgram {
  const char* wifi_ssid;
  const char* wifi_password;
  int wifi_channel;
  AsyncUDP udpListener;
  AsyncUDP udpSender;

  void getNetworkCredentials();
  bool connectToNetwork();
  void pairServer();

  public:

  JoystickProgram();
  void setup();
  void loop();
};