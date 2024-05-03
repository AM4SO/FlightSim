////#ifndef <WiFi.h>//
#include <WiFi.h>
#include <WiFiClient.h>
//#endif

class JoystickProgram {
  const char* wifi_ssid;
  const char* wifi_password;
  int wifi_channel;

  void getNetworkCredentials();
  bool connectToNetwork();
  void scanFullNetConfig();

  public:

  JoystickProgram();
  void setup();
  void loop();
};