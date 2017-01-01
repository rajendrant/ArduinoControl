#include "arduino_control.h"

#include <Servo.h>

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "msg/message.pb.h"

Message resp;
uint8_t resp_buffer[128];
 
void handle_ping_test(PingTest *msg) {
  resp = Message_init_zero;
  resp.which_msg = Message_ping_test_tag;
  resp.msg.ping_test = *msg;
}

void handle_pin_mode_control(PinModeControl *msg) {
  pinMode(msg->pin,
          msg->mode==PinModeControl_Mode_OUTPUT ? OUTPUT :
          msg->mode==PinModeControl_Mode_INPUT ? INPUT : INPUT_PULLUP);
  resp = Message_init_zero;
  resp.which_msg = Message_pin_mode_control_tag;
  resp.msg.pin_mode_control = *msg;
}

void handle_io_read_write(IOReadWrite *msg) {
  resp = Message_init_zero;
  resp.which_msg = Message_io_read_write_tag;
  resp.msg.io_read_write = *msg;
  switch(msg->operation) {
  case IOReadWrite_Operation_DIGITAL_READ:
    resp.msg.io_read_write.val = digitalRead(msg->pin);
    break;
  case IOReadWrite_Operation_ANALOG_READ:
    resp.msg.io_read_write.val = analogRead(msg->pin);
    break;
  case IOReadWrite_Operation_DIGITAL_WRITE:
    digitalWrite(msg->pin, resp.msg.io_read_write.val);
    break;
  case IOReadWrite_Operation_ANALOG_WRITE:
    analogWrite(msg->pin, resp.msg.io_read_write.val);
    break;
  }
}

Servo servos[20];
void handle_servo_control(ServoControl *msg) {
  resp = Message_init_zero;
  resp.which_msg = Message_servo_control_tag;
  resp.msg.servo_control = *msg;
  switch(msg->operation) {
  case ServoControl_Operation_ATTACH:
    servos[msg->pin].attach(msg->pin);
     break;
  case ServoControl_Operation_DETACH:
    servos[msg->pin].detach();
    break;
  case ServoControl_Operation_WRITE:
    servos[msg->pin].write(msg->val);
     break;
  case ServoControl_Operation_READ:
    resp.msg.servo_control.val = servos[msg->pin].read();
    break;
  }
}

bool process_message(const uint8_t *req, uint8_t req_len, uint8_t **resp, uint8_t *resp_len) {
  Message msg = Message_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(req, req_len);

  if (!pb_decode(&stream, Message_fields, &msg)) {
    return false;
  }
  switch(msg.which_msg) {
  case Message_ping_test_tag:
    handle_ping_test(&msg.msg.ping_test);
    break;
  case Message_pin_mode_control_tag:
    handle_pin_mode_control(&msg.msg.pin_mode_control);
    break;
  case Message_io_read_write_tag:
    handle_io_read_write(&msg.msg.io_read_write);
    break;
  case Message_servo_control_tag:
    handle_servo_control(&msg.msg.servo_control);
    break;
  }
  
  pb_ostream_t ostream = pb_ostream_from_buffer(resp_buffer, sizeof(resp_buffer));
  if (!pb_encode(&ostream, Message_fields, &msg))
    return false;

  *resp = resp_buffer;
  *resp_len = ostream.bytes_written;
  
  return true;
}
