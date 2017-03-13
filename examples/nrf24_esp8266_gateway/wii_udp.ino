#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "";
const char* password = "";
int port_to_listen = 6666;

WiFiUDP wifiUDP;

void connectToWiFi() {
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  wifiUDP.begin(port_to_listen);
  digitalWrite(LED_BUILTIN, HIGH);
}

void checkConnectivity() {
  static uint32_t last_check = 0;
  if (WiFi.status() != WL_CONNECTED) {
    // If disconnected, attempt to connect every 10 seconds.
    if (millis() - last_check > 10000) {
      connectToWiFi();
      last_check = millis();
    }
  }
}

void wifi_udp_setup() {
  connectToWiFi();
}

void wifi_udp_loop() {
  uint8_t *resp;
  uint8_t req_len, resp_len;

  checkConnectivity();
}

uint8_t wifi_udp_recv_request(uint8_t *req, uint8_t req_len) {
  if (!wifiUDP.parsePacket())
    return 0;

  return wifiUDP.read(req, req_len);
}

uint8_t wifi_udp_send_response(uint8_t *resp, uint8_t resp_len) {
  wifiUDP.beginPacket(wifiUDP.remoteIP(), wifiUDP.remotePort());
  wifiUDP.write(resp, resp_len);
  wifiUDP.endPacket();
}
