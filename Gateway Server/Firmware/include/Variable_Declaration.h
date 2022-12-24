#ifndef Variable_Declaration_h
#define Variable_Declaration_h
#include "Arduino.h"

/* ------------------------------ WIFI DETAILS ------------------------------ */
#define SSID_CHAR_LENGTH 50
#define PASSWORD_CHAR_LENGTH 50
char ssid[SSID_CHAR_LENGTH];
char password[SSID_CHAR_LENGTH];
bool wifi_connection_status = false;
bool internet_connection_status = false;
#define WEBSERVER_SSID "Gateway"
#define WEBSERVER_PASSWORD "123456789"
uint32_t last_internet_refresh;
bool digital_ocean_file_exists = false;

/* ----------------------------- MICROSD DETAILS ---------------------------- */
#define DIGITAL_OCEAN_BACKLOG_FILE "/log1.txt"
#define DIGITAL_OCEAN_BACKLOG_FILE_2 "/log2.txt"
#define SSID_FILE "/ssid.txt"
#define PASSWORD_FILE "/password.txt"

/* ------------------------------ SENSOR VALUES ----------------------------- */
#define MAX_NODES 10
float database_temp[MAX_NODES];
uint8_t database_humiditiy[MAX_NODES];
uint16_t database_lux[MAX_NODES];
uint16_t database_co2[MAX_NODES];
uint8_t temp_values = 0, humidity_values = 0, lux_values = 0, co2_values = 0;
#define NUMBER_OF_PARAMS 16
uint16_t params_range[NUMBER_OF_PARAMS];
/*
Temp Min,
Temp Max,
Humidity Min,
Humidity Max,
Lux Min,
Lux Max,
CO2 Min,
CO2 Max
Fan Pad Min,
Fan Pad Max,
Fogger Min,
Fogger Max,
Dehumidifier Min,
Dehumidifier Max,
Circular Fan Min,
Circular Fan Max
*/
bool first_boot = true;

/* ----------------------------- DISPLAY DETAILS ---------------------------- */
#define DISPLAY_UPDATE_TIME 300000 // Update interval is 5 mins for display
uint32_t lastMillis;
uint32_t last_millis;
uint32_t last_millis_time_update;
bool temp_alert = false;
bool humidity_alert = false;
bool lux_alert = false;
bool co2_alert = false;
#define SUPPLY_CONNECTED_ID 16
#define NO_SUPPLY_CONNECTED_ID 8
#define NO_INTERNET_FLAG_CONNECTED_ID 9
#define NO_WIFI_CONNECTED_FLAG_ID 10
#define INTERNET_CONNECTED_FLAG_ID 11

/* ------------------------------ TIME DETAILS ------------------------------ */
String live_date, live_time;
uint16_t year;
uint8_t month, date, hour, minutes, seconds;

/* ------------------------- DEPARSE MESSAGE DETAILS ------------------------ */
bool calculated_first_average = false;
String node_type, nodeid, rec_date, rec_time;
float temp;
uint8_t humiditiy;
uint16_t lux, co2, battery_percentage;

/* -------------------------- DIGITAL OCEAN DETAILS ------------------------- */
bool mqtt_setup_flag = false;
uint32_t last_digital_ocean_upload;
#define DIGITAL_OCEAN_UPLOAD_TIME_IN_MINS 5
#define DIGITAL_OCEAN_MQTT_PUBLISH_CONFIRMATION_TIME_IN_SECONDS 5000
bool mqtt_publish_confirmation = false;
#define MQTT_HOST IPAddress(178, 62, 225, 6) // Digital Ocean MQTT Mosquitto Broker
#define MQTT_USERNAME "Citygreens_Gateway"
#define MQTT_PASSWORD "citygreens@123"
#define MQTT_PUB_TEST "test"

/* ------------------------------- OTA DETAILS ------------------------------ */
String firmware_version = {"2.8"};
#define URL_fw_Version "https://raw.githubusercontent.com/Onboardlogic/CityGreen-Gateway-Firmware-File/main/fw_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/Onboardlogic/CityGreen-Gateway-Firmware-File/main/firmware.bin"

/* ------------------------ ONBOARD SENSOR VARIABLES ------------------------ */
uint32_t last_sensor_fetch_millis;
#define ONBOARD_SENSOR_DATA_REFRESH_TIME_IN_MINS 5
/* ------------------------------ BLE VARIABLES ----------------------------- */
#define BLE_DEVICE_NAME "ESP32 Gateway Reciever"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "aca5483e-36e1-4688-b7f5-ea07361b26a7"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_SCAN_TIME_IN_SECONDS 20
String incoming_ble_msg;
bool ble_msg_recieve_complete = false;
uint32_t ble_msg_wait_millis;
String incoming_ble_message_list[1000];
uint16_t msg_list_number = 0;
#endif