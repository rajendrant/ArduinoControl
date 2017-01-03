#include <ESP8266WiFi.h>

#include "arduino_control.h"

const char* ssid     = "";
const char* password = "";
int port_to_listen = 6666;

WiFiServer wifiTCP(port_to_listen);

void connectToWiFi() {
  digitalWrite(0, LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  wifiTCP.begin();
  digitalWrite(0, HIGH);
}

void setup() {
  //Serial.begin(115200);
  connectToWiFi();
}

#define MAX_CLIENTS 4
WiFiClient clients[MAX_CLIENTS];
int client_end = 0;

void loop() {
  uint8_t req[128];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  clients[client_end] = wifiTCP.available();
  if (clients[client_end]) {
    client_end = (client_end+1) % MAX_CLIENTS;
  }
  for(int i=0; i < MAX_CLIENTS; i++) {
    WiFiClient *client = &clients[i];
    if (client && client->connected() && client->available()) {
      req_len = client->read();
      if (client->readBytes(req, req_len) == req_len) {
        if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
          client->write((uint8_t*)resp, resp_len);
        }
      }
    }
  }
}
