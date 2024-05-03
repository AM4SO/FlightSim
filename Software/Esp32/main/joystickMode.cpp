#include "WiFiType.h"
#include "joystickMode.h"

JoystickProgram::JoystickProgram(){
  //wifi_ssid = NULL;
  //wifi_password = NULL;
}

void JoystickProgram::getNetworkCredentials(){
  wifi_ssid = "AMASO";
  wifi_password = "Chadwick";
}

void JoystickProgram::scanFullNetConfig(){
  WiFi.scanNetworks(wifi_ssid);
  int numNets = WiFi.scanComplete();
  if (numNets == -2 || numNets == 0){
    Serial.print("Scan failed...:    ");
    Serial.println(numNets);
    return;
  }

  wifi_channel = WiFi.channel(0);

  WiFi.scanDelete();
}

bool JoystickProgram::connectToNetwork(){
  //scanFullNetConfig();

  Serial.println("Connecting to wifi...");

  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.waitForConnectResult();
  wl_status_t status = WiFi.status();

  switch (status){
    case WL_CONNECTED:  
      Serial.print("WiFi connected to: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP().toString());
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Failed to connect to network");
      break;
    default:
      Serial.println("Something went wrong");
      Serial.println(WiFi.SSID());
    }

  return true;
}

void JoystickProgram::setup(){
  Serial.println();
  Serial.println("Joystick program starting...");

  getNetworkCredentials();
  Serial.println("Got network credentials");

  connectToNetwork();
}

void JoystickProgram::loop(){

}