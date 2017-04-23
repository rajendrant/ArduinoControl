#include "arduino_control.h"
#include "NRF24_low_latency.h"
#include "utility/nrf24_arduino_control.h"

const char this_address[] = "s-1";
const char tx_address[]   = "GAT";
const char mux_address[]  = "MUX";

//const uint8_t pin = 9; // Mini
const uint8_t pin = 8; // Nano

void setup()
{
  Serial.begin(115200);

  if (!nrf24_setup(pin))
    Serial.println("NRF24 init failed");

  if (!nrf24_setup_addresses(this_address, tx_address, mux_address))
    Serial.println("set address failed");

  uint32_t address = 0;
  nrf24->getThisAddress((uint8_t*)&address, 3);
  ArduinoControl.set_this_address(address);

  Serial.println("initialised");
}

void loop()
{
  uint8_t req[32];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  nrf24->powerUpRx();
  req_len = nrf24->waitAndRecv(req, 400);
  if (req_len) {
    Serial.print("req ");
    //Serial.println(millis());
    //hexdump(req, req_len);
    if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
      Serial.println("resp ");
      //Serial.println(millis());
      //hexdump(resp, resp_len);
      delay(1);
      nrf24->setTransmitAddress(tx_address, 3);
      if(!nrf24->sendBlocking(resp, resp_len))
        Serial.println("send failed");
    }
  }
}
