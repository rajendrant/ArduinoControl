#include <Servo.h>
#include <ESP8266WiFi.h>

#define UDP_SERVER
#ifdef UDP_SERVER
#include <WiFiUdp.h>
#endif

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "message.pb.h"

const char* ssid     = "";
const char* password = "";
int port_to_listen = 6666;

#ifdef UDP_SERVER
WiFiUDP wifiUDP;
#else
WiFiServer wifiTCP(port_to_listen);
#endif

// Wraps the TCP and UDP client.
class ClientWrap {

#ifdef UDP_SERVER
 public:

  ClientWrap() {}

  void Write(void *buffer, size_t size) {
    wifiUDP.beginPacket(wifiUDP.remoteIP(), wifiUDP.remotePort());
    wifiUDP.write((uint8_t*)buffer, size);
    wifiUDP.endPacket();
  }
#else
 public:

  ClientWrap(Client *client) : client(client) {}

  void Write(void *buffer, size_t size) {
    client->write((uint8_t*)buffer, size);
  }

private:
  Client *client;
#endif

};

bool send_message(Message *msg, ClientWrap *client) {
  uint8_t buffer[128];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer+1, sizeof(buffer)-1);
  if (!pb_encode(&stream, Message_fields, msg))
    return false;

  buffer[0] = stream.bytes_written;
  client->Write(buffer, stream.bytes_written+1);
  return true;
}

bool receive_message(Message *msg, Client *client) {
  uint8_t buffer[128];
  int start = 0;
#ifdef UDP_SERVER
  int message_length = wifiUDP.parsePacket();
  if (!message_length)
    return false;
  int len = wifiUDP.read(buffer, sizeof(buffer));
  if (!len)
    return false;
  buffer[len] = 0;
  start = 1;
#else
  uint8_t message_length = client->read();
  if (client->readBytes(buffer, message_length) != message_length) {
    return false;
  }
#endif

  pb_istream_t stream = pb_istream_from_buffer(buffer+1, message_length);

  if (!pb_decode(&stream, Message_fields, msg)) {
    return false;
  }

  return true;
}

void handle_ping_test(PingTest *msg, ClientWrap *client) {
  Message resp = Message_init_zero;
  resp.which_msg = Message_ping_test_tag;
  resp.msg.ping_test = *msg;
  send_message(&resp, client);
}

void handle_pin_mode_control(PinModeControl *msg, ClientWrap *client) {
  pinMode(msg->pin,
          msg->mode==PinModeControl_Mode_OUTPUT ? OUTPUT :
          msg->mode==PinModeControl_Mode_INPUT ? INPUT : INPUT_PULLUP);
  Message resp = Message_init_zero;
  resp.which_msg = Message_pin_mode_control_tag;
  resp.msg.pin_mode_control = *msg;
  send_message(&resp, client);
}

void handle_io_read_write(IOReadWrite *msg, ClientWrap *client) {
  Message resp = Message_init_zero;
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
  send_message(&resp, client);
}

Servo servos[20];
void handle_servo_control(ServoControl *msg, ClientWrap *client) {
  Message resp = Message_init_zero;
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
  send_message(&resp, client);
}

void handle_message(Message *msg, ClientWrap *client) {
  switch(msg->which_msg) {
    case Message_ping_test_tag:
      handle_ping_test(&msg->msg.ping_test, client);
      break;
    case Message_pin_mode_control_tag:
      handle_pin_mode_control(&msg->msg.pin_mode_control, client);
      break;
    case Message_io_read_write_tag:
      handle_io_read_write(&msg->msg.io_read_write, client);
      break;
    case Message_servo_control_tag:
      handle_servo_control(&msg->msg.servo_control, client);
      break;
  }
}

void connectToWiFi() {
  digitalWrite(0, LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
#ifdef UDP_SERVER
  wifiUDP.begin(port_to_listen);
#else
  wifiTCP.begin();
#endif
  digitalWrite(0, HIGH);
}

void checkConnectivity() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
}

#ifdef UDP_SERVER
void processUDPRequest() {
  Message req = Message_init_zero;
  if (receive_message(&req, NULL)) {
    ClientWrap client_wrap;
    handle_message(&req, &client_wrap);
  }
}
#else
void processTCPRequest(Client* client) {
  if (client->connected() && client->available()) {
    Message req = Message_init_zero;
    if (receive_message(&req, client)) {
      ClientWrap client_wrap(client);
      handle_message(&req, &client_wrap);
    }
  }
}
#endif



int servo_pins[][2] = {{14, 180}, {12, 0}, {5, 0}};
int input_pins[] = {0, 2, 4, 13, 15, 16};

void setup() {
  //Serial.begin(115200);
  // Servo connected pins.
  for(int i=0; i<3; i++) {
    Servo s;
    s.attach(servo_pins[i][0]);
    s.write(servo_pins[i][1]);
    s.detach();
  }

  // Start with all pins as OUTPUT
  for(int i=0; i<sizeof(input_pins)/sizeof(int); i++) {
    pinMode(input_pins[i], OUTPUT);
  }

  connectToWiFi();
}

#ifndef UDP_SERVER
#define MAX_CLIENTS 4
WiFiClient clients[MAX_CLIENTS];
int client_end = 0;
#endif

void loop() {
  static int connectivity_check = 0;

#ifdef UDP_SERVER
  processUDPRequest();
#else
  clients[client_end] = wifiTCP.available();
  if (clients[client_end]) {
    client_end = (client_end+1) % MAX_CLIENTS;
  }
  for(int i=0; i < MAX_CLIENTS; i++) {
    if (clients[i])
      processTCPRequest(&clients[i]);
  }
#endif

  if(++connectivity_check == 1000) {
    checkConnectivity();
    connectivity_check = 0;
  }
  delay(10);
}
