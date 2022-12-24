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
bool aws_file_exists = false;

/* ----------------------------- MICROSD DETAILS ---------------------------- */
#define AWS_BACKLOG_FILE "/AWS_Backlog_file.txt"
#define SSID_FILE "/ssid.txt"
#define PASSWORD_FILE "/password.txt"
#define FIRMWARE_VERSION_FILE "/firmware_version.txt"
#define THINGNAME_FILE "/thingname.txt"
#define AWS_IOT_PUBLISH_TOPIC_FILE "/pub.txt"
#define AWS_IOT_SUBSCRIBE_TOPIC_FILE "/sub.txt"
#define AWS_IOT_ENDPOINT_FILE "/aws_endpoint.txt"
#define URL_FW_VERSION_FILE "/Version_File.txt" // Github link for the firmware version (.txt) file
#define URL_FW_BIN "/Version_File_Link.txt"     // Github link for the firmware (.bin) file
String variable_str_val PROGMEM;

/* ------------------------------ SENSOR VALUES ----------------------------- */
#define MAX_NODES 10
float database_temp[MAX_NODES];
uint8_t database_humiditiy[MAX_NODES];
uint16_t database_lux[MAX_NODES];
uint16_t database_co2[MAX_NODES];
uint8_t temp_values = 0, humidity_values = 0, lux_values = 0, co2_values = 0;
#define NUMBER_OF_PARAMS 8
uint8_t config_index = 0;
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
*/

/* ----------------------------- DISPLAY DETAILS ---------------------------- */
#define DISPLAY_UPDATE_TIME 300000 // Update interval is 5 mins for display
uint32_t lastMillis;
uint32_t last_millis_time_update;
bool temp_alert = false;
bool humidity_alert = false;
bool lux_alert = false;
bool co2_alert = false;

/* ------------------------------ TIME DETAILS ------------------------------ */
String live_date, live_time;

/* ------------------------- DEPARSE MESSAGE DETAILS ------------------------ */
bool calculated_first_average = false;
bool device_connection_status = false;
String cmd, node_type, nodeid, rec_date, rec_time;
float temp;
uint8_t humiditiy;
uint16_t lux, co2, battery_percentage;

/* ----------------------------- DEVICE DETAILS ----------------------------- */
String firmware_version;

/* ------------------------------- AWS DETAILS ------------------------------ */
char THINGNAME[50];
String AWS_IOT_PUBLISH_TOPIC;
String AWS_IOT_SUBSCRIBE_TOPIC;
char AWS_IOT_ENDPOINT[100];
bool aws_setup_flag = false;
uint32_t last_aws_upload;
/* ------------------------------- OTA DETAILS ------------------------------ */
String URL_fw_Version;
String URL_fw_Bin;

#endif