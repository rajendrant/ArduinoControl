#ifndef ARDUINO_CONTROL_h
#define ARDUINO_CONTROL_h

#include <Arduino.h>

class ArduinoControlClass {
public:
  void set_this_address(uint32_t address);
  bool process_message(const uint8_t *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len);

  void set_tlv_handler(uint8_t (*fn)(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp)) {
    fn_tlv_handler = fn;
  }

private:
  uint32_t this_address = 0;
  uint8_t (*fn_tlv_handler)(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp) = NULL;
};

extern ArduinoControlClass ArduinoControl;

#endif // ARDUINO_CONTROL_h
