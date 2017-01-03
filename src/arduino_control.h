#ifndef ARDUINO_CONTROL_h
#define ARDUINO_CONTROL_h

#include <Arduino.h>

class ArduinoControlClass {
public:
  void set_this_address(uint32_t address);
  bool process_message(const uint8_t *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len);

private:
  uint32_t this_address = 0;
};

extern ArduinoControlClass ArduinoControl;

#endif // ARDUINO_CONTROL_h
