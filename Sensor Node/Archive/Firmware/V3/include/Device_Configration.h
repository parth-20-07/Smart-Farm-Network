#ifndef Device_Configuration_h
#define Device_Configuration_h

/* -------------------------------------------------------------------------- */
/*                         BASIC DEVICE CONFIGURATION                         */
/* -------------------------------------------------------------------------- */
String DEVICE_NAME = "NODE - CITY GREENS";
#define BLE_UPDATE_TIME_COMMAND "time"   // Command recieved over ble to update time
#define BLE_UPDATE_SYNC_COMMAND "sync"   // Command to sync backlog data
#define BLE_UPDATE_SLEEP_COMMAND "sleep" // Command to go to sleep

/* -------------------------------------------------------------------------- */
/*                            SENSOR CONFIGURATION                            */
/* -------------------------------------------------------------------------- */
#define HTU21D_TEMP_HUMIDITY_SENSOR // Reference: https://learn.sparkfun.com/tutorials/htu21d-humidity-sensor-hookup-guide
#define MHZ19C_CO2_SENSOR           // Reference: https://github.com/WifWaf/MH-Z19/blob/master/examples/BasicUsage/BasicUsage.ino
#define TSL2561_LUX_SENSOR          // Reference: https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library/blob/master/examples/SparkFunTSL2561Example/SparkFunTSL2561Example.ino

/* -------------------------------------------------------------------------- */
/*                              RTC CONFIGURATION                             */
/* -------------------------------------------------------------------------- */
// #define DS3231_RTC
#define PCF8563_RTC

#endif