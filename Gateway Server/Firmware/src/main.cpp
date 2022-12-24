/* -------------------------------------------------------------------------- */
/*                               BASIC LIBRARIES                              */
/* -------------------------------------------------------------------------- */
#include <Arduino.h>
#include "Pin_Connection.h"
#include "Variable_Declaration.h"
#include <pgmspace.h>
#include <ESP32Ping.h>

/* -------------------------------------------------------------------------- */
/*                                CUSTOM FILES                                */
/* -------------------------------------------------------------------------- */
#include "WiFi_Setup.cpp"
#include "MicroSD_Control.cpp"
#include "Display_Control.cpp"
#include "Time_Configuration.cpp"
#include "OTA_Configuration.cpp"
#include "Digital_Ocean_Connection.cpp"
#include "esp32_webserver.cpp"
#include "Sensor_Data_Collection.cpp"
#include "NimBLE_Communication.cpp"
#include "Variable_Declaration.h"

/* -------------------------------------------------------------------------- */
/*                              BASIC DEFINITION                              */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                             FUNCTION DEFINITION                            */
/* -------------------------------------------------------------------------- */
void read_digital_ocean_backlog_file(void);
void deparse_msg(String msg);
void calculate_averages_and_update_display(void);
void load_configuration_server(void);
void fetch_configuration_data(void);
void data_processing_operation(void);
void basic_operation(void);

/* -------------------------------------------------------------------------- */
/*                            FUNCTION DECLARATION                            */
/* -------------------------------------------------------------------------- */

/* ---------------------------- SD CARD FUNCTIONS --------------------------- */

void read_digital_ocean_backlog_file(void)
{
  Serial.println("Reading Data Backlog");
  change_screen("upload_screen");
  disconnect_from_mqtt();
  delay(500);
  connect_to_wifi();
  if (internet_connection_status)
  {
    if (!mqtt_setup_flag)
      mqtt_setup();

    connectToMqtt();
    delay(5000); // TODO: 5 sec delay for mqtt to connect
    Serial.println("Reading Backlog Files");

    {
      // Read Backlog 2 File
      File printFile = SD.open(DIGITAL_OCEAN_BACKLOG_FILE_2);
      bool complete_file_read = true;
      if (!printFile) // No digital_ocean File
        Serial.print("The text file cannot be opened");
      else // digital_ocean File Exists
      {
        Serial.println("Reading Backlog File 2");
        int lineIndex = 1;
        while (printFile.available())
        {
          String send_string = printFile.readStringUntil('\n');
          Serial.println((String)lineIndex + " : " + send_string);
          deparse_msg((String)send_string);
          sendData("node", rec_date, rec_time, nodeid, node_type, temp, humiditiy, lux, co2, battery_percentage);
          uint32_t mqtt_upload_millis = millis();
          while (((millis() - mqtt_upload_millis) < (DIGITAL_OCEAN_UPLOAD_TIME_IN_MINS * 60000)) && !mqtt_publish_confirmation)
            delay(1);
          if (mqtt_publish_confirmation)
            Serial.println("Publish Success!");
          else
          {
            Serial.println("Publish Failure!");
            complete_file_read = false;
            break;
          }
          lineIndex++;
        }
        printFile.close();
        if (complete_file_read)
          deleteFile(SD, DIGITAL_OCEAN_BACKLOG_FILE_2);
      }
    }

    {
      // Read Main Backlog File
      File printFile = SD.open(DIGITAL_OCEAN_BACKLOG_FILE);
      if (!printFile) // No digital_ocean File
      {
        Serial.print("The text file cannot be opened");
        digital_ocean_file_exists = false;
      }
      else // digital_ocean File Exists
      {
        Serial.println("Reading Backlog File 1");
        int lineIndex = 1;
        while (printFile.available())
        {
          String send_string = printFile.readStringUntil('\n');
          Serial.println((String)lineIndex + " : " + send_string);
          deparse_msg((String)send_string);
          sendData("node", rec_date, rec_time, nodeid, node_type, temp, humiditiy, lux, co2, battery_percentage);
          uint32_t mqtt_upload_millis = millis();
          while (((millis() - mqtt_upload_millis) < (DIGITAL_OCEAN_UPLOAD_TIME_IN_MINS * 60000)) && !mqtt_publish_confirmation)
            delay(1);
          if (mqtt_publish_confirmation)
            Serial.println("Publish Success!");
          else
          {
            Serial.println("Publish Failure!");
            appendFile(SD, DIGITAL_OCEAN_BACKLOG_FILE_2, send_string);
          }
          lineIndex++;
        }
        printFile.close();
        deleteFile(SD, DIGITAL_OCEAN_BACKLOG_FILE);
        digital_ocean_file_exists = false;
      }
    }
  }
  change_screen("main_screen");
  live_time = (String)hour + ":" + (String)minutes;
  update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, temp, humiditiy, lux, co2);
  delay(50);
  return;
}

