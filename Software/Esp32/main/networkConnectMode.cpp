#include "networkConnectMode.h"

NetworkConnectProgram::NetworkConnectProgram(){
  ssid = "JoystickPrototype";
  password = "Joystick";
  webPageP0 = "<!DOCTYPE html><html><head><style>*{margin-top: 1rem;}input{width: 20rem;max-width: 100%;}</style></head><body><script>function submitNetwork(){var ssid = document.getElementById('ssid').value;var password = document.getElementById('password').value;sendString(ssid+'\\n'+password);}function sendString(string){const xhr = new XMLHttpRequest();xhr.open('POST', '/');const body = string;xhr.onload = () => {if (xhr.readyState == 4 && xhr.status == 201) {console.log('Success!');} else {console.log(`Error: ${xhr.status}`);}};xhr.send(body);}</script><section style='margin-left: auto; box-sizing: border-box; padding: 3rem; margin-top: 5rem; border-width: 2px; outline-style: solid; margin-right: auto; width:70dvw; height:80dvh; border-radius: 10px; background-color: azure;'><a href='/'><button>refresh network list</button></a><br><div style='width:80%; outline-style: solid; height:40%; background-color: white;'>";
  webPageP1 = "</div><input id='ssid' placeholder='ssid' type='text'><br><input id='password' placeholder='password (leave blank if open)' type='password'><br><input type='button' value='Submit' onclick='submitNetwork()'></section></body></html>";
  networkListString = "";
  server = new WiFiServer(80);
}


String NetworkConnectProgram::readRequestLine(WiFiClient* client){
  String ret = "";
  char c;
  while (client->connected() && client->available()){
    c = (char) client->read();
    //Serial.write(c);
    
    if (c == '\r') continue;

    if (c == '\n') return ret;

    ret += c;
  }
  return ret;
}

///// Handle post request: Received ssid and password /////
void NetworkConnectProgram::doPost(WiFiClient* client, int bodyLength){
  String ssid = "";
  String password = "";

  String* writingTo = &ssid;

  ///////////// Read SSID and password from body //////////////
  for (int i = 0; i < bodyLength; i++){
    char c = (char) client->read();

    if (c == '\n') {
      writingTo = &password;
    } else {
      *writingTo += c;
    }
  }
  Serial.println("Received ssid: " + ssid);
  Serial.println("Received password: " + password);

  ////////////////// Respond to the request ////////////////////
  client->write("HTTP/1.1 201 OK\n");
  client->write("Content-type:text\n");
  client->write("\n");
  /// close the connection ///
  client->stop();
  //////////////////////////////////////////////////////////////

  if (ssid.length() == 0 || ssid.length() > 32)
    return; // Invalid ssid received

  ////////////////// Write ssid and password to EEPROM ////////////////
  EEPROM.write(0, (uint8_t) ssid.length()); // Write ssid length
  EEPROM.write(1, (uint8_t) password.length()); /// Write password length

  int ssidAddress = 2;
  int passwordAddress = ssidAddress + EEPROM.writeString(ssidAddress, ssid);
  EEPROM.writeString(passwordAddress, password);

  //EEPROM.commit();

  ESP.restart();
}

void NetworkConnectProgram::setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println("Configuring access point...");

  wifiHandler.scanNetworks();
  doNetScan();

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid)) {
    log_e("Soft AP creation failed.");
    while(1);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server->begin();

  Serial.println("Server started");

  //int n = WiFi.scanNetworks(true);
  
}

void NetworkConnectProgram::doNetScan(){
  int numResults = WiFi.scanComplete();

  //////////// If failed or incomplete, return early ///////
  if (numResults == -1)
    return;
  else if (numResults == -2){
    Serial.println("Scan failed");
    return;
  }


  if (numResults == 0) {
    Serial.println("no networks found");
    return;
  }

  Serial.print(numResults);
  Serial.println(" networks found");
  Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
  for (int i = 0; i < numResults; ++i) {
    // Print SSID and RSSI for each network found


    String SSID = WiFi.SSID(i);

    Serial.printf("%2d",i + 1);
    Serial.print(" | ");
    Serial.printf("%-32.32s", SSID.c_str());
    Serial.print(" | ");
    Serial.printf("%4d", WiFi.RSSI(i));
    Serial.print(" | ");
    Serial.printf("%2d", WiFi.channel(i));
    Serial.print(" | ");
    String encryptionType = "";
    switch (WiFi.encryptionType(i))
    {
    case WIFI_AUTH_OPEN:
        encryptionType = "open";
        break;
    case WIFI_AUTH_WEP:
        encryptionType = "WEP";
        break;
    case WIFI_AUTH_WPA_PSK:
        encryptionType = "WPA";
        break;
    case WIFI_AUTH_WPA2_PSK:
        encryptionType = "WPA2";
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        encryptionType = "WPA+WPA2";
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        encryptionType = "WPA2-EAP";
        break;
    case WIFI_AUTH_WPA3_PSK:
        encryptionType = "WPA3";
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        encryptionType = "WPA2+WPA3";
        break;
    case WIFI_AUTH_WAPI_PSK:
        encryptionType = "WAPI";
        break;
    default:
        encryptionType = "unknown";
    }
    Serial.println(encryptionType);

    networkListString += "<p>";
    networkListString += SSID;
    networkListString += "       |       ";
    networkListString += encryptionType;
    networkListString += "</p>";
  }
  Serial.println("");
  

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  //wifiHandler.scanNetworks(true);
}

void NetworkConnectProgram::loop() {
  //doNetScan();

  WiFiClient client = server->available();   // listen for incoming clients

  if ((int) client == 0) return;
  
  int bodyLength = 0;

  ////////// Read first line of request ////////////
  String firstLine = readRequestLine(&client);
  Serial.println(firstLine);
  firstLine.toLowerCase();

  /////////////////////////// Skip through header lines ////////////////////////////
  String line = "textToStop0Length";
  while (client.connected() && client.available() && line.length() != 0){
    line = readRequestLine(&client);
    line.toLowerCase();

    //////////// store content length header in case of POST request //////////
    if (line.startsWith("content-length:")){
      bodyLength = std::stoi(line.substring(16).c_str());
    }
    ///////////////////////////////////////////////////////////////////////////
  }
  //////////////////////////////////////////////////////////////////////////////////

  if (firstLine.startsWith("post")){  ////////// If post /////////
    doPost(&client, bodyLength);
  } else { //////////////// else send default response as webpage //////////
    client.write("HTTP/1.1 200 OK\n");
    client.write("Content-type:text/html\n");
    client.write("\n");

    client.write(webPageP0);
    client.write(networkListString.c_str());
    client.write(webPageP1);

    // close the connection:
    client.stop();
  }
}