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

  WiFi.begin(wifi_ssid, wifi_password);

  return true;
}

void JoystickProgram::setup(){
  getNetworkCredentials();

  connectToNetwork();
}

void JoystickProgram::loop(){

}