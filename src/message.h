#include <stdint.h>

typedef struct __attribute__((__packed__)) PinModeControl__ {
  uint8_t pin;
  enum Mode {
    MODE_INPUT = 0,
    MODE_OUTPUT = 1,
    MODE_INPUT_PULLUP = 2,
  };
  uint8_t mode;
} PinModeControl;

typedef struct __attribute__((__packed__)) IOReadWrite__ {
  uint8_t pin;
  enum Operation {
    DIGITAL_READ  = 0,
    DIGITAL_WRITE = 1,
    ANALOG_READ   = 2,
    ANALOG_WRITE  = 3,
  };
  uint8_t operation;
  uint8_t val;
} IOReadWrite;

typedef struct __attribute__((__packed__)) ServoControl {
  uint8_t pin;
  enum Operation {
    ATTACH = 0,
    DETACH = 1,
    WRITE = 2,
    READ = 3,
  };
  uint8_t operation;
  uint8_t val;
} ServoControl;

typedef struct __attribute__((__packed__)) TLVMessage__ {
  uint8_t type;
  uint8_t len;
  uint8_t val[32];
} TLVMessage;

typedef struct __attribute__((__packed__)) Message__ {
  enum Type {
    PING_TEST = 0,
    PIN_MODE_CONTROL = 1,
    IO_READ_WRITE = 2,
    SERVO_CONTROL = 3,
    THIS_ADDRESS = 4,
    SYSTEM_UPTIME = 5,
    TLV_MESSAGE = 6,
  };
  uint8_t which_msg;
  union {
    uint32_t ping_test;
    PinModeControl pin_mode_control;
    IOReadWrite io_read_write;
    ServoControl servo_control;
    uint32_t this_address;
    uint32_t system_uptime;
    TLVMessage tlv_message;
  } msg;
} Message;
