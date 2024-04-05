#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H

#include "Arduino.h"

#define GATE_SPEED_SERVICE_UUID         "8360008c-e79f-4618-be03-0f4561d11800"
#define GATE_SPEED_NOTIFY_UUID          "8360008c-e79f-4618-be03-0f4561d11801"
#define GATE_CALIBRATE_TEST_START_UUID  "8360008c-e79f-4618-be03-0f4561d11802"
/*
#define SPEED_SERVICE_UUID                  "46eca9b1-96e9-4c6b-a52f-197444917300"
#define NOTIFY_SPEED_UUID                   "46eca9b1-96e9-4c6b-a52f-197444917301"
#define INTIATE_HARDWARE_CALIBRATION_TEST   "46eca9b1-96e9-4c6b-a52f-197444917301"
*/
/*
#define SERVICE_UUID              "fb1e4001-54ae-4a28-9f74-dfccb248601d"
#define CHARACTERISTIC_UUID_RX    "fb1e4002-54ae-4a28-9f74-dfccb248601d"
#define CHARACTERISTIC_UUID_TX    "fb1e4003-54ae-4a28-9f74-dfccb248601d"
*/

#define BUILTINLED      2
#define NORMAL_MODE   0   // normal
#define UPDATE_MODE   1   // receiving firmware
#define OTA_MODE      2   // installing firmware


void sendOtaResult(String result);
void initBLE();
void BLE_loop();
void notify_speed(float speed);

#endif

