#include "NRF24_low_latency.h"

NRF24 nrf24;

void setup()
{
  Serial.begin(115200);
  if (!nrf24.init())
    Serial.println("NRF24 init failed");
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setThisAddress((uint8_t*)"s-1", 3))
    Serial.println("setThisAddress failed");
  if (!nrf24.setTransmitAddress((uint8_t*)"s-1", 3))
    Serial.println("setTransmitAddress failed");
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPowerm18dBm))
    Serial.println("setRF failed");

  // Dynamic payload mode.
  if (!nrf24.setDynamicPayloadMode())
    Serial.println("setDynamicPayloadMode failed");
}

uint8_t from_hex(uint8_t no) {
  if (no>='0' && no<='9')
    return no-'0';
  if (no>='a' && no<='f')
    return no-'a'+10;
  if (no>='A' && no<='F')
    return no-'A'+10;
  return 0;
}

uint8_t to_hex(uint8_t no) {
  if (no<10)
    return '0'+no;
  if (no<16)
    return 'A'+no-10;
  return 'X';
}

uint8_t recv_request(uint8_t *req) {
  uint8_t buf[128];
  uint8_t len;
  len = Serial.readBytesUntil('\n', buf, sizeof(buf));
  if (!len)
    return 0;

  for(int i=0; 2*i<len; i++) {
    req[i] = (from_hex(buf[2*i])<<4) | from_hex(buf[2*i+1]);
  }
  return len/2;
}

void send_response(uint8_t* resp, uint8_t resp_len) {
  uint8_t buf[128];
  for(int i=0; i<resp_len; i++) {
    buf[2*i] = to_hex((resp[i]&0xF0)>>4);
    buf[2*i+1] = to_hex(resp[i]&0xF);
  }
  Serial.write(buf, 2*resp_len);
  Serial.println();
}

void loop()
{
  uint8_t req[128];
  uint8_t *resp;
  uint8_t req_len, resp_len;

  req_len = recv_request(req);
  if (!req_len)
    return;

  nrf24.sendBlocking(req, req_len);

  nrf24.powerUpRx();
  resp_len = nrf24.waitAndRecv(req, 20);
  if (resp_len) {
    send_response(resp, resp_len);
  }
  //send_response(req, req_len);
}
