#include "nrf24_arduino_control.h"

#include "message.h"

#ifdef NRF24_ARDUINO_CONTROL_LOW_POWER_ENABLE
#include "LowPower.h"
#endif

NRF24 *nrf24;
LowPowerSleepModeMessage mode = LOW_POWER_SLEEP_MODE_DISABLED;
bool low_power_sleep_enabled = false;
uint32_t next_inactivity_sleep_ms = 0;
const char *gateway_address = NULL;
ArduinoControlClass::TLVMessageHandler fn_tlv_message_handler = NULL;
period_t sleep_duration_period;

period_t get_sleep_duration() {
  int ms = mode.sleep_duration_desisecs*100;
  if (ms <= 60)
    return SLEEP_60MS;
  else if (ms <= 120)
    return SLEEP_120MS;
  else if (ms <= 250)
    return SLEEP_250MS;
  else if (ms <= 500)
    return SLEEP_500MS;
  else if (ms <= 1000)
    return SLEEP_1S;
  else if (ms <= 2000)
    return SLEEP_2S;
  else if (ms <= 4000)
    return SLEEP_4S;
  else if (ms <= 8000)
    return SLEEP_8S;
}

uint8_t nrf24_tlv_msg_handler(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp) {
  if (type == TYPE_LOW_POWER_SLEEP_MODE) {
    if (len != sizeof(LowPowerSleepModeMessage)) return 0;
    const LowPowerSleepModeMessage* msg = nrf24_setup_low_power_sleep((LowPowerSleepModeMessage*)val);
    memcpy(resp, msg, sizeof(LowPowerSleepModeMessage));
    return sizeof(LowPowerSleepModeMessage);
  } else if (type == TYPE_LOW_POWER_WAKE_PULSE) {
    return 0;
  } else if (fn_tlv_message_handler) {
    return fn_tlv_message_handler(type, len, val, resp);
  }
  return 0;
}

bool nrf24_setup(uint8_t nrf24ChipEnablePin,
                 ArduinoControlClass::TLVMessageHandler fn_tlv_handler) {
  nrf24 = new NRF24(nrf24ChipEnablePin);
  if (!nrf24->init() ||
      !nrf24->setChannel(1) ||
      !nrf24->setRF(NRF24::NRF24DataRate1Mbps, NRF24::NRF24TransmitPower0dBm)) {
    return false;
  }
  fn_tlv_message_handler = fn_tlv_handler;
  ArduinoControl.set_tlv_handler(nrf24_tlv_msg_handler);
  sleep_duration_period = get_sleep_duration();
  low_power_sleep_enabled = false;
  return true;
}

const LowPowerSleepModeMessage* nrf24_setup_low_power_sleep(const LowPowerSleepModeMessage *msg) {
  mode = *msg;
  LowPowerSleepModeMessage def = LOW_POWER_SLEEP_MODE_DISABLED;
  if (mode.sleep_duration_desisecs==0)
    mode.sleep_duration_desisecs = def.sleep_duration_desisecs;
  if (mode.wake_pulse_count==0)
    mode.wake_pulse_count = def.wake_pulse_count;
  if (mode.sleep_after_inactivity_secs==0)
    mode.sleep_after_inactivity_secs = def.sleep_after_inactivity_secs;
  if (mode.mode != LowPowerSleepMode::AUTO_AFTER_INACTIVITY)
    mode.sleep_after_inactivity_secs = 0;
  if (mode.wake_up_recv_ms==0)
    mode.wake_up_recv_ms = def.wake_up_recv_ms;
  sleep_duration_period = get_sleep_duration();
  if (mode.mode == LowPowerSleepMode::MANUAL)
    low_power_sleep_enabled = false;
  return &mode;
}

bool nrf24_setup_addresses(const char* this_address,
                           const char* gateway, const char* mux) {
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
      !nrf24->setBroadcastAddress((uint8_t*)mux, 3)) {
    return false;
  }
  next_inactivity_sleep_ms = millis() + mode.sleep_after_inactivity_secs*1000;
  return true;
}

void nrf24_loop()
{
  uint8_t req[32];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  nrf24->powerUpRx();
  req_len = nrf24->waitAndRecv(req, mode.wake_up_recv_ms);
  if (req_len) {
    next_inactivity_sleep_ms = millis() + mode.sleep_after_inactivity_secs*1000;
    Serial.print("req ");
    Serial.println(req_len);
    //Serial.println((uint32_t)millis());
    //hexdump(req, req_len);
    if (ArduinoControl.process_message(req, req_len, &resp, &resp_len)) {
      Serial.print("resp ");
      Serial.println(resp_len);
      //Serial.println((uint32_t)millis());
      //hexdump(resp, resp_len);
      delay(1);
      nrf24->setTransmitAddress(gateway_address, 3);
      nrf24->sendBlocking(resp, resp_len);
    }
  }
#ifdef NRF24_ARDUINO_CONTROL_LOW_POWER_ENABLE
  if ((mode.mode == LowPowerSleepMode::MANUAL && low_power_sleep_enabled) ||
      (mode.mode == LowPowerSleepMode::AUTO_AFTER_INACTIVITY &&
       next_inactivity_sleep_ms <= millis())) {
    Message resp;
    nrf24->powerDown();
    LowPower.powerDown(sleep_duration_period, ADC_OFF, BOD_OFF);
    // After wake up, send pulse msg.
    resp.which_msg = Message::TLV_MESSAGE | (TYPE_LOW_POWER_WAKE_PULSE>>4);
    nrf24->powerUpTx();
    nrf24->setTransmitAddress(gateway_address, 3);
    for(int i=mode.wake_pulse_count; i--;) {
      nrf24->sendNoAck(&resp, 1);
      nrf24->waitPacketSent();
    }
    nrf24->flushTx();
  }
#endif
}
