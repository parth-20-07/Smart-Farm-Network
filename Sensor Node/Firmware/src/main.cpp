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

bool recieved_data_via_ble(void)
{
  recieved_data_flag = false;
  String msg = read_recieved_message();
  Serial.println("Rec. Command: " + msg);
  String cmd_msg, cmd_instruction;
  {
    char token = ',';
    uint8_t msg_length = msg.length();
    for (size_t i = 0; i < msg_length; i++)
      if (msg[i] == token)
      {
        cmd_msg = msg.substring(0, i);
        cmd_instruction = msg.substring(i + 1, msg_length - 2);
        i = msg_length;
      }
    Serial.println("CMD MSG: " + cmd_msg);
    Serial.println("CMD INSTRUCTION: " + cmd_instruction);
  }

  if (cmd_msg == BLE_UPDATE_TIME_COMMAND)
    set_time(cmd_instruction);
  else if (cmd_msg == BLE_UPDATE_SYNC_COMMAND)
    read_historical_data();
  else if (cmd_msg == BLE_UPDATE_SLEEP_COMMAND)
    return true; // False is returned only when device is needed to sleep
  else
    Serial.println("Unknown Message format");
  return false;
}

void fetch_node_id(void)
{

  EEPROM.begin(NODE_ID_SIZE);
  String node_id[NODE_ID_SIZE];
  uint8_t first_id = EEPROM.read(0);
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
  Serial.println("\n\n/* -------------------------- STARTING DEVICE SETUP ------------------------- */");
  fetch_node_id();
  bool external_wakeup = false;

#ifdef DUMMY_NODE
  Serial.println("In Dummy Mode");
  node_type = "THCL";
#else
  uint8_t wakeup_reason_flag;
  Serial.println("In Operational Mode");
  node_type = setup_sensors();
  wakeup_reason_flag = print_wakeup_reason();
  if (wakeup_reason_flag == ESP_SLEEP_WAKEUP_EXT0) // Wakeup caused by user
    external_wakeup = true;
  setup_rtc();
  battery_setup();
  pinMode(WAKEUP_BUTTON, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, HIGH); // 1 = High, 0 = Low
#endif // DUMMY_NODE
  setup_data_logger();

  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000);
  delay(random(0, 10) * 1000);
  ble_setup();
  Serial.println("/* ----------------------------- SETUP COMPLETE ----------------------------- */");

  Serial.println("Collecting Data");
  String msg = "N:" + node_type + ',' + "I:" + NODE_ID; // Starting to write message
  msg += "," + fetch_formatted_time();                  // Fetching Time from RTC

// Collecting Sensor Value
#ifdef DUMMY_NODE
  msg += ",T:" + (String)random(0, 100) + ",H:" + (String)random(0, 100);
  msg += ",C:" + (String)random(20, 400);
  msg += ",L:" + (String)random(20, 600);
  msg += ",B:" + (String)random(0, 100);
#else
  msg += collect_temperature_and_humidity_values();
  msg += collect_co2_values();
  msg += collect_lux_values();
  msg += battery_level_percentage();
#endif // DUMMY_NODE

  Serial.println("NODE MSG: " + msg);
  send_data_via_ble("start", external_wakeup, true);
  delay(500);

  if (!send_data_via_ble(msg, external_wakeup, true)) // Sending Data to BLE Device
  {
    Serial.println("\nData Send Failure");
    String sd_msg = "hist," + msg;
    write_historical_data(sd_msg); // If device is not connected, then saving the data in historical data array
  }
  else
  {
    read_historical_data();
    delay(500);
    send_data_via_ble("end", false, true);
  }

#ifndef DUMMY_NODE
  if (wakeup_reason_flag == ESP_SLEEP_WAKEUP_EXT0) // This line is checked only when the device is woken up by external int.
  {
    Serial.println("Waiting for user to enter sleep mode command");
    while (1)
    {
      if (recieved_data_flag)
        if (recieved_data_via_ble())
          break;
      delay(50);
    }
  }
#endif // DUMMY_NODE

  // Serial.println("Raw Battery ADC: " + (String)analogRead(BATTERY_ADC_PIN));// TODO: Uncomment this line when battery ADC needs to be measured.

  Serial.println("Going to Sleep");
  delay(50);
  esp_deep_sleep_start();
}

void loop()
{
}