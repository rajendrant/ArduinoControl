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

extern NRF24 *nrf24;

uint8_t nrf24_tlv_msg_handler(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp);

bool nrf24_setup(uint8_t nrf24ChipEnablePin, bool lowPowerSleep=false,
                 ArduinoControlClass::TLVMessageHandler fn_tlv_handler=NULL);

bool nrf24_setup_addresses(const char* this_address=NULL,
                           const char* gateway="GAT", const char* mux="MUX");

void nrf24_loop();

#endif // NRF24_ARDUINO_CONTROL_h
