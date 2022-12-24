#include <Arduino.h>
#include "Pin_Connection.h"
#include "InterBoard_Communication.cpp"
#include "Sensor_Data_Collection.cpp"
#include "NimBLE_Communication.cpp"
#include "Variable_Declaration.h"

void setup()
{
  Serial.begin(115200); // open serial via USB to PC on default port
  Serial.println("\n\n===================================================");
  Serial.println("Starting Device");
  setup_pins();
  setup_communication_between_boards();
  setup_sensors();
  delay(10000);
  while (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
    ;
  delay(5000);
  while (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
    ;
  last_millis = millis();
  last_ble_recieved_message = millis();
  Serial.println("===================================================");
}

void loop()
{
  if (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
    Serial.print("Server Busy ");
  while (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
  {
    server_intitated_pause = true;
    delay(50);
    Serial.print(".");
  }
  if (server_intitated_pause)
  {
    Serial.println();
    delay(20000);
  }
  server_intitated_pause = false;

  if (((millis() - last_millis) > ONBOARD_SENSOR_DATA_REFRESH_TIME) || (first_boot)) // Reading onboard sensor data
  {
    first_boot = false;
    Serial.println("Fetching onboard Sensor Values");
    String msg = "live";
    msg += ",N:TH,I:00000";       // Starting to write message
    msg += ",D:00/00/00,G:00-00"; // Fetching Time from RTC
    msg += ",T:" + (String)read_onboard_temperature() + ",H:" + (String)read_onboard_humidity();
    send_message(msg);
    last_millis = millis();
    last_ble_recieved_message = millis();
  }

  // Serial.println("last_millis: " + (String)(millis() - last_millis));
  // Serial.println("last_ble_recieved_message: " + (String)(millis() - last_ble_recieved_message));

  if ((millis() - last_ble_recieved_message) > 30000) // Reducing onboard sensor data collection delay since no node is connected
  {
    Serial.println("No Device Connected");
    ONBOARD_SENSOR_DATA_REFRESH_TIME = 30000;
    if (nodes_connected)
    {
      send_message("no_nodes");
      nodes_connected = false;
    }
  }

  read_msg();             // Trying to read message via ble
  if (recieved_data_flag) // Sending data to server board recieved via ble
  {
    recieved_data_flag = false;
    if (incoming_ble_msg != "")
    {
      send_message((String)incoming_ble_msg);
      ONBOARD_SENSOR_DATA_REFRESH_TIME = 300000;
      if (((String)incoming_ble_msg).length() > 10)
        if (!nodes_connected)
        {
          send_message("nodes_active");
          nodes_connected = true;
        }
      last_ble_recieved_message = millis();
    }
  }
}