/* ------------------------ DATA PROCESSING FUNCTION ------------------------ */
void deparse_msg(String deparse_msg)
{
  // Seperating the csv string
  String cmd = deparse_msg.substring(0, 4);
  if (cmd == "hist")
  {
    String msg = deparse_msg.substring(5);
    appendFile(SD, DIGITAL_OCEAN_BACKLOG_FILE, msg);
    digital_ocean_file_exists = true;
    return;
  }

  String msg = "";
  if (cmd == "live")
    msg = deparse_msg.substring(5);
  else
    msg = deparse_msg;

  Serial.println("Deparsing Message: " + msg);
  String params[10];
  char seperation_token = ',';
  uint8_t last_pos = 0;
  uint8_t param_index = 0;
  for (size_t i = 0; i < msg.length(); i++)
  {
    if (msg[i] == seperation_token)
    {
      params[param_index] = msg.substring(last_pos, i);
      last_pos = i + 1;
      param_index++;
    }
  }
  params[param_index] = msg.substring(last_pos);

  // Seperating the colon seperated values
  String param_values[10];
  String param_identifiers[10];
  char break_token = ':';
  for (size_t i = 0; i < param_index + 1; i++)
  {
    String parameter = params[i];
    for (size_t j = 0; j < parameter.length(); j++)
      if (parameter[j] == break_token)
      {
        param_identifiers[i] = parameter.substring(0, j);
        param_values[i] = parameter.substring(j + 1);
      }
  }

  node_type = "", nodeid = "", rec_date = "", rec_time = "", temp = 0, humiditiy = 0, lux = 0, co2 = 0, battery_percentage = 0;

  for (size_t i = 0; i < 10; i++)
  {
    if (param_identifiers[i] == "I")
      nodeid = param_values[i];
    else if (param_identifiers[i] == "D")
      rec_date = param_values[i];
    else if (param_identifiers[i] == "G")
      rec_time = param_values[i];
    else if (param_identifiers[i] == "N")
      node_type = param_values[i];
    else
    {
      char char_value[param_values[i].length()];
      strcpy(char_value, param_values[i].c_str());
      if (param_identifiers[i] == "T")
        temp = atof(char_value);
      else if (param_identifiers[i] == "H")
        humiditiy = atoi(char_value);
      else if (param_identifiers[i] == "L")
        lux = atoi(char_value);
      else if (param_identifiers[i] == "C")
        co2 = atoi(char_value);
      else if (param_identifiers[i] == "B")
        battery_percentage = atoi(char_value);
    }
  }

  if (temp != 0)
  {
    database_temp[temp_values] = temp;
    if ((database_temp[temp_values] < params_range[0]) || (database_temp[temp_values] > params_range[1]))
      temp_alert = true;
    temp_values++;
  }
  if (humiditiy != 0)
  {
    database_humiditiy[humidity_values] = humiditiy;
    if ((database_humiditiy[humidity_values] < params_range[2]) || (database_humiditiy[humidity_values] > params_range[3]))
      humidity_alert = true;
    humidity_values++;
  }
  if (lux != 0)
  {
    database_lux[lux_values] = lux;
    if ((database_lux[lux_values] < params_range[4]) || (database_lux[lux_values] > params_range[5]))
      lux_alert = true;
    lux_values++;
  }
  if (co2 != 0)
  {
    database_co2[co2_values] = co2;
    if ((database_co2[co2_values] < params_range[6]) || (database_co2[co2_values] > params_range[7]))
      co2_alert = true;
    co2_values++;
  }
  return;
}

