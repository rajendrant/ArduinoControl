#include "arduino_control.h"
#include "NRF24_low_latency.h"

const uint8_t this_address[] = "s-1";
const uint8_t tx_address[]   = "GAT";
const uint8_t mux_address[]  = "MUX";

//NRF24 nrf24(9); // Mini
NRF24 nrf24;

void setup()
{
  Serial.begin(115200);

  int retries=5;
  while (!nrf24.init() && retries--)
    Serial.println("NRF24 init failed");
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate1Mbps, NRF24::NRF24TransmitPower0dBm))
    Serial.println("setRF failed");

  if (!nrf24.setThisAddress(this_address, 3))
    Serial.println("setThisAddress failed");
  if (!nrf24.setTransmitAddress(tx_address, 3))
    Serial.println("setTransmitAddress failed");
  if (!nrf24.setBroadcastAddress(mux_address, 3))
    Serial.println("setBroadcastAddress failed");

  uint32_t address = 0;
  nrf24.getThisAddress((uint8_t*)&address, 3);
  ArduinoControl.set_this_address(address);

  ArduinoControl.set_low_power_sleep(true);

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
    Serial.print("req ");
    Serial.println(req_len);
    if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
      /*Serial.println("response");
      Serial.println(resp_len);
      Serial.println(resp[0]);
      Serial.println(resp[1]);
      Serial.println(resp[2]);
      Serial.println(resp[3]);*/
      nrf24.setTransmitAddress(tx_address, 3);
      if(!nrf24.sendBlocking(resp, resp_len))
        Serial.println("send failed");
    }
  }
  if (ArduinoControl.get_low_power_sleep()) {
    nrf24.powerDown();
    if (ArduinoControl.power_down_loop(&resp, &resp_len) && resp_len) {
      Serial.println("wake pulse");
      nrf24.powerUpTx();
      nrf24.setTransmitAddress(tx_address, 3);
      nrf24.sendNoAck(resp, resp_len);
      nrf24.flushTx();
    }
  }
}
