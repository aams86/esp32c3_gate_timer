#ifndef HARDWARE_TEST_H
#define HARDWARE_TEST_H

void init_hardware_test();
bool hardware_test();
int get_gate_offset();
bool timer_test(uint16_t timeout_us);
#endif