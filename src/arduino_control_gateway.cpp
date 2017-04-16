#include "arduino_control.h"
#include "arduino_control_gateway.h"
#include "message.h"

#ifdef AVR
#include <avr/wdt.h>
#endif

#define ADDRESS_WIDTH 4

uint8_t resp_buf[128];

uint8_t recv_with_retry(uint8_t msg_id, uint8_t (*recv_handler)(void*, uint8_t)) {
  uint8_t retries = 2;
  do {
    uint8_t resp_len = recv_handler(resp_buf, sizeof(resp_buf));
    if (resp_len && resp_buf[0] == msg_id) {
      return resp_len;
    }
  } while(--retries);
  return 0;
}

bool ArduinoControlGateway::process_message(const void *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len) {
  if (!req || !req_len || req_len<=ADDRESS_WIDTH || !resp || !resp_len)
    return false;

  const void *tx_addr = req;
  Message *msg = (Message*)((uint8_t*)req+ADDRESS_WIDTH);
  uint8_t msg_len = req_len-ADDRESS_WIDTH;
  uint32_t address = ArduinoControl.get_this_address();

  if (!memcmp(tx_addr, &address, ADDRESS_WIDTH)) {
    return ArduinoControl.process_message(msg, msg_len, resp, resp_len);
  } else if (msg->which_msg == Message::LOW_POWER_SLEEP_MODE && msg_len == 2) {
    uint8_t retries=12;
    while(!send_handler(msg, 2, tx_addr) && --retries) {
      wdt_reset();
      *resp_len = recv_handler(resp_buf, sizeof(resp_buf));
      delay(1);
      // Wait for the device to send LOW_POWER_WAKE_PULSE.
    }
    if (retries) {
      *resp_len = recv_with_retry(msg->which_msg, recv_handler);
      if (*resp_len) {
        *resp = resp_buf;
        return true;
      }
    }
  } else {
    if (!send_handler(msg, msg_len, tx_addr)) {
      Serial.println("send failed");
      return 0;
    }
    *resp_len = recv_with_retry(msg->which_msg, recv_handler);
    if (*resp_len) {
      *resp = resp_buf;
      return true;
    }
  }
  return false;
}
