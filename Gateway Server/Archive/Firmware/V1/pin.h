/*|---------------------------------------------------------------------------------------|
                     Insterion of TAB Function.h for all the Functions
  |---------------------------------------------------------------------------------------|*/

#ifndef pin_h
#define pin_h
#include"Arduino.h"
#endif

/*|---------------------------------------------------------------------------------------|
                                      Libraries Used
  |---------------------------------------------------------------------------------------|*/

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <ThingsBoard.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Wire.h>
#include <Rtc_Pcf8563.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "time.h"
#include <analogWrite.h>

/*|---------------------------------------------------------------------------------------|
                                      Defination
  |---------------------------------------------------------------------------------------|*/

SoftwareSerial softSerial(25, 26); //rx tx
WiFiClient espClient;
ThingsBoard tb(espClient);
//Rtc_Pcf8563 rtc;

IPAddress myIP;
WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)
AsyncWebServer sr(80);
int status = WL_IDLE_STATUS;

/*|---------------------------------------------------------------------------------------|
                          Credentials -- [NEED UPDATE CONFIGURABLE]
  |---------------------------------------------------------------------------------------|*/

int sizes = 0;    //Size of SSID
int sizep = 0;    //Size of Password
int sizei = 0;    //Size of Interval
int sizeu = 0;    //Size of UUID

String sssid;
String pass;
String interval;
String uuuid;

const char* sta_ssid        = "project105";                     //Station mode credentials
const char* sta_password    = "project105";


static bool hasWifi = false;

const char* ap_ssid         = "ESP32";                     //Acess Point Mode credentials
const char* ap_password     = "password";

#define TOKEN "ClFUJlTWVqZBWBXjx9CB"                       //Cloud credentials
char thingsboardServer[] = "139.59.60.58";

/*|---------------------------------------------------------------------------------------|
                                      Node Variables
  |---------------------------------------------------------------------------------------|*/

int nodeID;
float temperature;
float humidity;
float VPD;
float batt_level;

int ST_AC;
int ST_DE;

/*|---------------------------------------------------------------------------------------|
                             Configuration Variables (HTML PAGE)
  |---------------------------------------------------------------------------------------|*/

//String sssid;
//String pass;
//String interval;
String temp_min;
String temp_max;
String hum_min;
String hum_max;
String co2_min;
String co2_max;
String par_min;
String par_max;
String vpd_min;
String vpd_max;

String fan_on;
String fan_off;
String pad_on;
String pad_off;
String fogger_on;
String fogger_off;
String dehum_on;
String dehum_off;
String cirFan_on;
String cirFan_off;
String ex_fan_on;
String ex_fan_off;
int count = 0;

float config_data[19];

/* Variable to be Written On EEPROM of ESP */
int INTERVAL;      // INTERVAL FOR EEPROM
int TEMP_MIN;      //Minimum Temperature
int TEMP_MAX;      //Maximum Temperature
int HUMI_MIN;      //Minimum Humidity
int HUMI_MAX;      //Maximum Humidity
int CO2_MIN;       //Minimum CO2
int CO2_MAX;       //Maximum CO2
int PAR_MIN;       //Minimum PAR
int PAR_MAX;       //Maximum PAR
int VPD_MIN;       //Minimum VPD
int VPD_MAX;       //Maximum VPD

//Automation Thresholds
int FAN_ON;       //ON Fan Pad when greater than temperature
int FAN_OFF;      //OFF Fan Pad when less than temperature
int PAD_ON;       //ON  Pad when greater than temperature
int PAD_OFF;      //OFF Pad when less than temperature
int FOG_ON;       //ON Fogger when less than humidity
int FOG_OFF;      //OFF Fogger when greater than humidity
int DEHUM_ON;     //ON Dehumidifier when greater than humidity
int DEHUM_OFF;    //OFF Dehumidifier when less than humidity
int CIR_FAN_ON;   //ON Circular Fan when greater than temperature
int CIR_FAN_OFF;  //OFF Circular Fan when less than temperature
int EX_FAN_ON;    // ON timer exhaust fan when greater than temperature
int EX_FAN_OFF;   // Off timer exhaust fan when greater than temperature

/*|---------------------------------------------------------------------------------------|
                                      PIN DECLARATIONS
  |---------------------------------------------------------------------------------------|*/

#define RED   24
#define GREEN 25
#define BLUE  27
#define RST_PIN 13
#define INTR_PIN 2

/*|---------------------------------------------------------------------------------------|
                                  Conditional Variables [MISC]
  |---------------------------------------------------------------------------------------|*/

int addr = 18;
int intr_check = 0;
int j;
int k;
int end_addr;
long int presentTime;

int red_pin = 34;
int green_pin = 35;
int blue_pin = 32; 



int delayTime;
int red_light_value;
int green_light_value; 
int blue_light_value; 


/*|-----------------------------------------------------------------------------|
                              Renaming of Fuctions
  |-----------------------------------------------------------------------------|*/


StaticJsonDocument <2000> doc;

/*|---------------------------------------------------------------------------------------|
                                  END OF PIN.H
  |---------------------------------------------------------------------------------------|*/
