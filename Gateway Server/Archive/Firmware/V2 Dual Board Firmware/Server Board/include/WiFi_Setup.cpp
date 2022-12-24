#ifndef WiFi_Setup_cpp
#define WiFi_Setup_cpp

#include "Arduino.h"
#include "WiFi.h"
#include "Variable_Declaration.h"
#include <WiFi.h>
#include <ESP32Ping.h>
const IPAddress remote_ip(192, 168, 0, 1);
/* --------------------------- FUNCTION DEFINITION -------------------------- */

/**
 * @brief Setup WiFi and NTP
 */
bool connect_to_wifi(void)
{
    Serial.println("Connecting to Wi-Fi");
    delay(10);

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int connection_attempts = 0;
    int max_attempts = 50;
    while (connection_attempts < max_attempts)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            delay(100);
            Serial.print(".");
            connection_attempts++;
        }
        else
        {
            Serial.println("\nWi-Fi Connected!");
            wifi_connection_status = true;
            internet_connection_status = Ping.ping("www.google.com");
            if (internet_connection_status)
                Serial.println("Internet Connected");
            else
                Serial.println("Internet Disconnected");
            break;
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        wifi_connection_status = false;
        internet_connection_status = false;
        Serial.println("\nWifi Not Connected to SSID: " + (String)ssid);
        return false;
    }
    else
        return true;
}

#endif