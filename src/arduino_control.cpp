#include "arduino_control.h"

#include <Servo.h>
#include <EEPROM.h>

#include "message.h"

// From https://github.com/arduino/Arduino/blob/master/libraries/Ethernet/src/utility/util.h
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )

ArduinoControlClass ArduinoControl;

namespace {

Message resp;

#define MSG_INIT_ZERO(m) memset(&m, 0, sizeof(Message));

void handle_ping_test(uint32_t msg) {
  resp.which_msg = Message::PING_TEST;
  resp.msg.ping_test = msg;
}

void handle_pin_mode_control(PinModeControl *msg) {
  pinMode(msg->pin,
          msg->mode==PinModeControl::MODE_INPUT ? INPUT :
          (msg->mode==PinModeControl::MODE_OUTPUT ? OUTPUT : INPUT_PULLUP));
  resp.which_msg = Message::PIN_MODE_CONTROL;
  resp.msg.pin_mode_control = *msg;
}

void handle_io_read_write(IOReadWrite *msg) {
  resp.which_msg = Message::IO_READ_WRITE;
  resp.msg.io_read_write = *msg;
  switch(msg->operation) {
  case IOReadWrite::DIGITAL_READ:
    resp.msg.io_read_write.val = digitalRead(msg->pin);
    break;
  case IOReadWrite::ANALOG_READ:
    resp.msg.io_read_write.val = analogRead(msg->pin);
    break;
  case IOReadWrite::DIGITAL_WRITE:
    digitalWrite(msg->pin, resp.msg.io_read_write.val);
    break;
  case IOReadWrite::ANALOG_WRITE:
    analogWrite(msg->pin, resp.msg.io_read_write.val);
    break;
  }
}

Servo servos[20];
void handle_servo_control(ServoControl *msg) {
  resp.which_msg = Message::SERVO_CONTROL;
  resp.msg.servo_control = *msg;
  switch(msg->operation) {
  case ServoControl::ATTACH:
    servos[msg->pin].attach(msg->pin);
    break;
  case ServoControl::DETACH:
    servos[msg->pin].detach();
    break;
  case ServoControl::WRITE:
    servos[msg->pin].write(msg->val);
    break;
  case ServoControl::READ:
    resp.msg.servo_control.val = servos[msg->pin].read();
    break;
  }
}

void handle_this_address(uint32_t this_address) {
  resp.which_msg = Message::THIS_ADDRESS;
  resp.msg.this_address = htonl(this_address);
}

void handle_system_uptime() {
  resp.which_msg = Message::SYSTEM_UPTIME;
  resp.msg.system_uptime = htonl(millis());
}

} // namespace

void ArduinoControlClass::set_this_address(uint32_t address) {
  this_address = address;
}

bool ArduinoControlClass::process_message(const void *req, uint8_t req_len, uint8_t **resp_buf, uint8_t *resp_len) {
  Message *msg = (Message*)req;
  MSG_INIT_ZERO(resp);

  if (req_len <= 0)
    return false;

  switch(msg->which_msg) {
  case Message::PING_TEST:
    if (req_len != 1+sizeof(uint32_t)) return false;
    handle_ping_test(msg->msg.ping_test);
    *resp_len = 1+sizeof(uint32_t);
    break;
  case Message::PIN_MODE_CONTROL:
    if (req_len != 1+sizeof(PinModeControl)) return false;
    handle_pin_mode_control(&msg->msg.pin_mode_control);
    *resp_len = 1+sizeof(PinModeControl);
    break;
  case Message::IO_READ_WRITE:
    if (req_len != 1+sizeof(IOReadWrite)) return false;
    handle_io_read_write(&msg->msg.io_read_write);
    *resp_len = 1+sizeof(IOReadWrite);
    break;
  case Message::SERVO_CONTROL:
    if (req_len != 1+sizeof(ServoControl)) return false;
    handle_servo_control(&msg->msg.servo_control);
    *resp_len = 1+sizeof(ServoControl);
    break;
  case Message::THIS_ADDRESS:
    if (req_len != 1+sizeof(uint32_t)) return false;
    handle_this_address(this_address);
    *resp_len = 1+sizeof(uint32_t);
    break;
  case Message::SYSTEM_UPTIME:
    if (req_len != 1+sizeof(uint32_t)) return false;
    handle_system_uptime();
    *resp_len = 1+sizeof(uint32_t);
    break;
  case Message::TLV_MESSAGE:
    if (!fn_tlv_handler) return false;
    if (req_len < 1+2) return false;
    resp.which_msg = Message::TLV_MESSAGE;
    resp.msg.tlv_message.type = msg->msg.tlv_message.type;
    resp.msg.tlv_message.len = fn_tlv_handler(msg->msg.tlv_message.type, msg->msg.tlv_message.len,
                                              msg->msg.tlv_message.val, resp.msg.tlv_message.val);
    *resp_len = 1+2+resp.msg.tlv_message.len;
    break;
  case Message::LOW_POWER_SLEEP_MODE:
#ifdef ENABLE_LOW_POWER_SUPPORT
    if (req_len != 1+sizeof(uint8_t)) return false;
    set_low_power_sleep(msg->msg.low_power_sleep);
    resp.which_msg = Message::LOW_POWER_SLEEP_MODE;
    resp.msg.low_power_sleep = low_power_sleep;
    *resp_len = 1+sizeof(uint8_t);
    break;
#endif
  case Message::LOW_POWER_WAKE_PULSE:
    return false;
  }

  *resp_buf = (uint8_t*)&resp;
  return true;
}

void ArduinoControlClass::init_address_from_eeprom() {
#ifdef AVR
  const int ADDRESS_WIDTH = 3;
  uint8_t address[ADDRESS_WIDTH];
  // This address is stored in EEPROM address 0-2.
  for(int i=0; i<ADDRESS_WIDTH; i++) {
    address[i] = EEPROM.read(i);
  }
  if (address[0] == 255) {
    // Init the random seed, by reading random values from unconnected pins.
    // http://arduino.stackexchange.com/questions/22070/powering-from-3v-coin-cell-to-vcc-using-i-o-pin-as-supply-gnd
    long seed = micros();
    seed ^= analogRead(A0) ^ analogRead(A3) ^ analogRead(A6);
    seed ^= (analogRead(A1) ^ analogRead(A4) ^ analogRead(A7))<<10;
    seed ^= (analogRead(A2) ^ analogRead(A5) ^ analogRead(A0))<<20;
    seed ^= micros();
    randomSeed(seed);

    // Update address.
    for(int i=0; i<ADDRESS_WIDTH; i++) {
      address[i] = random(256);
      EEPROM.write(i, address[i]);
    }
  }
  memcpy(&this_address, address, ADDRESS_WIDTH);
#endif
}

#ifdef ENABLE_LOW_POWER_SUPPORT
bool ArduinoControlClass::power_down_loop(uint8_t **resp_buf, uint8_t *resp_len) {
  if (low_power_sleep) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    resp.which_msg = Message::LOW_POWER_WAKE_PULSE;
    resp.msg.this_address = this_address;
    *resp_buf = (uint8_t*)&resp;
    *resp_len = 1+4;
    return true;
  }
  return false;
}
#endif
