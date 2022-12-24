#ifndef Display_Control_cpp
#define Display_Control_cpp
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Variable_Declaration.h"
#include "Pin_Connection.h"
#include <Battery_Level.cpp>

/* ---------------------------- BASIC DEFINITION ---------------------------- */
HardwareSerial nextionSerial(2);
#define SUPPLY_CONNECTED_PIN CHARGER_INPUT // Power Supply connected to pin
#define GREEN 34784
#define ORANGE 64520
#define RED 63488
bool display_first_boot = true;

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void setup_display(void);
void update_display(bool wifi_status, bool internet_status, bool temp_alert, bool humidity_alert, bool lux_alert, bool co2_alert, String date, String time, uint8_t average_temp, uint8_t average_humidity, uint16_t average_lux, uint16_t average_co2);

/* -------------------------- FUNCTION DECLARATION -------------------------- */
void update_data_on_screen(String param, String cmd, int cmd_value = 0, String cmd_string = "")
{
    String write_message;
    if (cmd_value != 0)
        write_message = param + "." + cmd + "=" + cmd_value;
    else if (cmd_string != "")
        write_message = param + "." + cmd + "=" + "\"" + cmd_string + "\"";
    nextionSerial.print(write_message);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    delay(10);
    Serial.println(param + "." + cmd + "=" + cmd_value + " " + cmd_string);
}

void update_alert_flags(bool alert_mode, String param)
{
    if (alert_mode)
        update_data_on_screen(param, "bco", RED, ""); // Create Alert
    else
        update_data_on_screen(param, "bco", GREEN, ""); // No Alert
}

void set_battery_level(void)
{
    uint8_t battery_level = read_battery_level();
    bool charger_status = digitalRead(SUPPLY_CONNECTED_PIN);
    update_data_on_screen("bat_level", "val", battery_level, "");

    if (battery_level >= 50)
        update_data_on_screen("bat_level", "pco", GREEN, "");
    else if (battery_level >= 20)
        update_data_on_screen("bat_level", "pco", ORANGE, "");
    else
        update_data_on_screen("bat_level", "pco", RED, "");

    if (charger_status)
        update_data_on_screen("supply_pin", "pic", SUPPLY_CONNECTED_ID, "");
    else
        update_data_on_screen("supply_pin", "pic", NO_SUPPLY_CONNECTED_ID, "");
}

void set_wifi_internet_flag(bool wifi_status, bool internet_status)
{
    if (internet_status)
        update_data_on_screen("wifi_status", "pic", INTERNET_CONNECTED_FLAG_ID, "");
    else if (wifi_status)
        update_data_on_screen("wifi_status", "pic", NO_INTERNET_FLAG_CONNECTED_ID, "");
    else
        update_data_on_screen("wifi_status", "pic", NO_WIFI_CONNECTED_FLAG_ID, "");
}

void change_screen(String page_name)
{
    String page_msg = "page " + page_name;
    Serial.println("Screen: " + page_msg);
    nextionSerial.print(page_msg);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
}

/**
 * @brief Set the up display object
 */
void setup_display(void)
{
    nextionSerial.begin(9600);
    Serial.println("Loading Boot Screen");
    change_screen("boot_screen");
    Serial.println("Display Setup Complete");
}

String read_nextion_input_message(void)
{
    uint32_t last_millis = millis();
    String input_msg, msg;
    if ((millis() - last_millis) < 1000)
        if (nextionSerial.available())
            input_msg = nextionSerial.readStringUntil('#');
    if (input_msg != "")
    {
        Serial.println("Nextion Message: " + input_msg);
        char token = ',';
        for (size_t i = 0; i < input_msg.length(); i++)
            if (input_msg[i] == token)
            {
                msg = input_msg.substring(i + 1);
                break;
            }
        Serial.println("Deparsed Message: " + msg);
    }
    return msg;
}

/**
 * @brief Call this function when all the data has been collected from sensors
 * @param wifi_status true -> WiFi Connected, false -> WiFi Not Connected
 * @param internet_status true -> Internet Connected, false -> Internet Not Connected
 * @param temp_alert true -> Average Temperature in Range, false -> Average Temperature not in Range
 * @param humidity_alert true -> Average Humidity in Range, false -> Average Humidity not in Range
 * @param lux_alert true -> Average Lux in Range, false -> Average Lux not in Range
 * @param co2_alert true -> Average CO2 in Range, false -> Average CO2 not in Range
 * @param date Date in format: DD/MM/YY
 * @param time Time in format: HH:MM
 * @param average_temp Average Temperature from all nodes and onboard sensor
 * @param average_humidity Average Humidity from all nodes and onboard sensor
 * @param average_lux Average Lux from all nodes and onboard sensor
 * @param average_co2 Average CO2 from all nodes and onboard sensor
 */
void update_display(bool wifi_status, bool internet_status, bool temp_alert, bool humidity_alert, bool lux_alert, bool co2_alert, String date, String time, uint8_t average_temp, uint8_t average_humidity, uint16_t average_lux, uint16_t average_co2)
{
    set_battery_level();                                        // Setting Battery Level and Power Supply Flag
    set_wifi_internet_flag(wifi_status, internet_status);       // Setting WiFi and Internet Flags
    update_alert_flags(temp_alert, "temp_alert");               // Setting Temperature Alerts
    update_alert_flags(humidity_alert, "humi_alert");           // Setting Humidity Alerts
    update_alert_flags(lux_alert, "lux_alert");                 // Setting Lux Alerts
    update_alert_flags(co2_alert, "co2_alert");                 // Setting CO2 Alerts
    update_data_on_screen("date", "txt", 0, date);              // Setting Date
    update_data_on_screen("time", "txt", 0, time);              // Setting Time
    update_data_on_screen("temp", "val", average_temp, "");     // Average Temp
    update_data_on_screen("humi", "val", average_humidity, ""); // Average Humidity
    update_data_on_screen("lux", "val", average_lux, "");       // Average Lux Value
    update_data_on_screen("co2", "val", average_co2, "");       // Average CO2
    Serial.println("Display Update Complete");
    Serial.println("============================================================");
    return;
}

#endif