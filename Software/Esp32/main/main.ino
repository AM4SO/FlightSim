#include "networkConnectMode.h"
#include "joystickMode.h"

NetworkConnectProgram* networkConnectProgram = 0;
JoystickProgram* joystickProgram = 0;

bool hasNetworkDetails;

void setup() {
  EEPROM.begin(512);
  Serial.begin(921600);

  /*
  int len = EEPROM.length();
  for (int i = 0; i < len; i++){
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  */
  
  hasNetworkDetails = EEPROM.read(0) == 0; //TODO: CHANGE THIS BACK TO x > 0
  if (!hasNetworkDetails){
    networkConnectProgram = new NetworkConnectProgram();
    networkConnectProgram->setup();
  } else {
    joystickProgram = new JoystickProgram();
    joystickProgram->setup();
  }
}

void loop() {
  if (!hasNetworkDetails){
    networkConnectProgram->loop();
  } else {
    joystickProgram->loop();
  }
}