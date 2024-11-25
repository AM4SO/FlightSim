#include "IPAddress.h"
////#ifndef <WiFi.h>//
#include <WiFi.h>
#include <WiFiClient.h>
#include "WiFiType.h"
#include <AsyncUDP.h>
#include <EEPROM.h>
#include "AS5600.h"

#include "esp_hidd_api.h"

//#endif

class JoystickProgram {
  const char* wifi_ssid;
  const char* wifi_password;
  int wifi_channel;
  AsyncUDP udpListener;
  AsyncUDP udpSender;
  bool pairedWithServer;
  IPAddress serverAddress;
  int serverPort;
  unsigned long pingStart;
  bool pingDone;
  AS5600 sensor1;
  AS5600 sensor2;
  int minPosition1 = 0;
  int maxPosition1 = 0;
  int minPosition2 = 0;
  int maxPosition2 = 0;
  int calibrating = 1000;
  double xScale = 1;
  double yScale = 1;
  int16_t prevPos1 = 0;
  int16_t prevPos2 = 0;
  int16_t posX = 0;
  int16_t posY = 0;

  void getNetworkCredentials();
  bool connectToNetwork();
  void pairServer();
  void packetReceived(AsyncUDPPacket packet);
  void sendPing();

  public:

  JoystickProgram();
  void setup();
  void loop();
};