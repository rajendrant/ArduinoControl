#ifndef ARDUINO_CONTROL_h
#define ARDUINO_CONTROL_h

#include <Arduino.h>

bool process_message(const uint8_t *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len);

#endif // ARDUINO_CONTROL_h
