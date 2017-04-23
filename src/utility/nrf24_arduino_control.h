#ifndef NRF24_ARDUINO_CONTROL_h
#define NRF24_ARDUINO_CONTROL_h

#include "NRF24_low_latency.h"
#include "arduino_control.h"

#ifdef AVR
#define NRF24_ARDUINO_CONTROL_LOW_POWER_ENABLE 1
#endif

#define TYPE_LOW_POWER_SLEEP_MODE 0
#define TYPE_LOW_POWER_WAKE_PULSE 1

#define NRF24_LOW_POWER_ARDUINO_CONTROL_MSG_TYPES       \
  LOW_POWER_SLEEP_MODE = TYPE_LOW_POWER_SLEEP_MODE,     \
  LOW_POWER_WAKE_PULSE = TYPE_LOW_POWER_WAKE_PULSE

enum LowPowerSleepMode {
  // Manually enable or disable low power sleep mode.
  MANUAL = 0,

  // Automatic mode. Goes to low power sleep after X seconds of inactivity.
  AUTO_AFTER_INACTIVITY = 1,
};

typedef struct __attribute__((__packed__)) LowPowerSleepModeMessage__ {
  uint8_t mode;

  // Duration of low power sleep, in desisecs(0.1 seconds).
  uint8_t sleep_duration_desisecs;

  // No of pulses sent after wakeup.
  uint8_t wake_pulse_count;

  // Duration to be in recv mode, after wakeup.
  uint8_t wake_up_recv_ms;

  // No of seconds after inactivity to sleep. Only used in Automatic mode.
  uint8_t sleep_after_inactivity_secs;
} LowPowerSleepModeMessage;

#define LOW_POWER_SLEEP_MODE_DISABLED {                                 \
      .mode                      = LowPowerSleepMode::MANUAL,           \
      .sleep_duration_desisecs   = 1,                                   \
      .wake_pulse_count          = 1,                                   \
      .wake_up_recv_ms           = 100,                                 \
      .sleep_after_inactivity_secs = 0                                  \
      }

#define LOW_POWER_SLEEP_MODE_AUTO {                                     \
      .mode                      = LowPowerSleepMode::AUTO_AFTER_INACTIVITY, \
      .sleep_duration_desisecs   = 10,                                  \
      .wake_pulse_count          = 2,                                   \
      .wake_up_recv_ms           = 4,                                   \
      .sleep_after_inactivity_secs = 2                                  \
      }

extern NRF24 *nrf24;

uint8_t nrf24_tlv_msg_handler(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp);

bool nrf24_setup(uint8_t nrf24ChipEnablePin,
                 ArduinoControlClass::TLVMessageHandler fn_tlv_handler=NULL);

const LowPowerSleepModeMessage* nrf24_setup_low_power_sleep(const LowPowerSleepModeMessage* msg);

bool nrf24_setup_addresses(const char* this_address=NULL,
                           const char* gateway="GAT", const char* mux="MUX");

bool nrf24_setup_low_power_sleep();

void nrf24_loop();

#endif // NRF24_ARDUINO_CONTROL_h
