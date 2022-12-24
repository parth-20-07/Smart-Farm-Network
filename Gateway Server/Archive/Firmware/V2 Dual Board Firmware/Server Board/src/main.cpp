/* ----------------------------- BASIC LIBRARIES ---------------------------- */
#include <Arduino.h>
#include "Pin_Connection.h"
#include "Variable_Declaration.h"
#include <ReadLines.h>
#include <pgmspace.h>

/* ------------------------------ CUSTOM FILES ----------------------------- */
#include "WiFi_Setup.cpp"
#include "MicroSD_Control.cpp"
#include "Display_Control.cpp"
#include "Time_Configuration.cpp"
#include "OTA_Configuration.cpp"
#include "MQTT_Connection.cpp"
#include "InterBoard_Communication.cpp"
#include "esp32_webserver.cpp"

/* --------------------------- FUNCTION DEFINITION -------------------------- */
bool deparse_msg(String msg, bool live_msg);
void handleEachSSIDLine(char line[], int lineIndex);
void handleEachPasswordLine(char line[], int lineIndex);
void read_aws_backlog_file(void);
void handleEachAWSBacklogLine(char line[], int lineIndex);

/* --------------------------- FUNCTION DECLARATION -------------------------- */

/* ---------------------------- SD CARD FUNCTIONS --------------------------- */
void fetch_ssid_password(void)
{
  RL.readLines(SSID_FILE, &handleEachSSIDLine);
  RL.readLines(PASSWORD_FILE, &handleEachPasswordLine);
}

void handleEachSSIDLine(char line[], int lineIndex)
{
  for (size_t i = 0; i < SSID_CHAR_LENGTH; i++)
    ssid[i] = line[i];
}

void handleEachConfigurationLine(char line[], int lineIndex)
{
  variable_str_val = (String)line;
}

void handleEachPasswordLine(char line[], int lineIndex)
{
  for (size_t i = 0; i < PASSWORD_CHAR_LENGTH; i++)
    password[i] = line[i];
}

void read_aws_backlog_file(void)
{
  connect_to_wifi();
  if (internet_connection_status)
  {
    if (!aws_setup_flag)
      aws_setup();
    if (connectToMqtt(false))
    {
      Serial.println("Reading Backlog Files");
      aws_file_exists = false;
      RL.readLines(AWS_BACKLOG_FILE, &handleEachAWSBacklogLine);
      if (!aws_file_exists)
        deleteFile(SD, AWS_BACKLOG_FILE);
    }
  }
}

void handleEachAWSBacklogLine(char line[], int lineIndex)
{
  deparse_msg((String)line, false);
  if (!sendData("node", rec_date, rec_time, nodeid, node_type, temp, humiditiy, lux, co2, battery_percentage))
    aws_file_exists = true;
  delay(500);
  return;
}

void handleEachConfigLine(char line[], int lineIndex)
{
  params_range[config_index] = atoi(line);
}

void handleFirmwareLine(char line[], int lineIndex)
{
  firmware_version = (String)line;
  if (firmware_version == "")
    firmware_version = "0.0";
}

