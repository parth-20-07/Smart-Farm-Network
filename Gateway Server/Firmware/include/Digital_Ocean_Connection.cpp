// Reference (Digital Ocean IoT Core): https://RandomNerdTutorials.com/cloud-mqtt-mosquitto-broker-access-anywhere-digital-ocean/
#ifndef Digital_Ocean_Connection_cpp
#define Digital_Ocean_Connection_cpp

#include "Arduino.h"
#include <ArduinoJson.h>
#include "Variable_Declaration.h"
#include "Time_Configuration.cpp"
#include "WiFi_Setup.cpp"
#include "WiFi.h"
#include <AsyncMqttClient.h>

/* ---------------------------- Basic Definition ---------------------------- */
#define MQTT_PORT 1883
AsyncMqttClient mqttClient;

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttPublish(uint16_t packetId);
void connectToMqtt();
void disconnect_from_mqtt();
void mqtt_setup(void);
void messageReceived(String &topic, String &payload);
void sendData(String data_format, String date, String time, String nodeid, String node_type, float temp, uint8_t humiditiy, uint16_t lux, uint16_t co2, uint8_t battery_percentage);

/* -------------------------- FUNCTION DECLARATION -------------------------- */
void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");
}

void onMqttPublish(uint16_t packetId)
{
    Serial.print("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    mqtt_publish_confirmation = true;
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void disconnect_from_mqtt(void)
{
    Serial.println("Disconnecting From MQTT");
    mqttClient.disconnect();
}

/**
 * @brief Setup digital_ocean
 *
 */
void mqtt_setup(void)
{
    Serial.println("Setting up Digital Ocean");
    delay(10);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    /*mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);*/
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    // If your broker requires authentication (username and password), set them below
    mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
    mqtt_setup_flag = true;
}

/**
 * @brief This function is activated when the message is recieved from digital_ocean
 *
 * @param topic
 * @param payload
 */
void messageReceived(String &topic, String &payload)
{ // Reference (JSON to String Converter): https://github.com/bblanchon/ArduinoJson/blob/6.x/examples/JsonParserExample/JsonParserExample.ino
    Serial.println("Recieved:\n" + payload);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.println("deserializeJson() failed: " + (String)error.f_str());
        return;
    }

    String cmd = doc["cmd"]; // Perform action as per the command from digital_ocean
}
/**
 * @brief
 * @param data_format Tells if data is from "node" or "average" data
 * @param cmd Specify if "live" data or "hist" = historical data
 * @param date
 * @param time
 * @param nodeid
 * @param node_type
 * @param temp
 * @param humiditiy
 * @param lux
 * @param co2
 * @param battery_percentage
 * @return true
 * @return false
 */
void sendData(String data_format, String date, String time, String nodeid, String node_type, float temp, uint8_t humiditiy, uint16_t lux, uint16_t co2, uint8_t battery_percentage)
{
    // Reference:https://github.com/bblanchon/ArduinoJson/blob/6.x/examples/JsonGeneratorExample/JsonGeneratorExample.ino
    Serial.print("Sending Data to Digital Ocean");
    mqtt_publish_confirmation = false;
    StaticJsonDocument<500> doc;
    doc["df"] = data_format; // Tells if data is from node or average data
    doc["date"] = date;
    doc["time"] = time;
    if (nodeid == "00000")
        doc["gatewayid"] = nodeid;
    else
        doc["nodeid"] = nodeid;
    doc["node_type"] = node_type;
    if (temp != 0)
        doc["temp"] = temp;
    if (humiditiy != 0)
        doc["humiditiy"] = humiditiy;
    if (lux != 0)
        doc["lux"] = lux;
    if (co2 != 0)
        doc["co2"] = co2;
    doc["batt"] = battery_percentage;
    char shadow[measureJson(doc) + 1];
    serializeJson(doc, shadow, sizeof(shadow));

    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEST, 1, true, shadow);
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEST, packetIdPub1);
    Serial.printf(" Message: %.2f \n", (String)shadow);
}

#endif // Digital_Ocean_Connection_cpp