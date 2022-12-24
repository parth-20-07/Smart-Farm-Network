#ifndef Battery_Level_cpp
#define Battery_Level_cpp
#include <Arduino.h>
#include <Pin_Connection.h>

/* ------------------------ BASIC VARIABLE DEFINITION ----------------------- */
#define BATTERY_ADC_PIN BATTERY_INPUT // Battery Voltage connected to pin
#define MIN_BATT_ADC 1026             // ADC Value at Min Battery
float MIN_BATT_VAL = 2.6;             // Min Battery Value
#define MAX_BATT_ADC 2042             // ADC Value at Max Battery
float MAX_BATT_VAL = 3.1;             // Max Battery Value

uint8_t read_battery_level(void)
{
    uint16_t battery_value = analogRead(BATTERY_INPUT);
    float bat_percentage = (battery_value - MIN_BATT_ADC) / (MAX_BATT_ADC - MIN_BATT_ADC);
    bat_percentage = bat_percentage * 100;
    uint8_t battery_voltage_level = (uint8_t)bat_percentage;

    if (battery_voltage_level >= 100)
        battery_voltage_level = 100;
    else if (battery_voltage_level <= 0)
        battery_voltage_level = 1;

    return battery_voltage_level;
}

#endif // Battery_Level_cpp