/* ------------------------ DATA PROCESSING FUNCTION ------------------------ */
bool deparse_msg(String msg, bool live_msg)
{
  // Seperating the csv string
  if ((msg.substring(0, 4) == "hist") && live_msg)
  {
    appendFile(SD, AWS_BACKLOG_FILE, msg);
    return false;
  }

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

  cmd = "",
  node_type = "", nodeid = "", rec_date = "", rec_time = "", temp = 0, humiditiy = 0, lux = 0, co2 = 0, battery_percentage = 0;
  cmd = params[0];

  for (size_t i = 0; i < 10; i++)
  {
    if (param_identifiers[i] == "I")
      node_type = param_values[i];
    else if (param_identifiers[i] == "D")
    {
      rec_date = param_values[i];
      if (rec_date == "00/00/00")
      {
        printLocalTime();
        rec_date = live_date;
      }
    }
    else if (param_identifiers[i] == "G")
    {
      rec_time = param_values[i];
      if (rec_time == "00-00")
        rec_time = live_time;
    }
    else if (param_identifiers[i] == "N")
      nodeid = param_values[i];
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

  if (cmd == "live")
  {
    Serial.println("-----------------------------------------------------------");
    if (temp != 0)
    {
      database_temp[temp_values] = temp;
      if ((database_temp[temp_values] < params_range[0]) || (database_temp[temp_values] > params_range[1]))
        temp_alert = true;
      temp_values++;
      Serial.print("Temp: ");
      for (size_t i = 0; i < temp_values; i++)
        Serial.print(" " + (String)database_temp[i] + " |");
      Serial.println();
    }
    if (humiditiy != 0)
    {
      database_humiditiy[humidity_values] = humiditiy;
      if ((database_humiditiy[humidity_values] < params_range[2]) || (database_humiditiy[humidity_values] > params_range[3]))
        humidity_alert = true;
      humidity_values++;
      Serial.print("Humidity: ");
      for (size_t i = 0; i < humidity_values; i++)
        Serial.print(" " + (String)database_humiditiy[i] + " |");
      Serial.println();
    }
    if (lux != 0)
    {
      database_lux[lux_values] = lux;
      if ((database_lux[lux_values] < params_range[4]) || (database_lux[lux_values] > params_range[5]))
        lux_alert = true;
      lux_values++;
      Serial.print("Lux: ");
      for (size_t i = 0; i < lux_values; i++)
        Serial.print(" " + (String)database_lux[i] + " |");
      Serial.println();
    }
    if (co2 != 0)
    {
      database_co2[co2_values] = co2;
      if ((database_co2[co2_values] < params_range[6]) || (database_co2[co2_values] > params_range[7]))
        co2_alert = true;
      co2_values++;
      Serial.print("CO2: ");
      for (size_t i = 0; i < co2_values; i++)
        Serial.print(" " + (String)database_co2[i] + " |");
      Serial.println();
    }
    Serial.println("Values: " + (String)temp_values + " | " + (String)humidity_values + " | " + (String)lux_values + " | " + (String)co2_values);
    Serial.println("-----------------------------------------------------------");
  }
  return true;
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

  digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
  connect_to_wifi();
  if (internet_connection_status)
  {
    if (!aws_setup_flag)
      aws_setup();
    if (connectToMqtt(false))
      sendData("average", live_date, live_time, "", "", average_temp, average_humidity, average_lux, average_co2, 0);
  }
  update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, average_temp, average_humidity, average_lux, average_co2, device_connection_status);
  temp_values = 0, humidity_values = 0, lux_values = 0, co2_values = 0, temp_alert = false, humidity_alert = false, lux_alert = false, co2_alert = false;
  memset(database_temp, 0, MAX_NODES);
  memset(database_humiditiy, 0, MAX_NODES);
  memset(database_lux, 0, MAX_NODES);
  memset(database_co2, 0, MAX_NODES);
  lastMillis = millis();
  digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
}