void calculate_averages_and_update_display(void)
{
  Serial.println("Calculating Average to update display");
  calculated_first_average = true;

  double total_temperature_sum = 0;
  for (size_t i = 0; i < temp_values; i++)
    total_temperature_sum += database_temp[i];
  Serial.println("Temp Sum: " + (String)total_temperature_sum);
  Serial.println("Temp Values: " + (String)temp_values);
  uint16_t average_temp = 0;
  if (temp_values != 0)
    average_temp = uint8_t(total_temperature_sum / temp_values);
  Serial.println("Temp average: " + (String)average_temp);

  double total_humidity_sum = 0;
  for (size_t i = 0; i < humidity_values; i++)
    total_humidity_sum += database_humiditiy[i];
  Serial.println("Humidity Sum: " + (String)total_humidity_sum);
  Serial.println("Humidity Values: " + (String)humidity_values);
  uint16_t average_humidity = 0;
  if (humidity_values != 0)
    average_humidity = uint8_t(total_humidity_sum / humidity_values);
  Serial.println("Humidity average: " + (String)average_humidity);

  double total_lux_sum = 0;
  for (size_t i = 0; i < lux_values; i++)
    total_lux_sum += database_lux[i];
  Serial.println("Lux Sum: " + (String)total_lux_sum);
  Serial.println("Lux Values: " + (String)lux_values);
  uint16_t average_lux = 0;
  if (lux_values != 0)
    average_lux = uint16_t(total_lux_sum / lux_values);
  Serial.println("Lux average: " + (String)average_lux);

  double total_co2_sum = 0;
  for (size_t i = 0; i < co2_values; i++)
    total_co2_sum += database_co2[i];
  Serial.println("CO2 Sum: " + (String)total_co2_sum);
  Serial.println("CO2 Values: " + (String)co2_values);
  uint16_t average_co2 = 0;
  if (co2_values != 0)
    average_co2 = uint16_t(total_co2_sum / co2_values);
  Serial.println("CO2 average: " + (String)average_co2);
  connect_to_wifi();
  if (internet_connection_status)
  {
    if (!mqtt_setup_flag)
      mqtt_setup();
    connectToMqtt();
    sendData("average", live_date, live_time, "", "", average_temp, average_humidity, average_lux, average_co2, 0);
  }
  live_time = (String)hour + ":" + (String)minutes;
  update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, average_temp, average_humidity, average_lux, average_co2);
  temp_values = 0, humidity_values = 0, lux_values = 0, co2_values = 0, temp_alert = false, humidity_alert = false, lux_alert = false, co2_alert = false;
  memset(database_temp, 0, MAX_NODES);
  memset(database_humiditiy, 0, MAX_NODES);
  memset(database_lux, 0, MAX_NODES);
  memset(database_co2, 0, MAX_NODES);
  lastMillis = millis();
}

/* --------------------------- WEBSERVER FUNCTION --------------------------- */
void load_configuration_server(void)
{
  Serial.println("Loading Configuration Screen");
  change_screen("conf_screen"); // Load Configuration screen on display
  delay(2000);
  launch_webserver();
  Serial.println("Saving Configuration Data");
  change_screen("save_screen");
  Serial.println("Saving Data from Webserver");
  Serial.println("--------------------------------");
  writeFile(SD, SSID_FILE, (String)ssid);
  Serial.println("--------------------------------");
  Serial.println("--------------------------------");
  writeFile(SD, PASSWORD_FILE, (String)password);
  Serial.println("--------------------------------");
  for (size_t i = 0; i < NUMBER_OF_PARAMS; i++)
  {
    Serial.println("--------------------------------");
    String str_path = "/" + (String)i + ".txt";
    writeFile(SD, str_path, (String)params_range[i]);
    Serial.println("--------------------------------");
    delay(10);
  }
  Serial.println("Restarting Device");
  delay(10);
  ESP.restart();
}

/* ------------------------ DEVICE STARTUP FUNCTIONS ------------------------ */

void fetch_configuration_data(void)
{
  String fetched_ssid = read_single_line_data_from_sd_card(SSID_FILE); // Retrieving SSID
  strcpy(ssid, fetched_ssid.c_str());
  String fetched_password = read_single_line_data_from_sd_card(PASSWORD_FILE); // Retrieving Password
  strcpy(password, fetched_password.c_str());

  for (size_t i = 0; i < NUMBER_OF_PARAMS; i++) // Retrieving Parameter Range Values
  {
    String str_path = "/" + (String)i + ".txt";
    String file_value = read_single_line_data_from_sd_card(str_path);
    char file_value_char[file_value.length()];
    strcpy(file_value_char, file_value.c_str());
    params_range[i] = atoi(file_value_char);
    delay(10);
  }

  Serial.println("Firmware Version: " + (String)firmware_version);
  Serial.println("Firmware Version Link: " + (String)URL_fw_Version);
  Serial.println("Firmware File Link: " + (String)URL_fw_Bin);
  Serial.println("Read Complete");
}

