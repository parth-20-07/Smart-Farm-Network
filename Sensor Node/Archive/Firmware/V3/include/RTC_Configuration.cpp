#ifndef RTC_Configuration_cpp
#define RTC_Configuration_cpp
#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"

#ifdef DS3231_RTC
RTC_DS3231 rtc;
#endif

#ifdef PCF8563_RTC
RTC_PCF8563 rtc;
#endif

/* -------------------------------------------------------------------------- */
/*                             FUNCTION DEFINITION                            */
/* -------------------------------------------------------------------------- */
void setup_rtc(void);
void set_time(String time_string);
String fetch_formatted_time(void);

/* -------------------------------------------------------------------------- */
/*                            FUNCTION DECLARATION                            */
/* -------------------------------------------------------------------------- */
void setup_rtc(void)
{
    uint8_t i = 0;
    while (!rtc.begin() && i < 5)
    {
        Serial.println("Couldn't find RTC");
        return;
    }
    rtc.start();
    Serial.println("RTC Setup Complete");
}

void set_time(String time_string)
{
    Serial.println("=====================================================================");
    uint8_t hour, minutes, seconds, date, month;
    int year;
    char token = '/';
    uint8_t time_string_length = time_string.length();
    String time_params[6];
    int j = 0, k = 0;
    for (size_t i = 0; i < time_string_length; i++)
    {
        if (time_string[i] == token)
        {
            time_params[j] = time_string.substring(k, i);
            k = i + 1;
            j++;
        }
    }
    time_params[j] = time_string.substring(k++);
    for (size_t i = 0; i < 6; i++)
        Serial.println("Time Params: " + time_params[i]);

    char char_time_params[6][6];
    for (size_t i = 0; i < 6; i++)
        strcpy(char_time_params[i], time_params[i].c_str());
    for (size_t i = 0; i < 6; i++)
        Serial.println("Char Time Params: " + (String)char_time_params[i]);

    year = atoi(char_time_params[0]);
    month = atoi(char_time_params[1]);
    date = atoi(char_time_params[2]);
    hour = atoi(char_time_params[3]);
    minutes = atoi(char_time_params[4]);
    seconds = atoi(char_time_params[5]);
    Serial.println("Setting Formatted Date: " + (String)year + '-' + (String)month + '-' + (String)date);
    Serial.println("Setting Formatted Time: " + (String)hour + ':' + (String)minutes + ':' + (String)seconds);
    Serial.println("Updating Time for RTC");

    rtc.adjust(DateTime(year, month, date, hour, minutes, seconds));

    Serial.println("Verifying Update");
    fetch_formatted_time();
    Serial.println("Update Process Complete");
    Serial.println("=====================================================================");
}

String fetch_formatted_time(void)
{
    Serial.println("Fetching Time from RTC");

    DateTime now = rtc.now();
    String time = (String)now.hour() + '-' + (String)now.minute() + '-' + (String)now.second();
    String date = (String)now.year() + '-' + (String)now.month() + '-' + (String)now.day();
    Serial.println("Fetched Formatted Date: " + date);
    Serial.println("Fetched Formatted Time: " + time);
    String msg = "D:" + date + ',' + "G:" + time;

    return msg;
}

#endif