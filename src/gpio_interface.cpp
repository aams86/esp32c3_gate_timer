#include "gpio_interface.h"
#include "Arduino.h"

//#define HARDWARE_TOGGLE_DISPLAY

#ifdef HARDWARE_TOGGLE_DISPLAY
volatile bool toggleState = false;
DRAM_ATTR volatile bool gate_1_flag;
DRAM_ATTR volatile bool gate_2_flag;
DRAM_ATTR volatile int64_t gate1_triggered_time = 0;
DRAM_ATTR volatile int64_t gate2_triggered_time = 0;
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
  toggleState = !toggleState;  // Toggle the state
  digitalWrite(IR_LED_PIN1, toggleState); // Toggle the pin
  digitalWrite(IR_LED_PIN2, !toggleState); // Toggle the pin
}

void IRAM_ATTR handleGate1() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  gate1_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN1);
  digitalWrite(INDICATOR_LED_PIN1, inputState);
  
}

void IRAM_ATTR handleGate2() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  gate2_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN2);
  digitalWrite(INDICATOR_LED_PIN2, inputState);
  
}



void init_gpio(void) {
    // Initialize the TOGGLE_PIN as an output
  pinMode(INDICATOR_LED_PIN1, OUTPUT);
  pinMode(INDICATOR_LED_PIN2, OUTPUT);
  // Initialize the CONTROL_PIN as an output
  pinMode(IR_LED_PIN1, OUTPUT);
  pinMode(IR_LED_PIN2, OUTPUT);
  // Initialize the INPUT_PIN as an input with pullup
  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);
  // Attach interrupt to handle button press
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN1), handleGate1, CHANGE); // Use CHANGE to trigger on both press and release
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN2), handleGate2, CHANGE); // Use CHANGE to trigger on both press and release
    // Set up timer
  timer = timerBegin(0, 80, true); // Timer 0, prescaler 80, counting up
  timerAttachInterrupt(timer, &onTimer, true); // Attach onTimer function
  timerAlarmWrite(timer, 1000000, true); // Fire the interrupt every 1 second (1e6 microseconds)
  timerAlarmEnable(timer); // Enable the alarm

}
#else
volatile bool toggleState = false;
DRAM_ATTR volatile bool gate_1_flag = false;
DRAM_ATTR volatile bool gate_2_flag = false;
DRAM_ATTR volatile bool gate_1_int_en = false;
DRAM_ATTR volatile bool gate_2_int_en = false;
DRAM_ATTR volatile int64_t gate1_triggered_time = 0;
DRAM_ATTR volatile int64_t gate2_triggered_time = 0;
hw_timer_t * timer = NULL;

void IRAM_ATTR disable_gate_1_interrupt() {
  if(gate_1_int_en) {
    gate_1_int_en = false;
    detachInterrupt(digitalPinToInterrupt(INPUT_PIN1)); // Use CHANGE to trigger on both press and release
  }
}

void IRAM_ATTR disable_gate_2_interrupt() {
  if(gate_2_int_en) {
    gate_2_int_en = false;
    detachInterrupt(digitalPinToInterrupt(INPUT_PIN2)); // Use CHANGE to trigger on both press and release
  }
}


void IRAM_ATTR handleGate1() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  gate1_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN1);
  digitalWrite(INDICATOR_LED_PIN1, inputState);
  disable_gate_1_interrupt();
  
}

void IRAM_ATTR handleGate2() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  gate2_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN2);

  disable_gate_2_interrupt();
}


void enable_interrupts() {
  // Attach interrupt to handle button press
  if(!gate_1_int_en) {
    gate_1_int_en = true;
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN1), handleGate1, RISING); // Use CHANGE to trigger on both press and release
  }
  if(!gate_2_int_en) {
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN2), handleGate2, RISING); // Use CHANGE to trigger on both press and release
  }

}

void disable_interrupts() {
  // Attach interrupt to handle button press
  disable_gate_1_interrupt();
  disable_gate_2_interrupt();
}



void init_gpio(void) {
    // Initialize the TOGGLE_PIN as an output
  pinMode(INDICATOR_LED_PIN1, OUTPUT);
  pinMode(INDICATOR_LED_PIN2, OUTPUT);
  // Initialize the CONTROL_PIN as an output
  pinMode(IR_LED_PIN1, OUTPUT);
  pinMode(IR_LED_PIN2, OUTPUT);
  // Initialize the INPUT_PIN as an input with pullup
  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);
  // Attach interrupt to handle button press
  enable_interrupts();

}

void ir_led_1_on() {
  digitalWrite(IR_LED_PIN1, HIGH);
}
void ir_led_1_off() {
  digitalWrite(IR_LED_PIN1, LOW);
}

void ir_led_2_on() {
  digitalWrite(IR_LED_PIN2, HIGH);
}
void ir_led_2_off() {
  digitalWrite(IR_LED_PIN2, LOW);
}

void reset_gate_triggers() {
  enable_interrupts();
  delay(10);
  gate1_triggered_time = 0;
  gate2_triggered_time = 0;
}

int64_t get_gate_1_time() {
  return gate1_triggered_time;
}

int64_t get_gate_2_time() {
  return gate2_triggered_time;
}

bool get_gate_1_status() {
  return digitalRead(INPUT_PIN1);
}
bool get_gate_2_status() {
  return digitalRead(INPUT_PIN2);
}

void log_gate_status() {
  Serial.print("gate 1 ");
  Serial.print(get_gate_1_status());
  Serial.print(" gate 2 ");
  Serial.println(get_gate_2_status());
}

uint8_t check_flags() {
  uint8_t count = 0;
  if(gate1_triggered_time > 0) {
    count++;
  }
  if(gate2_triggered_time > 0) {
    count++;
  }
  return count;
}


#endif