void data_processing_operation(void)
{
  if ((msg_list_number > 0) && ble_msg_recieve_complete) // Read any message stored in array during BLE Scan
  {
    Serial.println("================================================================");
    Serial.println("Processing Incoming Data");
    for (size_t i = 0; i < msg_list_number; i++)
    {
      deparse_msg(incoming_ble_message_list[i]);
      String str_msg = incoming_ble_message_list[i].substring(5);
      appendFile(SD, DIGITAL_OCEAN_BACKLOG_FILE, str_msg);
      digital_ocean_file_exists = true;
      if (!calculated_first_average && (incoming_ble_message_list[i].substring(4) == "live"))
      {
        live_time = (String)hour + ":" + (String)minutes;
        update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, temp, humiditiy, lux, co2);
      }
      delay(100);

      for (size_t j = 0; j < sizeof(incoming_ble_message_list[i]); j++)
        incoming_ble_message_list[i][j] = 0;
    }
    msg_list_number = 0;
    ble_msg_recieve_complete = false;
  }
}

void basic_operation(void)
{
  // Serial.println("Raw Battery ADC: " + (String)analogRead(BATTERY_ADC_PIN));// TODO: Uncomment this line when battery ADC needs to be measured.

  if (digitalRead(CONFIGURATION_BUTTON) == HIGH) // To Load Configuration Screen
    load_configuration_server();

  if (((millis() - last_sensor_fetch_millis) > (ONBOARD_SENSOR_DATA_REFRESH_TIME_IN_MINS * 60000)) || (first_boot)) // Reading onboard sensor data
  {
    Serial.println("================================");
    first_boot = false;
    Serial.println("Fetching onboard Sensor Values");
    String msg = "live";
    rtc_get_time();
    msg += ",N:TH,I:00000"; // Starting to write message
    msg += ",D:" + live_date + ",G:" + live_time;
    msg += ",T:" + (String)read_onboard_temperature() + ",H:" + (String)read_onboard_humidity();
    msg += ",B:" + (String)read_battery_level();
    deparse_msg(msg);
    String str_msg = msg.substring(5);
    appendFile(SD, DIGITAL_OCEAN_BACKLOG_FILE, str_msg);
    digital_ocean_file_exists = true;
    last_sensor_fetch_millis = millis();
    live_time = (String)hour + ":" + (String)minutes;
    update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, false, false, live_date, live_time, (uint8_t)read_onboard_temperature(), (uint8_t)read_onboard_humidity(), 0, 0);
    Serial.println("================================");
  }

  if (((millis() - lastMillis) > DISPLAY_UPDATE_TIME) || (temp_values == MAX_NODES - 1) || (humidity_values == MAX_NODES - 1) || (lux_values == MAX_NODES - 1) || (co2_values == MAX_NODES - 1)) // update display with new average values
    calculate_averages_and_update_display();

  if ((millis() - last_internet_refresh) > 300000) // Check Internet Connection periodically
  {
    Serial.println("================================");
    Serial.println("Checking Internet Status");
    bool internet_status = check_internet_connection_status();
    if (!internet_connection_status)
      connect_to_wifi();
    set_wifi_internet_flag(wifi_connection_status, internet_connection_status);
    last_internet_refresh = millis();
    Serial.println("================================");
  }

  if ((millis() - last_millis_time_update) > 60000) // Update time on display
  {
    Serial.println("================================");
    Serial.println("Updating Display Time");
    printLocalTime();
    update_data_on_screen("date", "txt", 0, live_date); // Setting Date
    update_data_on_screen("time", "txt", 0, live_time); // Setting Time
    last_millis_time_update = millis();
    Serial.println("================================");
  }

  String recieved_message = read_nextion_input_message(); // Check if nextion display sent any msg
  if (recieved_message == "main")
  {
    Serial.println("Loading Main Screen");
    change_screen("main_screen");
    live_time = (String)hour + ":" + (String)minutes;
    update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, temp, humiditiy, lux, co2);
  }
  else if (recieved_message == "ssid")
  {
    change_screen("ssid_screen");
    delay(500);
    update_data_on_screen("ssid", "txt", 0, (String)ssid);
    update_data_on_screen("password", "txt", 0, (String)password);
  }
  else if (recieved_message == "alerts")
  {
    change_screen("alert_screen");
    delay(500);
    update_data_on_screen("min_temp", "val", params_range[0], "");
    update_data_on_screen("max_temp", "val", params_range[1], "");
    update_data_on_screen("min_humi", "val", params_range[2], "");
    update_data_on_screen("max_humi", "val", params_range[3], "");
    update_data_on_screen("min_lux", "val", params_range[4], "");
    update_data_on_screen("max_lux", "val", params_range[5], "");
    update_data_on_screen("min_co2", "val", params_range[6], "");
    update_data_on_screen("max_co2", "val", params_range[7], "");
  }
  else if (recieved_message == "auto")
  {
    change_screen("auto_screen");
    delay(500);
    update_data_on_screen("min_fan", "val", params_range[8], "");
    update_data_on_screen("max_fan", "val", params_range[9], "");
    update_data_on_screen("min_fog", "val", params_range[10], "");
    update_data_on_screen("max_fog", "val", params_range[11], "");
    update_data_on_screen("min_deh", "val", params_range[12], "");
    update_data_on_screen("max_deh", "val", params_range[13], "");
    update_data_on_screen("min_cfan", "val", params_range[14], "");
    update_data_on_screen("max_cfan", "val", params_range[15], "");
  }
}

