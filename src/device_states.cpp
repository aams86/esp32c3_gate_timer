#include "device_states.h"
#include "Arduino.h"

device_state_t device_state = DEVICE_IDLE;
DRAM_ATTR volatile timer_state_t timer_state = TIMER_IDLE;

void log_device_states(device_state_t old_state, device_state_t new_state) {
    Serial.print("switching from state: ");
    Serial.print(old_state);
    Serial.print(" to state: ");
    Serial.println(new_state);
}

void update_device_state(device_state_t new_state) {
    //log_device_states(device_state, new_state);
    device_state = new_state;
}

device_state_t get_device_state() {
    return device_state;
}

void IRAM_ATTR update_timer_state(timer_state_t new_state) {
    timer_state = new_state;
}

timer_state_t get_timer_state() {
    return timer_state;
}
