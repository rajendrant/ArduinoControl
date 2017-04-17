#include "arduino_control.h"
#include "arduino_control_gateway.h"
#include "message.h"

#ifdef AVR
#include <avr/wdt.h>
#endif

#define ADDRESS_WIDTH 4

uint8_t resp_buf[128];

uint8_t recv_with_retry(uint8_t msg_id, uint8_t (*recv_handler)(void*, uint8_t), uint8_t retries) {
  do {
    uint8_t resp_len = recv_handler(resp_buf, sizeof(resp_buf));
    if (resp_len && resp_buf[0] == msg_id) {
      return resp_len;
    }
    wdt_reset();
  } while(--retries);
  return 0;
}

bool ArduinoControlGateway::process_message(const void *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len) {
  if (!req || !req_len || req_len<=sizeof(GatewayMessageHeader) || !resp || !resp_len)
    return false;

  const GatewayMessageHeader *hdr = (const GatewayMessageHeader*)req;
  uint8_t msg_len = req_len-sizeof(GatewayMessageHeader);

  if (hdr->forward_address == ArduinoControl.get_this_address()) {
    return ArduinoControl.process_message(hdr->msg, msg_len, resp, resp_len);
  } else {
    uint8_t retries = hdr->retry&0xF;
    while(!send_handler(hdr->msg, msg_len, &hdr->forward_address) && retries--) {
      wdt_reset();
      *resp_len = recv_handler(resp_buf, sizeof(resp_buf));
      delay(1);
    }
    if (retries) {
      *resp_len = recv_with_retry(hdr->msg[0].which_msg, recv_handler, (hdr->retry&0xF0)>>4);
      if (*resp_len) {
        *resp = resp_buf;
        return true;
      }
    }
  }
  return false;
}
