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
MHZ19 carbondioxide_sensor(&MHZ19CSerial); // Constructor for library
#endif

#ifdef BH1750_LUX_SENSOR
#include <Wire.h>
#include <BH1750.h>
BH1750 light_sensor; // Create an BH1750 object
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
    node_type += "C";
#endif

#ifdef BH1750_LUX_SENSOR
    Serial.println("Setting up Lux Sensor");
    Wire.begin();
    light_sensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
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
    MHZ19_RESULT response = carbondioxide_sensor.retrieveData();
    if (response == MHZ19_RESULT_OK)
    {
        uint16_t CO2 = carbondioxide_sensor.getCO2();
        Serial.println("CO2: " + (String)CO2 + "ppm");
        return (",C:" + (String)CO2);
    }
    else
    {
        Serial.print(F("Error, code: "));
        Serial.println(response);
        return "";
    }
#endif
}

String collect_lux_values(void)
{
#ifdef BH1750_LUX_SENSOR
    Serial.println("Reading  BH1750");
    double lux = light_sensor.readLightLevel();
    Serial.println("LUX: " + (String)lux + "lx");
#endif
    return (",L:" + (String)lux);
}

#endif