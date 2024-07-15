#include "freertos/FreeRTOS.h" //freeRTOS items to be used
#include "freertos/task.h"
#include "hardware_test.h"
#include "gpio_interface.h"
#include "utility_functions.h"
#include "Arduino.h"

bool hardware_test_active = true;
uint16_t buffer[10];
void init_hardware_test() {
  
}

#define TEST_LENGTH 100
int gate_time_diff_offset = 0;

bool hardware_test() {
  real_time_sd_calc gate_1_data;
  real_time_sd_calc gate_2_data;
  real_time_sd_calc gate_diffs;
  init_sd(&gate_1_data);
  init_sd(&gate_2_data);
  init_sd(&gate_diffs);
  uint16_t gate1_fail = 0;
  uint16_t gate1_long = 0;
  uint16_t gate2_fail = 0;
  uint16_t gate2_long = 0;

  for(int i = 0; i < TEST_LENGTH; i++) {
    ir_led_1_on();
    ir_led_2_on();
    delay(10);
    if(get_gate_1_status() > 0) {
      //Serial.println("gate 1 not ready");
      continue;
    }
    if(get_gate_2_status() > 0) {
      //Serial.println("gate 2 not ready");
      continue;
    }
    reset_gate_triggers();
    ir_led_1_off();
    int64_t toggle_start1 = esp_timer_get_time();
    delay(5);
    int64_t gate_time_1 = get_gate_1_time();
    int64_t diff_microseconds1 = differenceMicroseconds(toggle_start1, gate_time_1);
    if(get_gate_2_time() > 0) {
      Serial.println(" gate 2 triggered early");
      ir_led_2_on();
      delay(5);
      reset_gate_triggers();
    }
    ir_led_2_off();
    int64_t toggle_start2 = esp_timer_get_time();
    delay(5);
    int64_t gate_time_2 = get_gate_2_time();
    int64_t diff_microseconds2 = differenceMicroseconds(toggle_start2, gate_time_2);
    if(diff_microseconds1 < 0) {
      gate1_fail++;
      // Serial.print("gate 1 negative delay: ");
      // Serial.print(gate_time_1);
      // Serial.print(" ");
      // Serial.print(toggle_start1);
      // Serial.print(" ");
      // Serial.print(gate_time_2);
      // Serial.print(" ");
      // Serial.println(toggle_start2);
    } else if(diff_microseconds1 > 100) {
      gate1_long++;
      // Serial.print("gate 1 delayed response: ");
      // log_gate_status();
    } else {
      addDataPoint(&gate_1_data, diff_microseconds1);
    }
    if(diff_microseconds2 < 0) {
      gate2_fail++;
    } else if(diff_microseconds2 > 100) {
      gate2_long++;
      // Serial.print("gate 2 delayed response: ");
      // log_gate_status();
    } else {
      addDataPoint(&gate_2_data, diff_microseconds2);
    }
    if(diff_microseconds1 > 0 && diff_microseconds1 < 100 && diff_microseconds2 > 0 && diff_microseconds2 < 100) {
      addDataPoint(&gate_diffs, differenceMicroseconds(gate_time_1, gate_time_2));
    }

  }
  calculateStdDev(&gate_1_data);
  calculateStdDev(&gate_2_data);
  calculateStdDev(&gate_diffs);

  Serial.print(gate_1_data.n);
  Serial.print(" -- 1| mean: ");
  Serial.print(gate_1_data.mean);
  Serial.print(" sd: ");
  Serial.print(gate_1_data.sd);
  // Serial.print(" max: ");
  // Serial.print(gate_1_data.max);
  // Serial.print(" min: ");
  // Serial.print(gate_1_data.min);
  Serial.print(" -- 2| mean: ");
  Serial.print(gate_2_data.mean);
  Serial.print(" sd: ");
  Serial.print(gate_2_data.sd);
  // Serial.print(" max: ");
  // Serial.print(gate_2_data.max);
  // Serial.print(" min: ");
  // Serial.print(gate_2_data.min);
  Serial.print(" -- diffs| mean: ");
  Serial.print(gate_diffs.mean);
  Serial.print(" sd: ");
  Serial.print(gate_diffs.sd);
  // Serial.print(" max: ");
  // Serial.print(gate_diffs.max);
  // Serial.print(" min: ");
  // Serial.print(gate_diffs.min);
  Serial.print(" -- offset: ");
  gate_time_diff_offset = gate_1_data.mean - gate_2_data.mean;
  Serial.print(gate_time_diff_offset);
  Serial.print(" corr: ");
  Serial.println(gate_diffs.mean + gate_time_diff_offset);

  if(gate1_long > 0 || gate1_fail > 0) {
    Serial.print("gate 1 failures: ");
    Serial.print(gate1_fail);
    Serial.print(" delayed response: ");
    Serial.println(gate1_long);
  }
  if(gate2_long > 0 || gate2_fail > 0) {
    Serial.print("gate 2 failures: ");
    Serial.print(gate2_fail);
    Serial.print(" delayed response: ");
    Serial.println(gate2_long);
  }
  ir_led_1_on();
  ir_led_2_on();
  if(gate_1_data.sd > 2 || gate_1_data.n < 10 ||
     gate_2_data.sd > 2 || gate_2_data.n < 10 || 
     gate_diffs.sd > 10) {
      Serial.println("hardware test failed");
      return false;
  }
  return true;
}
//write control led high, wait for a bit
//write control led low, record start time
//wait for flag (or timeout)
//calculate time and store
//repeat x times

//complete test again with second sensor
//delete task

int get_gate_offset() {
  return gate_time_diff_offset;
}

bool timer_test(uint16_t timeout_us) {
  Serial.println("calibrating");
  hardware_test();
  //if(hardware_test()) {
    ir_led_1_on();
    ir_led_2_on();
    delay(1);
    reset_gate_triggers();
    ir_led_1_off();
    int64_t test_start = esp_timer_get_time();
    delayMicroseconds(timeout_us);
    ir_led_2_off();
    int64_t test_end = esp_timer_get_time();
    delay(1);
  
    int64_t gate_time_1 = get_gate_1_time();
    int64_t gate_time_2 = get_gate_2_time();
    int64_t diff_microseconds_actual = differenceMicroseconds(test_start, test_end);
    int64_t diff_microseconds_measured = differenceMicroseconds(gate_time_1, gate_time_2);
    Serial.print(gate_time_1);
    Serial.print(" ");
    Serial.println(gate_time_2);
    Serial.print("timer test: ");
    Serial.print(timeout_us/1000.0);
    Serial.print(" ms  ");
    Serial.print("actual time taken for test: ");
    Serial.print(diff_microseconds_actual / 1000.0);
    Serial.print(" ms ");
    Serial.print(diff_microseconds_actual);
    Serial.println(" us");
    Serial.print("measured time (sensor response time): ");
    Serial.print((diff_microseconds_measured - gate_time_diff_offset)/ 1000.0);
    Serial.print("ms ");
    Serial.print((diff_microseconds_measured - gate_time_diff_offset));
    Serial.println(" us");

    ir_led_1_on();
    ir_led_2_on();
    reset_gate_triggers();
  //} else {
  //  Serial.println("timer test hardware calibration failed");
  //}
  return false;
}