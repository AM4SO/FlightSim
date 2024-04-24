/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED

// Set these to your desired credentials.
const char *ssid = "Joystick";
const char *password = "Joystick";

const char *webPageP0 = "<style>*{margin-top: 1rem;}input{width: 20rem;max-width: 100%;}</style><body><section style='margin-left: auto; box-sizing: border-box; padding: 3rem; margin-top: 5rem; border-width: 2px; outline-style: solid; margin-right: auto; width:70dvw; height:80dvh; border-radius: 10px; background-color: azure;'><a href='/refresh'><button>refresh network list</button></a><br><div style='width:80%; outline-style: solid; height:40%; background-color: white;'>";
const char *webPageP1 = "</div><input id='ssid' placeholder='ssid' type='text'><br><input id='password' placeholder='password (leave blank if open)' type='password'><br><input type='button' value='Submit' onclick='submitNetwork'></section><script>function submitNetwork(){var ssid = document.getElementById('ssid').value;var password = document.getElementById('password').value;}</script></body>";

WiFiServer server(80);


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

String readLine(WiFiClient* client){
  String ret = "";
  while (client->connected() && client->available()){
    char c = (char) client->read();
    //Serial.write(c);
    
    if (c == '\r') continue;

    if (c == '\n') return ret;

    ret += c;
  }
  return ret;
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port

    String firstLine = readLine(&client);
    Serial.println(firstLine);
    firstLine.toLowerCase();

    int bodyLength = 0;
    
    /// Skip to end of headers
    String line = "p";
    while (client.connected() && client.available() && line.length() != 0){
      line = readLine(&client);
      line.toLowerCase();

      if (line.startsWith("content-length:")){
        bodyLength = std::stoi(line.substring(16).c_str());
      }
    }
    

    if (firstLine.startsWith("post /connectnetwork")){
      Serial.print("Got post request! Body length: ");
      Serial.println(bodyLength);

      String ssid = "";
      String password = "";
      String* active = &ssid;
      for (int i = 0; i < bodyLength; i++){
        char c = (char) client.read();
        if (c == '\n'){
          active = &password;
          continue;
        }
        *active += c;
      }
      Serial.println("Received ssid: " + ssid);
      Serial.println("Received password: " + password);
      
      client.write("HTTP/1.1 201 OK\n");
      client.write("Content-type:text\n");
      client.write("\n");

      client.stop();
      return;
    }

    client.write("HTTP/1.1 201 OK\n");
    client.write("Content-type:text/html\n");
    client.write("\n");

    client.write(webPageP0);
    client.write(webPageP1);

    // close the connection:
    client.stop();
  }
}

