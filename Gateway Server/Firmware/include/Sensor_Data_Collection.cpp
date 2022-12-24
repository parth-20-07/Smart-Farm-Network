#include "Arduino.h"

/* -------------------------------------------------------------------------- */
/*                              BASIC DEFINITION                              */
/* -------------------------------------------------------------------------- */
#define HTU21D_TEMP_HUMIDITY_SENSOR // Reference: https://learn.sparkfun.com/tutorials/htu21d-humidity-sensor-hookup-guide

#include <Wire.h>
#include "HTU21D.h"
HTU21D temperature_and_humidity_sensor; // Create an instance of the object

/* -------------------------------------------------------------------------- */
/*                             FUNCTION DEFINITION                            */
/* -------------------------------------------------------------------------- */
void setup_sensors(void)
{
    Serial.println("Setting up Temperature and Humidity Sensor");
    temperature_and_humidity_sensor.begin();
}

float read_onboard_temperature(void)
{
    Serial.println("Reading HTU21D");
    float temp = temperature_and_humidity_sensor.readTemperature();
    Serial.println("Temperature: " + (String)temp + "C");
    return temp;
}

uint8_t read_onboard_humidity(void)
{
    uint8_t humi = temperature_and_humidity_sensor.readHumidity();
    Serial.println("Humidity: " + (String)humi + "%");
    return humi;
}
