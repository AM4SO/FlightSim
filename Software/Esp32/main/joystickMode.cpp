#include "Wire.h"
#include "EEPROM.h"
#include "lwipopts.h"
#include "Arduino.h"
#include "assert.h"
#include "esp32-hal.h"
#include "IPAddress.h"
#include "AsyncUDP.h"
#include "joystickMode.h"

#define CLOCK_1 22
#define DATA_1 21
#define CLOCK_2 4//12
#define DATA_2 16//13

JoystickProgram::JoystickProgram(){
  wifi_ssid = NULL;
  wifi_password = NULL;
  pairedWithServer = false;
}

void JoystickProgram::getNetworkCredentials(){
  EEPROM.begin(256);
  int EEPROM_PTR = 0;
  int ssidLen = EEPROM.read(EEPROM_PTR++);
  int passwordLen = EEPROM.read(EEPROM_PTR++);
  
  wifi_ssid;
  uint8_t* temp = (uint8_t*) malloc(ssidLen+1);
  for (int i = 0; i < ssidLen; i++)
    temp[i] = EEPROM.readByte(EEPROM_PTR++);
  temp[ssidLen] = 0;
  wifi_ssid = reinterpret_cast<const char*>(temp); 

  wifi_password;
  uint8_t* temp2 = (uint8_t*) malloc(passwordLen+1);
  for (int i =0 ; i < passwordLen; i++){
    temp2[i] = EEPROM.readByte(EEPROM_PTR++);
  }
  temp2[passwordLen] = 0;
  wifi_password = reinterpret_cast<const char*>(temp2);

  EEPROM.end();
  
  //wifi_ssid = "AMASO";
  //wifi_password = "Chadwick";

  Serial.println("Read wifi credentials from EEPROM: ");
  Serial.print("SSID: ");
  Serial.println(wifi_ssid);
  Serial.println(sizeof(wifi_ssid));
  Serial.print("password: ");
  Serial.println(wifi_password);
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

void JoystickProgram::packetReceived(AsyncUDPPacket packet){

  if (packet.length() > 1)
    return; // No force feedback implemented yet, so ignore for now
  
  if (packet.data()[0] != 1 && packet.data()[0] != 11){
    Serial.print("Bad message: ");
    for (int i = 0; i < sizeof(packet.data()); i++){
      Serial.print(i);
      Serial.print(" ");
    }
    Serial.println();
    return;
  }
  if (packet.data()[0] == 11){
    Serial.print("Ping: ");
    Serial.print(millis() - pingStart);
    Serial.println("ms");
    pingDone = true;
    return;
  }
  if (pairedWithServer)
    return;

  serverAddress = packet.remoteIP();
  serverPort = 4446;
  
  Serial.print("Paired with server: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.println(packet.remotePort());

  pairedWithServer = true;
}

void JoystickProgram::pairServer(){
  // keep sending packets to the feeder multicast group
  // Feeder receives and knows our address
  // Feeder keeps sending back packets
  // We receive a packet, then stop sending packets: we know the remote address.
  // We start sending joystick inputs to the feeder
  // Feeder receives inputs, and now stops sending packets.
  // If forcefeedback is available, feeder will send FF to us on a different port.

  udpListener.listen(25575);
  
  if (udpSender.connect(WiFi.broadcastIP(), 4446)){//IPAddress(224,0,2,60)
    udpListener.onPacket(AuPacketHandlerFunction([&](AsyncUDPPacket packet){
      packetReceived(packet);
    }));

    Serial.println("Starting to multicast send requests...");
    while(!pairedWithServer){
      udpSender.write(0);
      udpSender.flush();
      delay(10);
    }
    Serial.println("Paired with server!");
  }

  if (!udpSender.connect(serverAddress, serverPort)){
    Serial.println("Something went wrong connecting to the server");
  }
}

void JoystickProgram::setup(){
  pingDone = true;

  Serial.println();
  Serial.println("Joystick program starting...");

  getNetworkCredentials();
  Serial.println("Got network credentials");

  connectToNetwork();

  
  Wire.begin(DATA_1, CLOCK_1);//, 50000
  Wire1.begin(DATA_2, CLOCK_2);

  sensor1 = AS5600(&Wire);
  sensor2 = AS5600(&Wire1);

  sensor1.begin(4);
  sensor2.begin(4);

  Serial.print("Sensor 1 connected: ");
  Serial.println(sensor1.isConnected());
  Serial.print("Sensor 2 connected: ");
  Serial.println(sensor2.isConnected());

  if(sensor1.magnetTooStrong())
    Serial.println("Sensor 1 magnet too strong");
  if(sensor1.magnetTooWeak())
    Serial.println("Sensor 1 magnet too weak");
  if(sensor2.magnetTooStrong())
    Serial.println("Sensor 2 magnet too strong");
  if(sensor2.magnetTooWeak())
    Serial.println("Sensor 2 magnet too weak");

  pairServer();

  sensor1.resetCumulativePosition(0);
  sensor2.resetCumulativePosition(0);
}
void JoystickProgram::sendPing(){
  //if (!pingDone) return;

  pingDone = false;
  uint8_t msg[1];
  for (uint8_t i = 0; i < sizeof(msg); i++){
    msg[i] = 10;
  }
  pingStart = millis();
  Serial.println("Sending...");
  udpSender.write(msg, sizeof(msg));
  udpSender.flush();
}

void JoystickProgram::loop(){

  int32_t currentPosition1 = sensor1.getCumulativePosition();
  int32_t currentPosition2 = sensor2.getCumulativePosition();

  
  if (calibrating > 0){
    if (currentPosition1 < minPosition1 && currentPosition1 != 0)
      minPosition1 = currentPosition1;
    if (currentPosition1 > maxPosition1 && currentPosition1 != 0)
      maxPosition1 = currentPosition1;

    if (currentPosition2 < minPosition2 && currentPosition2 != 0)
      minPosition2 = currentPosition2;
    if (currentPosition2 > maxPosition2 && currentPosition2 != 0)
      maxPosition2 = currentPosition2;

    if (calibrating % 100 == 0){
      Serial.print("Calibrating: ");
      Serial.println(calibrating);
    }
    
    //sensor1.resetCumulativePosition(currentPosition1 - minPosition1); //(int32_t)((minPosition1 + maxPosition1)/2));
    //sensor2.resetCumulativePosition(currentPosition2 - minPosition2); //(int32_t)((minPosition2 + maxPosition2)/2));

    /*if (calibrating > 500)
    {
      minPosition1 = currentPosition1;
      minPosition1 = currentPosition2;
      maxPosition1 = currentPosition1;
      maxPosition1 = currentPosition2;
    }*/

    calibrating--;
  } else if (calibrating == 0){
    calibrating--;

    
    if (maxPosition1 == minPosition1)
      maxPosition1 = minPosition1 + 1;
    if (maxPosition2 == minPosition2)
      maxPosition2 = minPosition2 + 1;

    // Desired range / actual range, gives scale factor
    xScale = (double)((32767 - 0)) / (maxPosition1 - minPosition1);
    yScale = (double)((32767 - 0)) / (maxPosition2 - minPosition2);
  }

  if (sensor1.isConnected()) {
    Serial.print("Sensor1: ");
    Serial.println(currentPosition1);
  }
  
  if (sensor2.isConnected()) {
    Serial.print("Sensor2: ");
    Serial.println(currentPosition2);
  }
  //if (currentPosition1 < minPosition1)
  //  sensor1.resetCumulativePosition(minPosition1);
  //if (currentPosition2 < minPosition2)
  //  sensor2.resetCumulativePosition(minPosition2);

  //if (currentPosition1 > maxPosition1)
  //  sensor1.resetCumulativePosition(maxPosition1);
  //if (currentPosition2 > maxPosition2)
  //  sensor2.resetCumulativePosition(maxPosition2);

  if (abs(currentPosition1 - prevPos1) > 1){
    posX = (int16_t) ((max(min(((currentPosition1 - minPosition1) * xScale), 32767.0), 0.0)) );
    prevPos1 = currentPosition1;
  }
  if (abs(currentPosition2 - prevPos2) > 1){
    posY = (int16_t) (32767 - (max(min( ( (currentPosition2 - minPosition2) * yScale) , 32767.0), 0.0)));
    prevPos2 = currentPosition2;
  }

  uint8_t msg[6];
  msg[0] = 2; // axis id = 1
  msg[1] = (posX >> 8); // byte 0
  msg[2] = (posX & 0xff); // byte 1
  msg[3] = 1; // axis id = 2
  msg[4] = (posY >> 8); // byte 0
  msg[5] = (posY & 0xff); // byte 1

  udpSender.write(msg, sizeof(msg));
  udpSender.flush();

  delay(15);
}