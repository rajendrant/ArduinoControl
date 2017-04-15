#ifndef ARDUINO_CONTROL_GATEWAY_h
#define ARDUINO_CONTROL_GATEWAY_h

#include "arduino_control.h"

class ArduinoControlGateway {
public:
  ArduinoControlGateway(uint8_t (*recv_handler)(void*, uint8_t), 
                        bool (*send_handler)(const void*, uint8_t, const void*))
    : recv_handler(recv_handler), send_handler(send_handler) {
  }

  bool process_message(const void *req, uint8_t req_len, uint8_t **resp_buf, uint8_t *resp_len);

private:
  uint8_t (*recv_handler)(void*, uint8_t);
  bool (*send_handler)(const void*, uint8_t, const void*);
};

#endif // ARDUINO_CONTROL_GATEWAY_h
