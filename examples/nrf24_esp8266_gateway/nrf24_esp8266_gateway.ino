#include "NRF24_low_latency.h"
#include "arduino_control.h"
#include "arduino_control_gateway.h"

#define THIS_ADDRESS "GAT"
NRF24 nrf24(4);

void setup()
{
  Serial.begin(115200);
  Serial.println("init");
  if (!nrf24.init())
    Serial.println("NRF24 init failed");
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate1Mbps, NRF24::NRF24TransmitPower0dBm))
    Serial.println("setRF failed");

  if (!nrf24.setThisAddress((uint8_t*)THIS_ADDRESS, 3))
    Serial.println("setThisAddress failed");

  uint32_t address = 0;
  nrf24.getThisAddress((uint8_t*)&address, 3);
  ArduinoControl.set_this_address(address);
  
  wifi_udp_setup();
}

uint8_t nrf24_recv_handler(void *msg, uint8_t msg_len) {
  nrf24.powerUpRx();
  return nrf24.waitAndRecv(msg, 400);
}

bool nrf24_send_handler(const void *msg, uint8_t msg_len, const void *tx_addr) {
  nrf24.setTransmitAddress(tx_addr, 3);
  return nrf24.sendBlocking(msg, msg_len);
}

ArduinoControlGateway gateway(nrf24_recv_handler, nrf24_send_handler);

void loop()
{
  uint8_t req[128];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  wifi_udp_loop();

  unsigned long start = millis();
  while(start + 10 > millis()) {
    req_len = wifi_udp_recv_request(req, sizeof(req));
    if (req_len && gateway.process_message(req, req_len, &resp, &resp_len) && resp_len) {
      wifi_udp_send_response(resp, resp_len);
    }
    delay(1);
  }
}
