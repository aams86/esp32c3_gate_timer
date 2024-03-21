#include <esp_timer.h>  // Make sure to include this for esp_timer_get_time()
#include <Arduino.h>

#define TOGGLE_PIN1   0  // Pin to toggle in ISR
#define TOGGLE_PIN2   1  // Pin to toggle in ISR
#define CONTROL_PIN1   2 // Pin to control based on button press
#define CONTROL_PIN2   3 // Pin to control based on button press
#define INPUT_PIN1     7   // Button input pin
#define INPUT_PIN2     6   // Button input pin


typedef enum timer_state_t {
  TIMER_IDLE,
  TIMER_GATE1_TRIGGERED,
  TIMER_GATE1_TRIGGERED_TIMEOUT,
  TIMER_GATE2_TRIGGERED,
  TIMER_CALCULATE_TIME
} timer_state_t;


volatile bool toggleState = false;
volatile int64_t gate1_triggered_time = 0;
volatile int64_t gate2_triggered_time = 0;
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
  toggleState = !toggleState;  // Toggle the state
  digitalWrite(TOGGLE_PIN1, toggleState); // Toggle the pin
  digitalWrite(TOGGLE_PIN2, !toggleState); // Toggle the pin
}

void IRAM_ATTR handleGate1() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  // Note: INPUT_PIN state is read immediately after the button press; debouncing is not handled here.
  gate1_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN1);
  digitalWrite(CONTROL_PIN1, inputState);
  
}

void IRAM_ATTR handleGate2() {
  // Directly set CONTROL_PIN to match the INPUT_PIN state
  // Note: INPUT_PIN state is read immediately after the button press; debouncing is not handled here.
  gate2_triggered_time = esp_timer_get_time();
  bool inputState = digitalRead(INPUT_PIN2);
  digitalWrite(CONTROL_PIN2, inputState);
  
}

// Calculate the difference in microseconds
int64_t differenceMicroseconds(int64_t start, int64_t end) {
  return end - start;
}

// Calculate the difference in ms
float differenceMilliseconds(int64_t start, int64_t end) {
  return (float)differenceMicroseconds(start, end) / 1000.0;
}

// Calculate the difference in ms
float differenceSeconds(int64_t start, int64_t end) {
  return differenceMilliseconds(start, end) / 1000.0;
}

void setup() {
  // Initialize the TOGGLE_PIN as an output
  pinMode(TOGGLE_PIN1, OUTPUT);
  pinMode(TOGGLE_PIN2, OUTPUT);
  // Initialize the CONTROL_PIN as an output
  pinMode(CONTROL_PIN1, OUTPUT);
  pinMode(CONTROL_PIN2, OUTPUT);
  // Initialize the INPUT_PIN as an input with pullup
  pinMode(INPUT_PIN1, INPUT_PULLUP);
  pinMode(INPUT_PIN2, INPUT_PULLUP);
  // Attach interrupt to handle button press
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN1), handleGate1, CHANGE); // Use CHANGE to trigger on both press and release
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN2), handleGate2, CHANGE); // Use CHANGE to trigger on both press and release

  // Set up timer
  timer = timerBegin(0, 80, true); // Timer 0, prescaler 80, counting up
  timerAttachInterrupt(timer, &onTimer, true); // Attach onTimer function
  timerAlarmWrite(timer, 1000000, true); // Fire the interrupt every 1 second (1e6 microseconds)
  timerAlarmEnable(timer); // Enable the alarm
}

void loop() {
  // The loop can remain empty if no additional logic is required here.
  delay(100); // Small delay to keep the loop under control
}
