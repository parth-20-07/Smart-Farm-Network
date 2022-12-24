#include <Arduino.h>
#include "Pin_Connection.h"
#include "Variable_Declaration.h"
#include "Device_Configration.h"
#include "NimBLE_Configuration.cpp"
#include "Sensor_Data_Collection.cpp"
#include "RTC_Configuration.cpp"
#include "Historical_data_log.cpp"
#include "Battery_Configuration.cpp"
#include "EEPROM.h"

/* -------------------------------------------------------------------------- */
/*                            VARIABLE DECLARATION                            */
/* -------------------------------------------------------------------------- */
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
uint8_t wakeup_reason_flag;

/* -------------------------------------------------------------------------- */
/*                            FUNCTION DECLARATION                            */
/* -------------------------------------------------------------------------- */
uint8_t print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
  return wakeup_reason;
}

String create_formatted_data_string(void)
{
  String msg = "N:" + node_type + ',' + "I:" + NODE_ID; // Starting to write message
  msg += "," + fetch_formatted_time();                  // Fetching Time from RTC

  // Collecting Sensor Value
  msg += collect_temperature_and_humidity_values();
  msg += collect_co2_values();
  msg += collect_lux_values();
  msg += battery_level_percentage();
  return msg;
}

bool recieved_data_via_ble(void)
{
  recieved_data_flag = false;
  String msg = read_recieved_message();
  Serial.println("Rec. Command: " + msg);
  String cmd_msg, cmd_instruction;
  {
    char token = ',';
    for (size_t i = 0; i < msg.length(); i++)
      if (msg[i] == token)
      {
        cmd_msg = msg.substring(0, i);
        cmd_instruction = msg.substring(i + 1);
      }
    Serial.println("CMD MSG: " + cmd_msg);
    Serial.println("CMD INSTRUCTION: " + cmd_instruction);
  }

  String cmd;
  {
    char token = ':';
    for (size_t i = 0; i < cmd_msg.length(); i++)
      if (cmd_msg[i] == token)
        cmd = cmd_msg.substring(i + 1);
    Serial.println("CMD: " + cmd);
  }
  if (cmd == BLE_UPDATE_TIME_COMMAND)
    set_time(cmd_instruction);
  else if (cmd == BLE_UPDATE_SYNC_COMMAND)
    read_historical_data();
  else if (cmd == BLE_UPDATE_SLEEP_COMMAND)
    return false; // False is returned only when device is needed to sleep
  else
    Serial.println("Unknown Message format");
  return true;
}

bool collect_and_send_data()
{
  Serial.println("Collecting Data");
  String msg = create_formatted_data_string();
  bool external_wakeup = false;
  if (wakeup_reason_flag == ESP_SLEEP_WAKEUP_EXT0) // Wakeup caused by user
    external_wakeup = true;
  if (!send_data_via_ble(msg, external_wakeup, true)) // Sending Data to BLE Device
  {
    Serial.println("\nData Send Failure");
    String sd_msg = "hist," + msg;
    write_historical_data(sd_msg); // If device is not connected, then saving the data in historical data array
    return false;
  }
  else
  {
    Serial.println("Data Send Success");
    return true;
  }
}

void fetch_node_id(void)
{

  EEPROM.begin(NODE_ID_SIZE);
  String node_id[NODE_ID_SIZE];
  uint8_t first_id = EEPROM.read(0);
  Serial.println("Node ID[0]: " + (String)first_id);
  if (first_id > 9)
  {
    Serial.println("Generating New Node ID");
    for (size_t i = 0; i < NODE_ID_SIZE; i++)
    {
      uint8_t id = random(0, 10);
      Serial.println("Write Node ID [" + (String)i + "]: " + (String)id);
      EEPROM.write(i, id);
    }
    EEPROM.commit();
  }

  for (size_t i = 0; i < NODE_ID_SIZE; i++)
  {
    node_id[i] = (String)EEPROM.read(i);
    NODE_ID += node_id[i];
  }
  Serial.println("Node ID: " + NODE_ID);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n=============================================================");
  Serial.println("Starting Device");
  node_type = setup_sensors();
  fetch_node_id();
  ble_setup();
  setup_rtc();
  setup_data_logger();
  battery_setup();
  wakeup_reason_flag = print_wakeup_reason();
  pinMode(WAKEUP_BUTTON, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, HIGH); // 1 = High, 0 = Low
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000);
  delay(random(0, 10) * 1000);
  Serial.println("=============================================================");
  if (collect_and_send_data())
    read_historical_data();

  if (recieved_data_flag)
    recieved_data_via_ble();

  if (wakeup_reason_flag == ESP_SLEEP_WAKEUP_EXT0) // This line is checked only when the device is woken up by external int.
  {
    Serial.println("Waiting for user to enter sleep mode command");
    while (1)
      if (recieved_data_flag)
        if (!recieved_data_via_ble())
          break;
    delay(50);
  }
  Serial.println("Going to Sleep");
  delay(50);
  esp_deep_sleep_start();
}

void loop()
{
}