// Reference (RTC DS3231): https://how2electronics.com/esp32-ds3231-based-real-time-clock/
// Reference (RTCLib Library): //https://adafruit.github.io/RTClib/html/index.html/
#ifndef Time_Configuration_cpp
#define Time_Configuration_cpp
#include "Arduino.h"
#include <SPI.h>
#include "RTClib.h"
#include "Variable_Declaration.h"
#include <Wire.h>

/* -------------------------------------------------------------------------- */
/*                              BASIC DEFINITION                              */
/* -------------------------------------------------------------------------- */
RTC_DS3231 rtc;
const char *ntpServer = "asia.pool.ntp.org";
const long gmtOffset_sec = 19800; // +5:30 = (5*60*60) + (30*60) = 19800
const int daylightOffset_sec = 0; // India doesn't observe DayLight Saving

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void connect_to_ntp(void);
void rtc_setup(void);
void printLocalTime(void);
void rtc_get_time(void);

/* -------------------------- FUNCTION DECLARATION -------------------------- */
void connect_to_ntp(void)
{
    Serial.println("Connecting to NTP");
    if (internet_connection_status)
    {
        Serial.println("Setting up NTP Server");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Configure NTP
        printLocalTime();                                         // Fetching time from NTP
    }
    else
        rtc_get_time(); // Fetching time from RTC
    return;
}

/**
 * @brief Setup RTC
 */
void rtc_setup(void)
{
    Serial.println("Connecting to RTC");
    delay(10);
    Wire.begin();
    if (!rtc.begin())
        Serial.println("RTC Connection Failed");
    rtc.disable32K();
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    rtc.writeSqwPinMode(DS3231_OFF);
    rtc.disableAlarm(1);
    rtc.disableAlarm(2);
    Serial.println("RTC Connection Successful");
    return;
}

/**
 * @brief Tries to get local time from NTP:
 * -> If it's successful, The year, month, date, hour, minutes, seconds is updated to global variables with time from NTP,
 * and the time is saved to RTC.
 * -> If it fails, the fuction is called rtc_get_time()
 */
void printLocalTime(void)
{
    if (internet_connection_status)
    {
        Serial.println("Updating Time from NTP");
        uint16_t year;
        uint8_t month, date, hour, minutes, seconds;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("NTP Failed to Update");
            rtc_get_time();
            return;
        }
        year = timeinfo.tm_year - 100 + 2000;
        month = timeinfo.tm_mon + 1;
        date = timeinfo.tm_mday;
        hour = timeinfo.tm_hour;
        minutes = timeinfo.tm_min;
        seconds = timeinfo.tm_sec;
        Serial.println("NTP Update Successful");
        live_date = (String)year + "/" + (String)month + "/" + (String)date;
        live_time = (String)hour + ":" + (String)minutes;
        Serial.println("Time from NTP: " + live_date + ' ' + live_time);
        rtc.adjust(DateTime(year, month, date, hour, minutes, seconds)); // this will adjust to the date and time at compilation
        return;
    }
    else
        rtc_get_time();
}

/**
 * @brief Fetch Time from RTC and save it in global time variables
 */
void rtc_get_time(void)
{
    Serial.println("Fetching Time from RTC");
    DateTime now = rtc.now();
    uint16_t year;
    uint8_t month, date, hour, minutes, seconds;
    year = now.year();
    month = now.month();
    date = now.day();
    hour = now.hour();
    minutes = now.minute();
    seconds = now.second();
    live_date = (String)year + "/" + (String)month + "/" + (String)date;
    live_time = (String)hour + ":" + (String)minutes;
    Serial.println("Time from RTC: " + live_date + ' ' + live_time);
    delay(10);
    return;
}

#endif // Time_Configuration_cpp
