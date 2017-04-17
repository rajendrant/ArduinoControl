#ifndef ARDUINO_CONTROL_h
#define ARDUINO_CONTROL_h

#include <Arduino.h>

class ArduinoControlClass {
public:
  typedef uint8_t (*TLVMessageHandler)(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp);

  void set_this_address(uint32_t address);
  uint32_t get_this_address() const {return this_address;}

  void init_address_from_eeprom();
  bool process_message(const void *req, uint8_t req_len, uint8_t **resp_buf, uint8_t *resp_len);

  void set_tlv_handler(TLVMessageHandler fn) {
    fn_tlv_handler = fn;
  }

private:
  uint32_t this_address = 0;
  TLVMessageHandler fn_tlv_handler = NULL;
};

extern ArduinoControlClass ArduinoControl;

#endif // ARDUINO_CONTROL_h
