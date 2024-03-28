#ifndef GPIO_INTERFACE_H
#define GPIO_INTERFACE_H

#include "Arduino.h"

#define INDICATOR_LED_PIN1   2  // Pin to toggle in ISR
#define INDICATOR_LED_PIN2   3  // Pin to toggle in ISR
#define IR_LED_PIN1    0 // Pin to control based on button press
#define IR_LED_PIN2    1 // Pin to control based on button press
#define INPUT_PIN1     6   // Button input pin
#define INPUT_PIN2     7   // Button input pin

void init_gpio(void);

void ir_led_1_on();
void ir_led_1_off();

void ir_led_2_on();
void ir_led_2_off();

void reset_gate_triggers();

int64_t get_gate_1_time();
int64_t get_gate_2_time();

bool get_gate_1_status();
bool get_gate_2_status();

void log_gate_status();

uint8_t check_flags();

#endif
