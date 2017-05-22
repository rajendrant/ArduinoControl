#include "arduino_control.h"
#include "utility/nrf24_arduino_control.h"

#define PIN_VOLTAGE_DETECTOR A1
#define PIN_VOLTAGE_DETECTOR_ENABLE 5
#define PIN_PC_CONTROL 3
#define PIN_CHARGER_CONTROL 2

// 1M 220k
void get_voltage(uint32_t *resp) {
  resp[0] = read_vcc();
  const float MULT = 1.0/1024 * (221.0+1017)/221;
  pinMode(PIN_VOLTAGE_DETECTOR_ENABLE, OUTPUT);
  digitalWrite(PIN_VOLTAGE_DETECTOR_ENABLE, LOW);
  delay(10);
  resp[1] = analogRead(PIN_VOLTAGE_DETECTOR) * MULT * resp[0];
  pinMode(PIN_VOLTAGE_DETECTOR_ENABLE, INPUT);
}

enum tlv_msg_type {
  NRF24_LOW_POWER_ARDUINO_CONTROL_MSG_TYPES,
  MSG_MEASURE_VOLTAGE,
};

uint8_t tlv_msg_handler(uint8_t type, uint8_t len, uint8_t* val, uint8_t* resp) {
  if (type==MSG_MEASURE_VOLTAGE && len==0) {
    get_voltage((uint32_t*)resp);
    return 8;
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_VOLTAGE_DETECTOR, INPUT);
  pinMode(PIN_VOLTAGE_DETECTOR_ENABLE, INPUT);
  pinMode(PIN_PC_CONTROL, OUTPUT);
  pinMode(PIN_CHARGER_CONTROL, OUTPUT);
  if (!nrf24_setup(9, tlv_msg_handler))
    Serial.println("init failed");

  LowPowerSleepModeMessage mode = LOW_POWER_SLEEP_MODE_AUTO;
  mode.sleep_duration_desisecs = 20; // 2 Seconds.
  if (!nrf24_setup_low_power_sleep(&mode))
    Serial.println("init low power sleep failed");

  if (!nrf24_setup_addresses())
    Serial.println("init address failed");
}

void loop() {
  nrf24_loop();
}
