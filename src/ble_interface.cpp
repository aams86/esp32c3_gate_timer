
#include "Arduino.h"
#include "ble_interface.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "gpio_interface.h"

static BLECharacteristic* pCharacteristicTX;
static BLECharacteristic* pCharacteristicRX;
static BLECharacteristic* pCharacteristicNotifySpeed;
static BLECharacteristic* pCharacteristicCompleteHardwareTest;

static bool deviceConnected = false, sendMode = false, sendSize = true;
static bool writeFile = false, request = false;
static int writeLen = 0, writeLen2 = 0;
static bool current = true;
static int parts = 0, next = 0, cur = 0, MTU = 0;
static int MODE = NORMAL_MODE;
unsigned long rParts, tParts;



void start_advertising();

typedef enum ble_state_t {
  BLE_IDLE,
  BLE_ADVERTISING,
  BLE_CONNECTED,
  BLE_DISCONNECTED
} ble_state_t;

ble_state_t BLE_STATE = BLE_IDLE;

unsigned long adv_start;
unsigned long adv_length = 60000;
uint16_t toggle_length_on = 100;
uint16_t toggle_length_off = 3000;


void notify_speed(float speed) {
  if(BLE_STATE == BLE_CONNECTED) {
    uint16_t output = (uint16_t)(speed * 100);
    pCharacteristicNotifySpeed->setValue(output);
    pCharacteristicNotifySpeed->notify();
  }
}

// Define the desired MTU size
#define DESIRED_MTU 517
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLE_STATE = BLE_CONNECTED;
      Serial.print("ble connected");
      digitalWrite(LED_INDICATOR_PIN, HIGH);

        // Request a change to the desired MTU upon connection
        pServer->getConnId(); // Get the connection ID if needed

    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.print("ble disconnected");
      BLE_STATE = BLE_DISCONNECTED;
      digitalWrite(LED_INDICATOR_PIN, LOW);
    }

    void onMTUChange(uint16_t mtu, uint16_t connId) {
        Serial.println("MTU size changed to " + String(mtu));
    }
};

uint32_t reverse_32bit_int(uint32_t start) {
    uint32_t reversed = 0;

    reversed |= (start & 0x000000FF) << 24; // Move first byte to last
    reversed |= (start & 0x0000FF00) << 8;  // Move second byte to third
    reversed |= (start & 0x00FF0000) >> 8;  // Move third byte to second
    reversed |= (start & 0xFF000000) >> 24; // Move last byte to first
    return reversed;
}

class MyCallbacks: public BLECharacteristicCallbacks {

    //    void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) {
    //      Serial.print("Status ");
    //      Serial.print(s);
    //      Serial.print(" on characteristic ");
    //      Serial.print(pCharacteristic->getUUID().toString().c_str());
    //      Serial.print(" with code ");
    //      Serial.println(code);
    //    }

    void onNotify(BLECharacteristic *pCharacteristic) {
      uint8_t* pData;
      std::string value = pCharacteristic->getValue();
      int len = value.length();
      pData = pCharacteristic->getData();
      if (pData != NULL) {
        //        Serial.print("Notify callback for characteristic ");
        //        Serial.print(pCharacteristic->getUUID().toString().c_str());
        //        Serial.print(" of data length ");
        //        Serial.println(len);
        Serial.print("TX  ");
        for (int i = 0; i < len; i++) {
          Serial.printf("%02X ", pData[i]);
        }
        Serial.println();
      }
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData;
      std::string value = pCharacteristic->getValue();
      int len = value.length();
      pData = pCharacteristic->getData();
      if (pData != NULL) {
        // Get the UUID of the characteristic that was written to
        BLEUUID uuid = pCharacteristic->getUUID();

        // Compare this UUID with the UUIDs of known characteristics
        if (uuid.equals(BLEUUID(GATE_CALIBRATE_TEST_START_UUID))) {
          //writeToUpdatePartition( pData, len);
            Serial.println("write to calibrate test start: ");
            Serial.println(pData[0]);

        } else {
            Serial.println("unhandled bluetooth write: ");
            Serial.println(pData[0]);   
        }
        // Serial.print("length: ");
        // Serial.println(len);
      }

    }
};



void init_speed_service(BLEServer *pServer) {
  BLEService *pService = pServer->createService(GATE_SPEED_SERVICE_UUID);
  pCharacteristicNotifySpeed = pService->createCharacteristic(GATE_SPEED_NOTIFY_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pCharacteristicCompleteHardwareTest = pService->createCharacteristic(GATE_CALIBRATE_TEST_START_UUID, BLECharacteristic::PROPERTY_WRITE );

  pCharacteristicNotifySpeed->setCallbacks(new MyCallbacks());
  pCharacteristicNotifySpeed->addDescriptor(new BLE2902());
  pCharacteristicNotifySpeed->setNotifyProperty(true);
  pService->start();

}

void initBLE() {
  pinMode(LED_INDICATOR_PIN, OUTPUT);

  BLEDevice::init("Timer");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
/*
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY );
  pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);

  pCharacteristicRX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->addDescriptor(new BLE2902());
  pCharacteristicTX->setNotifyProperty(true);
  
  pService->start();
  */

  init_speed_service(pServer);
  BLEDevice::setMTU(DESIRED_MTU);
 
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(GATE_SPEED_SERVICE_UUID);
  //pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(6);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(12);

  start_advertising();
}


void start_advertising() {
  adv_start = millis();
  Serial.println("advertising start");
  BLE_STATE = BLE_ADVERTISING;
  BLEDevice::startAdvertising();
}

void stop_advertising() {
  BLE_STATE = BLE_IDLE;
  Serial.println("advertising stop");
  BLEDevice::stopAdvertising();
  digitalWrite(LED_INDICATOR_PIN, LOW);
}


void toggle_led() {
  static bool led_state = false;
  static unsigned long last_toggle = 0;
  unsigned long current_time = millis();
  if(led_state) {
    if(current_time - last_toggle > toggle_length_off) {
      last_toggle = current_time;
      digitalWrite(LED_INDICATOR_PIN, true);
      led_state = false;
    }
  } else {
    if(current_time - last_toggle > toggle_length_on) {
      last_toggle = current_time;
      digitalWrite(LED_INDICATOR_PIN, false);
      led_state = true;
    }
  }
}

void BLE_loop() {
  switch(BLE_STATE) {
    case BLE_ADVERTISING:
      toggle_led();
      break;
    case BLE_CONNECTED:
      break;
    case BLE_DISCONNECTED:
      start_advertising();
      break;
    case BLE_IDLE:
      break;
    default:
      break;
  }

//   if(update_ready()) {
//     rebootEspWithReason("completed update");
//   }
}