/* -------------------------------------------------------------------------- */
/*                                 DEVICE CODE                                */
/* -------------------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200); // open serial via USB to PC on default port
  Serial.println("\n\n=================================================");
  Serial.println("Setting Up Device");
  setup_pins();
  setup_sensors();
  setup_display();
  setup_microsd();
  fetch_configuration_data();

  if ((((String)ssid).length() == 0) || (((String)password).length() == 0))
    load_configuration_server();

  wifi_connection_status = connect_to_wifi();
  rtc_setup();

  if (internet_connection_status) // Checking for OTA
  {
    connect_to_ntp();
    if (internet_connection_status)
    {
      //   Serial.println("IP address: ");
      //   Serial.println(WiFi.localIP());
      //   bool new_firmware_version_available = false;
      //   for (size_t i = 0; i < 60; i++)
      //   {
      //     switch (check_for_new_firmware())
      //     {
      //     case 0:
      //       Serial.println("Trying again!");
      //       break;
      //     case 1:
      //       new_firmware_version_available = true;
      //       break;
      //     case 2:
      //       new_firmware_version_available = false;
      //       break;
      //     }
      //     break;
      //   }
      // if (new_firmware_version_available)//TODO: Undo Comment OTA When testing is complete
      // {
      //   Serial.println("Loading Firmware Upgrade Page");
      //   change_screen("firm_screen");
      //   update_firmware();
      //   delay(100);
      //   Serial.println("Firmware Update Complete.\n\nRestarting Device.");
      //   ESP.restart();
      // }
      // else
      mqtt_setup();
      read_digital_ocean_backlog_file(); // TODO: Enable this is digital_ocean data is high priority than BLE
    }
  }

  uint32_t millis_timer = millis();
  lastMillis = millis_timer;
  last_sensor_fetch_millis = millis_timer;
  last_internet_refresh = millis_timer;
  last_millis_time_update = millis_timer + 70000;
  last_digital_ocean_upload = millis_timer;
  Serial.println("Device Setup Complete");
  change_screen("main_screen");
  Serial.println("=================================================\n\n");
  setup_ble();
}

void loop()
{
  read_msg();
  basic_operation();
  data_processing_operation();

  if (digital_ocean_file_exists)
  {
    if ((millis() - last_digital_ocean_upload) > (DIGITAL_OCEAN_UPLOAD_TIME_IN_MINS * 60000)) // TODO: Change this to set the digital_ocean Upload Interval
    {
      stop_ble();
      data_processing_operation();
      read_digital_ocean_backlog_file();
      setup_ble();
      last_digital_ocean_upload = millis();
    }
  }
}