/* --------------------------- WEBSERVER FUNCTION --------------------------- */
void load_configuration_server(void)
{
  load_configuration_screen(); // Load Configuration screen on display
  launch_webserver();
  load_boot_screen();
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
void fetch_config_value(String str_path)
{
  char path[str_path.length()];
  strcpy(path, str_path.c_str());
  RL.readLines(path, &handleEachConfigLine);
}

void fetch_firmware_version(void)
{
  String str_path = FIRMWARE_VERSION_FILE;
  char path[str_path.length()];
  strcpy(path, str_path.c_str());
  RL.readLines(path, &handleFirmwareLine);
}

void fetch_configuration_data(void)
{
  fetch_ssid_password(); // Retrieving SSID and Password

  // Retrieving Parameter Range Values
  for (size_t i = 0; i < NUMBER_OF_PARAMS; i++)
  {
    String str_path = "/" + (String)i + ".txt";
    config_index = i;
    fetch_config_value(str_path);
    delay(10);
  }

  fetch_firmware_version();

  RL.readLines(THINGNAME_FILE, &handleEachConfigurationLine);
  strcpy(THINGNAME, variable_str_val.c_str());
  Serial.println("Thingname: " + (String)THINGNAME);

  RL.readLines(AWS_IOT_PUBLISH_TOPIC_FILE, &handleEachConfigurationLine);
  AWS_IOT_PUBLISH_TOPIC = variable_str_val;
  Serial.println("AWS Publish Topic: " + AWS_IOT_PUBLISH_TOPIC);

  RL.readLines(AWS_IOT_SUBSCRIBE_TOPIC_FILE, &handleEachConfigurationLine);
  AWS_IOT_SUBSCRIBE_TOPIC = variable_str_val;
  Serial.println("AWS Subscribe Topic: " + AWS_IOT_SUBSCRIBE_TOPIC);

  RL.readLines(AWS_IOT_ENDPOINT_FILE, &handleEachConfigurationLine);
  strcpy(AWS_IOT_ENDPOINT, variable_str_val.c_str());
  Serial.println("AWS IOT Endpoint: " + (String)AWS_IOT_ENDPOINT);
  Serial.println("Read Complete");

  RL.readLines(URL_FW_VERSION_FILE, &handleEachConfigurationLine);
  URL_fw_Version = variable_str_val;
  Serial.println("Firmware Version Link: " + URL_fw_Version);

  RL.readLines(URL_FW_BIN, &handleEachConfigurationLine);
  URL_fw_Bin = variable_str_val;
  Serial.println("Firmware File Link: " + URL_fw_Bin);
}

/* ------------------------------- DEVICE CODE ------------------------------ */
void setup()
{
  Serial.begin(115200); // open serial via USB to PC on default port
  Serial.println("\n\n=================================================");
  Serial.println("Setting Up Device");
  setup_pins();
  setup_display();
  setup_microsd();
  fetch_configuration_data();
  if ((((String)ssid).length() == 0) || (((String)password).length() == 0))
    load_configuration_server();
  wifi_connection_status = connect_to_wifi();
  rtc_setup();
  if (internet_connection_status)
  {
    connect_to_ntp();
    if (internet_connection_status)
    {
      if (check_for_new_firmware())
      {
        load_firmware_upgrade_screen();
        writeFile(SD, FIRMWARE_VERSION_FILE, firmware_version);
        update_firmware();
        delay(10);
        ESP.restart();
      }
      aws_setup();
      read_aws_backlog_file();
    }
  }
  setup_communication_between_boards();
  digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
  lastMillis = millis();
  last_internet_refresh = millis();
  last_millis_time_update = millis() + 70000;
  load_main_data_screen();
  last_aws_upload = millis();
  Serial.println("Device Setup Complete");
  Serial.println("=================================================");
}

void loop()
{
  // Check if sensor esp board send any msg
  if (digitalRead(SENSOR_BOARD_CONTROL_PIN) == LOW)
  {
    String msg = read_message();
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    if (msg == "nodes_active")
      device_connection_status = true;
    else if (msg == "no_nodes")
      device_connection_status = false;
    else if (msg.length() > 10)
      if (deparse_msg(msg, true))
      {
        String str_msg = "N:" + node_type + ",I:" + nodeid + ",D:" + rec_date + ",G:" + rec_time + ",T:" + (String)temp + ",H:" + (String)humiditiy + ",C:" + (String)co2 + ",L:" + (String)lux + ",B:" + (String)battery_percentage;
        appendFile(SD, AWS_BACKLOG_FILE, str_msg);
        aws_file_exists = true;
      }

    if ((!calculated_first_average) && (cmd == "live"))
      update_display(wifi_connection_status, internet_connection_status, temp_alert, humidity_alert, lux_alert, co2_alert, live_date, live_time, temp, humiditiy, lux, co2, device_connection_status);
  }

  if (((millis() - lastMillis) > DISPLAY_UPDATE_TIME) || (temp_values == MAX_NODES - 1) || (humidity_values == MAX_NODES - 1) || (lux_values == MAX_NODES - 1) || (co2_values == MAX_NODES - 1)) // update display with new average values
  {
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    calculate_averages_and_update_display();
  }

  String recieved_message = read_nextion_input_message(); // Check if nextion display sent any msg
  if (recieved_message == "config")
  {
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    load_configuration_server();
  }

  if ((millis() - last_internet_refresh) > 300000) // Check Internet Connection periodically
  {
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    if ((WiFi.status() != WL_CONNECTED) || (!internet_connection_status))
      if (connect_to_wifi())
        connect_to_ntp();
    last_internet_refresh = millis();
  }

  if ((millis() - last_millis_time_update) > 60000) // Update time on display
  {
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    rtc_get_time();
    update_data_on_screen("date", "txt", 0, live_date); // Setting Date
    update_data_on_screen("time", "txt", 0, live_time); // Setting Time
    last_millis_time_update = millis();
  }

  if ((millis() - last_aws_upload) > 900000)
  {
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    if (aws_file_exists)
      read_aws_backlog_file(); // Update to AWS
  }
  last_aws_upload = millis();

  digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
}