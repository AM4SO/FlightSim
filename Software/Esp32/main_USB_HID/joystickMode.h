#include <AsyncUDP.h>
#include "AS5600.h"

#include "esp_hidd_api.h"
#include "USB.h"
#include "USBHID.h"

//#endif

USBHID HID;

class JoystickHIDDevice{
  
  static const uint8_t report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xa1, 0x01,        // Collection (Application)
    0xa1, 0x00,        //   Collection (Physical)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x00, 0x00,        //     Logical Minimum (0)
    0x26, 0x7f, 0xff,        //     Logical Maximum (0111_1111 11111111 = 2^15 - 1)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x02,        //     Report Count (8)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
  };

  public:
  JoystickHIDDevice(void){
    static bool initialized = false;
    if(!initialized){
      initialized = true;
      HID.addDevice(this, sizeof(report_descriptor));
    }
  }

  void begin(void){
    HID.begin();
  }
    
  uint16_t _onGetDescriptor(uint8_t* buffer){
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
  }

  bool send_X(uint16_t * value){
    return HID.SendReport(0, value, 16);
  }
  bool send_Y(uint16_t * value){
    return HID.SendReport(1, value, 16);
  }
}

class JoystickProgram{
  JoystickHIDDevice hidDevice;
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


  void packetReceived(AsyncUDPPacket packet);

  public:

  JoystickProgram();
  void setup();
  void loop();
};