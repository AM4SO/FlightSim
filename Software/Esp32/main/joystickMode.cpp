#include "IPAddress.h"
#include "AsyncUDP.h"
#include "joystickMode.h"

JoystickProgram::JoystickProgram(){
  //wifi_ssid = NULL;
  //wifi_password = NULL;
}

void JoystickProgram::getNetworkCredentials(){
  wifi_ssid = "AMASO";
  wifi_password = "Chadwick";
}

bool JoystickProgram::connectToNetwork(){
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

void JoystickProgram::pairServer(){
  // keep sending packets to the feeder multicast group
  // Feeder receives and knows our address
  // Feeder keeps sending back packets
  // We receive a packet, then stop sending packets: we know the remote address.
  // We start sending joystick inputs to the feeder
  // Feeder receives inputs, and now stops sending packets.
  // If forcefeedback is available, feeder will send FF to us on a different port.

  //udpListener.listen(25576);
  
  if (udpSender.connect(WiFi.broadcastIP(), 4446)){//IPAddress(224,0,2,60)
    udpSender.onPacket([](AsyncUDPPacket packet){
      Serial.print("Packet Received from ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.println(packet.remotePort());
    });
    while(true){
      udpSender.print("hello?");
      delay(1000);
    }
  }
  
}

void JoystickProgram::setup(){
  Serial.println();
  Serial.println("Joystick program starting...");

  getNetworkCredentials();
  Serial.println("Got network credentials");

  connectToNetwork();

  pairServer();
}

void JoystickProgram::loop(){

}