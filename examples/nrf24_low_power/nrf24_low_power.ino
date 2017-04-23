#include "arduino_control.h"
#include "utility/nrf24_arduino_control.h"

const char this_address[] = "s-1";
const char tx_address[]   = "GAT";
const char mux_address[]  = "MUX";

//const uint8_t pin = 9; // Mini
const uint8_t pin = 8; // Nano

void setup()
{
  Serial.begin(115200);
  
  if (!nrf24_setup(pin))
    Serial.println("init failed");

  LowPowerSleepModeMessage mode = LOW_POWER_SLEEP_MODE_AUTO;
  if (!nrf24_setup_low_power_sleep(&mode))
    Serial.println("init low power sleep failed");

  if (!nrf24_setup_addresses(this_address, tx_address, mux_address))
    Serial.println("init address failed");

  Serial.println("initialised");
}

void loop()
{
  nrf24_loop();
}
