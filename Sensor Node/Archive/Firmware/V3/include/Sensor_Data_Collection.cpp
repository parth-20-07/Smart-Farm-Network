#ifndef Sensor_Data_Collection_cpp
#define Sensor_Data_Collection_cpp

#include "Arduino.h"
#include "Device_Configration.h"
#include "Pin_Connection.h"
#include "Variable_Declaration.h"

/* -------------------------------------------------------------------------- */
/*                              BASIC DEFINITION                              */
/* -------------------------------------------------------------------------- */
#ifdef HTU21D_TEMP_HUMIDITY_SENSOR
#include <Wire.h>
#include "HTU21D.h"
HTU21D temperature_and_humidity_sensor; // Create an instance of the object
#endif

#ifdef MHZ19C_CO2_SENSOR
#include "MHZ19.h"
#include "HardwareSerial.h"
HardwareSerial MHZ19CSerial(2);
MHZ19 carbondioxide_sensor; // Constructor for library
#endif

#ifdef TSL2561_LUX_SENSOR
#include <Wire.h>
#include <SparkFunTSL2561.h>
SFE_TSL2561 light_sensor; // Create an SFE_TSL2561 object
// Global variables:
boolean gain;    // Gain setting, 0 = X1, 1 = X16;
unsigned int ms; // Integration ("shutter") time in milliseconds
#endif

/* -------------------------------------------------------------------------- */
/*                             FUNCTION DEFINITION                            */
/* -------------------------------------------------------------------------- */
String setup_sensors(void)
{
    Serial.println("Setting up Sensors");
#ifdef HTU21D_TEMP_HUMIDITY_SENSOR
    Serial.println("Setting up Temperature and Humidity Sensor");
    temperature_and_humidity_sensor.begin();
    node_type += "TH";
#endif

#ifdef MHZ19C_CO2_SENSOR
    Serial.println("Setting up CO2 Sensor");
    MHZ19CSerial.begin(9600);
    carbondioxide_sensor.begin(MHZ19CSerial);
    carbondioxide_sensor.autoCalibration();
    node_type += "C";
#endif

#ifdef TSL2561_LUX_SENSOR
    Serial.println("Setting up Lux Sensor");
    // Initialize the SFE_TSL2561 library
    // You can pass nothing to light.begin() for the default I2C address (0x39),
    // or use one of the following presets if you have changed
    // the ADDR jumper on the board:
    // TSL2561_ADDR_0 address with '0' shorted on board (0x29)
    // TSL2561_ADDR   default address (0x39)
    // TSL2561_ADDR_1 address with '1' shorted on board (0x49)
    light_sensor.begin();
    // The light sensor has a default integration time of 402ms,
    // and a default gain of low (1X).
    // If you would like to change either of these, you can
    // do so using the setTiming() command.
    // If gain = false (0), device is set to low gain (1X)
    // If gain = high (1), device is set to high gain (16X)
    gain = 0;
    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    // If time = 3, use manual start / stop to perform your own integration
    unsigned char time = 2;
    // setTiming() will set the third parameter (ms) to the requested integration time in ms (this will be useful later):
    Serial.println("Set timing...");
    light_sensor.setTiming(gain, time, ms);
    Serial.println("Powerup..."); // To start taking measurements, power up the sensor
    light_sensor.setPowerUp();
    node_type += "L";
#endif
    return node_type;
}

String collect_temperature_and_humidity_values(void)
{
#ifdef HTU21D_TEMP_HUMIDITY_SENSOR
    Serial.println("Reading HTU21D");
    uint8_t humd = temperature_and_humidity_sensor.readHumidity();
    uint8_t temp = temperature_and_humidity_sensor.readTemperature();
    Serial.println("Temperature: " + (String)temp + "C");
    Serial.println("Humidity: " + (String)humd + "%");
    return (",T:" + (String)temp + ",H:" + (String)humd);
#endif
}

String collect_co2_values(void)
{
#ifdef MHZ19C_CO2_SENSOR
    Serial.println("Reading MHZ19C");
    uint16_t CO2 = carbondioxide_sensor.getCO2();
    Serial.println("CO2: " + (String)CO2 + "ppm");
    return (",C:" + (String)CO2);
#endif
}

String collect_lux_values(void)
{
#ifdef TSL2561_LUX_SENSOR
    Serial.println("Reading TSL2561");
    double lux;
    unsigned int data0, data1;
    if (light_sensor.getData(data0, data1)) // getData() returned true, communication was successful
    {
        Serial.print("data0: ");
        Serial.print(data0);
        Serial.print(" data1: ");
        Serial.println(data1);
        boolean good;                                            // True if neither sensor is saturated
        good = light_sensor.getLux(gain, ms, data0, data1, lux); // Perform lux calculation:
        Serial.print("Lux: " + (String)lux);                     // Print out the results:
        if (good)
            Serial.println(" | GOOD");
        else
            Serial.println(" | BAD");
    }
#endif
    return (",L:" + (String)lux);
}

#endif