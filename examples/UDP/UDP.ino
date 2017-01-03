#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "arduino_control.h"

const char* ssid     = "";
const char* password = "";
int port_to_listen = 6666;

WiFiUDP wifiUDP;

void connectToWiFi() {
  digitalWrite(0, LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  wifiUDP.begin(port_to_listen);
  digitalWrite(0, HIGH);
}

void setup() {
  //Serial.begin(115200);
  connectToWiFi();
}

void loop() {
  uint8_t req[128];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  if (!wifiUDP.parsePacket())
    return;

  req_len = wifiUDP.read(req, sizeof(req));
  if (!req_len)
    return;

  if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
    wifiUDP.beginPacket(wifiUDP.remoteIP(), wifiUDP.remotePort());
    wifiUDP.write(resp, resp_len);
    wifiUDP.endPacket();
  }
}
