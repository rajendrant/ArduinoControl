#include "NRF24_low_latency.h"
#include "arduino_control.h"

#define THIS_ADDRESS "GAT"
NRF24 nrf24(4);

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  if (!nrf24.init())
    Serial.println("NRF24 init failed");
  
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPowerm18dBm))
    Serial.println("setRF failed");

  if (!nrf24.setThisAddress((uint8_t*)THIS_ADDRESS, 3))
    Serial.println("setThisAddress failed");

  uint32_t address = 0;
  nrf24.getThisAddress((uint8_t*)&address, 3);
  ArduinoControl.set_this_address(address);
  
  wifi_udp_setup();
}

void loop()
{
  uint8_t req[128];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  wifi_udp_loop();

  unsigned long start = millis();
  while(start + 10 > millis()) {
    req_len = wifi_udp_recv_request(req, sizeof(req));
    if (req_len) {
      if (!strncmp((char*)req, THIS_ADDRESS, 3)) {
        if (ArduinoControl.process_message(req+3, req_len-3, &resp, &resp_len)) {
          wifi_udp_send_response(resp, resp_len);
        }    
      } else {
        nrf24.setTransmitAddress(req, 3);
        nrf24.sendBlocking(req+3, req_len-3);

        nrf24.powerUpRx();
        resp_len = nrf24.waitAndRecv(req, 400);
        if (resp_len) {
          wifi_udp_send_response(req, resp_len);
        }
      }
    }
    delay(1);
  }
}
