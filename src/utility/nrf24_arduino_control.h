#include "arduino_control.h"
#include "message.h"
#include "NRF24_low_latency.h"
#include "LowPower.h"

#define TYPE_LOW_POWER_SLEEP_MODE 0
#define TYPE_LOW_POWER_WAKE_PULSE 1

#define NRF24_LOW_POWER_ARDUINO_CONTROL_MSG_TYPES       \
  LOW_POWER_SLEEP_MODE = TYPE_LOW_POWER_SLEEP_MODE, \
  LOW_POWER_WAKE_PULSE = TYPE_LOW_POWER_WAKE_PULSE


NRF24 *nrf24;
const char *gateway_address = NULL;
bool low_power_sleep = false;
ArduinoControlClass::TLVMessageHandler fn_tlv_message_handler = NULL;

uint8_t nrf24_tlv_msg_handler(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp) {
  if (type == TYPE_LOW_POWER_SLEEP_MODE) {
    if (len != 1) return 0;
    resp[0] = low_power_sleep = val[0];
    return 1;
  } else if (type == TYPE_LOW_POWER_WAKE_PULSE) {
    return 0;
  } else if (fn_tlv_message_handler) {
    return fn_tlv_message_handler(type, len, val, resp);
  }
  return 0;
}

bool nrf24_setup(uint8_t nrf24ChipEnablePin, bool lowPowerSleep=false,
                 ArduinoControlClass::TLVMessageHandler fn_tlv_handler=NULL,
                 const char* this_address=NULL,
                 const char* gateway="GAT", const char* mux="MUX") {
  nrf24 = new NRF24(nrf24ChipEnablePin);
  if (!nrf24->init() ||
      !nrf24->setChannel(1) ||
      !nrf24->setRF(NRF24::NRF24DataRate1Mbps, NRF24::NRF24TransmitPower0dBm))
    return false;

  gateway_address = gateway;
  if (!this_address) {
    ArduinoControl.init_address_from_eeprom();
    uint32_t address = ArduinoControl.get_this_address();
    if (!nrf24->setThisAddress((uint8_t*)&address, 3))
      return false;
  } else {
    if (!nrf24->setThisAddress(this_address, 3))
      return false;
    uint32_t address = 0;
    nrf24->getThisAddress((uint8_t*)&address, 3);
    ArduinoControl.set_this_address(address);
  }
  if (!nrf24->setTransmitAddress((uint8_t*)gateway, 3) ||
      !nrf24->setBroadcastAddress((uint8_t*)mux, 3))
    return false;

  low_power_sleep = lowPowerSleep;
  fn_tlv_message_handler = fn_tlv_handler;
  ArduinoControl.set_tlv_handler(nrf24_tlv_msg_handler);

  return true;
}

void nrf24_loop()
{
  uint8_t req[32];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  nrf24->powerUpRx();
  req_len = nrf24->waitAndRecv(req, 4);
  if (req_len) {
    Serial.print("req ");
    Serial.println(req_len);
    if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
      Serial.print("resp ");
      Serial.println(resp_len);
      nrf24->setTransmitAddress(gateway_address, 3);
      nrf24->sendBlocking(resp, resp_len);
    }
  }
  if (low_power_sleep) {
    Message resp;
    nrf24->powerDown();
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
    // After wake up, send pulse msg.
    resp.which_msg = Message::TLV_MESSAGE | (TYPE_LOW_POWER_WAKE_PULSE>>4);
    nrf24->powerUpTx();
    nrf24->setTransmitAddress(gateway_address, 3);
    nrf24->sendNoAck(&resp, 1);
    nrf24->waitPacketSent();
    nrf24->sendNoAck(&resp, 1);
    nrf24->waitPacketSent();
    nrf24->flushTx();
  }
}
