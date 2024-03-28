#include <esp_timer.h> 
#include <Arduino.h>
#include "gpio_interface.h"
#include "hardware_test.h"
#include "device_states.h"
#include "utility_functions.h"
#include "config.h"

void hardware_test_loop();
void device_control_loop();

void setup() {
  Serial.begin(115200);
  init_gpio();
}

void loop() {
  //hardware_test_loop();
  device_control_loop();
}

static unsigned long device_ready_time = 0;
uint16_t current_measurement = 0;
void device_control_loop() {
  uint8_t flag_count = 0;
  int64_t microseconds = 0;
  
  // The loop can remain empty if no additional logic is required here.
  switch(get_device_state()) {
    case DEVICE_IDLE:
      update_device_state(DEVICE_RESET);
      break;
    case DEVICE_RESET:
      update_device_state(DEVICE_HARDWARE_TEST);
      break;
    case DEVICE_HARDWARE_TEST:
      if(hardware_test()) {
        update_device_state(DEVICE_READY);
        reset_gate_triggers();
        device_ready_time = millis();
        Serial.println("device ready");
      }
      break;
    case DEVICE_READY:
      if(millis() - device_ready_time > DEVICE_READY_TIMEOUT) {
        update_device_state(DEVICE_IDLE);
        Serial.println("device calibration timeout");
      }
      flag_count = check_flags();
      if(flag_count == 2) {
        update_device_state(DEVICE_MEASUREMENT_COMPLETED);
      } else if (flag_count == 1) {
        update_device_state(DEVICE_MEASUREMENT_IN_PROGRESS);
        device_ready_time = millis();
      }
      break;
    case DEVICE_MEASUREMENT_IN_PROGRESS:
      if(check_flags() == 2) {
        update_device_state(DEVICE_MEASUREMENT_COMPLETED);
      } else if(millis() - device_ready_time > 1000U) {
        update_device_state(DEVICE_MEASUREMENT_TIMEOUT);
      }
      break;
    case DEVICE_MEASUREMENT_COMPLETED:
      microseconds = differenceMicroseconds(get_gate_1_time(), get_gate_2_time()) + get_gate_offset();
      if(microseconds < 0) {
        update_device_state(DEVICE_MEASUREMENT_INVALID);
      } else {
        update_device_state(DEVICE_MEASUREMENT_VALID);
        current_measurement = microseconds;
      }
      break;
    case DEVICE_MEASUREMENT_VALID:
      Serial.println("valid measurement");
      Serial.print("time between gates: ");
      Serial.print(current_measurement);
      Serial.print(" uS, ");
      Serial.print((float)current_measurement / 1000.0);
      Serial.print(" mS, ");
      Serial.print(microSecondsToSeconds(current_measurement));
      Serial.println(" S");
      Serial.print("speed: ");
      Serial.print(SENSOR_GAP_M / microSecondsToSeconds(current_measurement));
      Serial.println(" m/s");
      update_device_state(DEVICE_IDLE);
      break;
    case DEVICE_MEASUREMENT_INVALID:
      Serial.println("invalid measurement");
      update_device_state(DEVICE_IDLE);
      break;
    case DEVICE_MEASUREMENT_TIMEOUT:
      Serial.println("sensor timeout during measurement");
      update_device_state(DEVICE_IDLE);
      break;
    default:
      break;

  }
  delay(100);
}

void hardware_test_loop() {
  while(!hardware_test()) {
    delay(100);
  }
  delay(5000);
}