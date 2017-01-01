#include "arduino_control.h"
#include "NRF24_low_latency.h"

NRF24 nrf24;

void setup()
{
  Serial.begin(115200);
  if (!nrf24.init())
    Serial.println("NRF24 init failed");
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setThisAddress((uint8_t*)"s-1", 3))
    Serial.println("setThisAddress failed");
  if (!nrf24.setTransmitAddress((uint8_t*)"s-1", 3))
    Serial.println("setTransmitAddress failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPowerm18dBm))
    Serial.println("setRF failed");

  // Dynamic payload mode.
  if (!nrf24.setDynamicPayloadMode())
    Serial.println("setDynamicPayloadMode failed");

  Serial.println("initialised");
}

void loop()
{
  uint8_t req[32];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  nrf24.powerUpRx();
  req_len = nrf24.waitAndRecv(req, 600);
  if (req_len) {
    if (process_message(req, req_len, &resp, &resp_len)) {
      nrf24.sendBlocking(resp, resp_len);
    }
  }
}
