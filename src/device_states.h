#ifndef DEVICE_STATES_H
#define DEVICE_STATES_H


typedef enum device_state_t {
  DEVICE_IDLE = 0,                  //0
  DEVICE_RESET,                     //1
  DEVICE_HARDWARE_TEST,             //2
  DEVICE_READY,                     //3
  DEVICE_MEASUREMENT_IN_PROGRESS,   //4
  DEVICE_MEASUREMENT_COMPLETED,     //5
  DEVICE_MEASUREMENT_VALID,         //6
  DEVICE_MEASUREMENT_INVALID,       //7
  DEVICE_MEASUREMENT_TIMEOUT        //8
} device_state_t;

typedef enum timer_state_t {
  TIMER_IDLE,
  TIMER_GATE1_TRIGGERED,
  TIMER_GATE1_TRIGGERED_TIMEOUT,
  TIMER_GATE2_TRIGGERED,
  TIMER_CALCULATE_TIME
} timer_state_t;

void update_device_state(device_state_t new_state);
device_state_t get_device_state();
void update_timer_state(timer_state_t new_state);
timer_state_t get_timer_state();


#endif