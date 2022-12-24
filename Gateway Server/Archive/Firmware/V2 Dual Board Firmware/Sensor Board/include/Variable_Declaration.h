#ifndef Variable_Declaration_h
#define Variable_Declaration_h
#include "Arduino.h"

/* ------------------------ ONBOARD SENSOR VARIABLES ------------------------ */
uint32_t last_millis;
uint32_t ONBOARD_SENSOR_DATA_REFRESH_TIME = 30000; // Refresh time of 30 sec
bool first_boot = true;

/* ------------------------------ BLE VARIABLES ----------------------------- */
#define BLE_DEVICE_NAME "ESP32 Gateway Reciever"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "aca5483e-36e1-4688-b7f5-ea07361b26a7"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
bool recieved_data_flag = false;
String incoming_ble_msg;
uint32_t last_ble_recieved_message;
bool nodes_connected = false;

/* --------------------- SERIAL COMMUNICATION VARIABLES --------------------- */
bool server_intitated_pause = false;

#endif