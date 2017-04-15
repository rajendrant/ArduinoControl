#ifndef ARDUINO_CONTROL_h
#define ARDUINO_CONTROL_h

#include <Arduino.h>

#ifdef AVR
#define ENABLE_LOW_POWER_SUPPORT
#endif

#ifdef ENABLE_LOW_POWER_SUPPORT
#include "LowPower.h"
#endif

class ArduinoControlClass {
public:
  void set_this_address(uint32_t address);
  uint32_t get_this_address() const {return this_address;}

  void init_address_from_eeprom();
  bool process_message(const void *req, uint8_t req_len, uint8_t **resp_buf, uint8_t *resp_len);

  void set_tlv_handler(uint8_t (*fn)(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp)) {
    fn_tlv_handler = fn;
  }

#ifdef ENABLE_LOW_POWER_SUPPORT
  void set_low_power_sleep(bool low_power_sleep_) {
    low_power_sleep = low_power_sleep_;
  }
  bool get_low_power_sleep() const {
    return low_power_sleep;
  }

  bool power_down_loop(uint8_t **resp_buf, uint8_t *resp_len);
#endif

private:
  uint32_t this_address = 0;
  uint8_t (*fn_tlv_handler)(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp) = NULL;
  bool low_power_sleep = false;
};

extern ArduinoControlClass ArduinoControl;

#endif // ARDUINO_CONTROL_h
