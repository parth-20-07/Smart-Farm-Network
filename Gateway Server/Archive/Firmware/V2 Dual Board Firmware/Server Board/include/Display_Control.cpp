#ifndef Display_Control_cpp
#define Display_Control_cpp
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Variable_Declaration.h"
#include "Pin_Connection.h"

/* ---------------------------- BASIC DEFINITION ---------------------------- */
HardwareSerial nextionSerial(2);
#define BATTERY_ADC_PIN BATTERY_INPUT      // Battery Voltage connected to pin
#define MIN_BATT_ADC 1026                  // ADC Value at 3.3V
#define MAX_BATT_ADC 2042                  // ADC Value at 4.2V
#define SUPPLY_CONNECTED_PIN CHARGER_INPUT // Power Supply connected to pin
#define GREEN 34784
#define ORANGE 64520
#define RED 63488
#define WHITE 65535
bool display_first_boot = true;

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void setup_display(void);
void load_boot_screen(void);
void load_configuration_screen(void);
void update_display(bool wifi_status, bool internet_status, bool temp_alert, bool humidity_alert, bool lux_alert, bool co2_alert, String date, String time, uint8_t average_temp, uint8_t average_humidity, uint16_t average_lux, uint16_t average_co2, bool node_connection_status);

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

    float adjusted_battery_level = (analogRead(BATTERY_ADC_PIN) - MIN_BATT_ADC) / (MAX_BATT_ADC - MIN_BATT_ADC);
    uint8_t battery_level = ((uint8_t)adjusted_battery_level) * 100;
    bool charger_status = digitalRead(SUPPLY_CONNECTED_PIN) == HIGH;
    update_data_on_screen("bat_level", "val", battery_level, "");

    if (battery_level > 50)
        update_data_on_screen("bat_level", "pco", GREEN, "");
    else if (battery_level > 20)
        update_data_on_screen("bat_level", "pco", ORANGE, "");
    else
        update_data_on_screen("bat_level", "pco", RED, "");

    if (charger_status)
        update_data_on_screen("supply_pin", "pic", 3, "");
    else
        update_data_on_screen("supply_pin", "pic", 4, "");
}

void set_wifi_internet_flag(bool wifi_status, bool internet_status)
{
    if (internet_status)
        update_data_on_screen("wifi_status", "pic", 7, "");
    else if (wifi_status)
        update_data_on_screen("wifi_status", "pic", 5, "");
    else
        update_data_on_screen("wifi_status", "pic", 6, "");
}

void change_screen(uint8_t page_num)
{
    String page_msg = "page " + (String)page_num;
    nextionSerial.print(page_msg);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
}

/**
 * @brief Boot Screen which shows company Logo
 */
void load_boot_screen(void)
{
    Serial.println("Loading Bootup Page");
    change_screen(0);
}

/**
 * @brief Screen to show all the sensor values, Date, Time, WiFi and battery status
 */
void load_main_data_screen(void)
{
    Serial.println("Loading Main Display Page");
    change_screen(1);
}

/**
 * @brief Load the configuration page which shows SSID, Password and Web Address to connect
 */
void load_configuration_screen(void)
{
    Serial.println("Loading Configuration Page");
    change_screen(2);
}

void load_firmware_upgrade_screen(void)
{
    Serial.println("Loading Firmware Upgrade Page");
    change_screen(3);
}

void load_alert_data_on_configuration_screen(void)
{
    update_data_on_screen("ssid", "txt", 0, (String)ssid);
    update_data_on_screen("password", "txt", 0, (String)password);
    update_data_on_screen("min_temp", "val", params_range[0], "");
    update_data_on_screen("max_temp", "val", params_range[1], "");
    update_data_on_screen("min_humi", "val", params_range[2], "");
    update_data_on_screen("max_humi", "val", params_range[3], "");
    update_data_on_screen("min_lux", "val", params_range[4], "");
    update_data_on_screen("max_lux", "val", params_range[5], "");
    update_data_on_screen("min_co2", "val", params_range[6], "");
    update_data_on_screen("max_co2", "val", params_range[7], "");
}

/**
 * @brief Set the up display object
 */
void setup_display(void)
{
    pinMode(BATTERY_ADC_PIN, INPUT);
    pinMode(SUPPLY_CONNECTED_PIN, INPUT);

    nextionSerial.begin(9600);
    load_boot_screen();
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
        if (msg == "restart")
        {
            load_boot_screen();
            delay(10);
            ESP.restart();
        }
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
void update_display(bool wifi_status, bool internet_status, bool temp_alert, bool humidity_alert, bool lux_alert, bool co2_alert, String date, String time, uint8_t average_temp, uint8_t average_humidity, uint16_t average_lux, uint16_t average_co2, bool node_connection_status)
{
    Serial.println("============================================================");
    load_main_data_screen();
    Serial.println("Updating Display");
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
    if (node_connection_status)
        update_data_on_screen("node_status", "bco", GREEN, ""); // Create Alert
    else
        update_data_on_screen("node_status", "bco", WHITE, ""); // Create Alert
    Serial.println("Display Update Complete");
    Serial.println("============================================================");
    return;
}

#endif