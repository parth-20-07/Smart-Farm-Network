// Reference (AWS IoT Core): https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
#ifndef MQTT_Connection_cpp
#define MQTT_Connection_cpp

#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "AWS_Configuration.h"
#include "Variable_Declaration.h"
#include "Time_Configuration.cpp"
#include "WiFi_Setup.cpp"
#include "WiFi.h"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void aws_setup(void);
void messageReceived(String &topic, String &payload);
void read_aws_backlog_file();
void handleEachAWSBacklogLine(char line[], int lineIndex);
void lwMQTTErr(lwmqtt_err_t reason);
void lwMQTTErrConnection(lwmqtt_return_code_t reason);
bool connectToMqtt(bool nonBlocking);
void checkMQTTNonBlocking(void);
bool sendData(String data_format, String date, String time, String nodeid, String node_type, float temp, uint8_t humiditiy, uint16_t lux, uint16_t co2, uint8_t battery_percentage);

/* -------------------------- FUNCTION DECLARATION -------------------------- */
/**
 * @brief Setup AWS
 *
 */
void aws_setup(void)
{
    Serial.println("Setting up AWS");
    delay(10);

    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);
    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.begin(AWS_IOT_ENDPOINT, 8883, net);
    // Create a message handler
    client.onMessage(messageReceived);
    Serial.print("Connecting to AWS IOT");
    while (!client.connect(THINGNAME))
    {
        Serial.print(".");
        delay(10);
    }
    if (!client.connected())
    {
        Serial.println("AWS IoT Timeout!");
        return;
    }
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC); // Subscribe to a topic
    Serial.println("AWS IoT Connected!");
    aws_setup_flag = true;
    delay(10);
}

/**
 * @brief This function is activated when the message is recieved from AWS
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

    String cmd = doc["cmd"]; // Perform action as per the command from AWS
}

void lwMQTTErr(lwmqtt_err_t reason)
{
    if (reason == lwmqtt_err_t::LWMQTT_SUCCESS)
        Serial.print("Success");
    else if (reason == lwmqtt_err_t::LWMQTT_BUFFER_TOO_SHORT)
        Serial.print("Buffer too short");
    else if (reason == lwmqtt_err_t::LWMQTT_VARNUM_OVERFLOW)
        Serial.print("Varnum overflow");
    else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_CONNECT)
        Serial.print("Network failed connect");
    else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_TIMEOUT)
        Serial.print("Network timeout");
    else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_READ)
        Serial.print("Network failed read");
    else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_WRITE)
        Serial.print("Network failed write");
    else if (reason == lwmqtt_err_t::LWMQTT_REMAINING_LENGTH_OVERFLOW)
        Serial.print("Remaining length overflow");
    else if (reason == lwmqtt_err_t::LWMQTT_REMAINING_LENGTH_MISMATCH)
        Serial.print("Remaining length mismatch");
    else if (reason == lwmqtt_err_t::LWMQTT_MISSING_OR_WRONG_PACKET)
        Serial.print("Missing or wrong packet");
    else if (reason == lwmqtt_err_t::LWMQTT_CONNECTION_DENIED)
        Serial.print("Connection denied");
    else if (reason == lwmqtt_err_t::LWMQTT_FAILED_SUBSCRIPTION)
        Serial.print("Failed subscription");
    else if (reason == lwmqtt_err_t::LWMQTT_SUBACK_ARRAY_OVERFLOW)
        Serial.print("Suback array overflow");
    else if (reason == lwmqtt_err_t::LWMQTT_PONG_TIMEOUT)
        Serial.print("Pong timeout");
}

void lwMQTTErrConnection(lwmqtt_return_code_t reason)
{
    if (reason == lwmqtt_return_code_t::LWMQTT_CONNECTION_ACCEPTED)
        Serial.print("Connection Accepted");
    else if (reason == lwmqtt_return_code_t::LWMQTT_UNACCEPTABLE_PROTOCOL)
        Serial.print("Unacceptable Protocol");
    else if (reason == lwmqtt_return_code_t::LWMQTT_IDENTIFIER_REJECTED)
        Serial.print("Identifier Rejected");
    else if (reason == lwmqtt_return_code_t::LWMQTT_SERVER_UNAVAILABLE)
        Serial.print("Server Unavailable");
    else if (reason == lwmqtt_return_code_t::LWMQTT_BAD_USERNAME_OR_PASSWORD)
        Serial.print("Bad UserName/Password");
    else if (reason == lwmqtt_return_code_t::LWMQTT_NOT_AUTHORIZED)
        Serial.print("Not Authorized");
    else if (reason == lwmqtt_return_code_t::LWMQTT_UNKNOWN_RETURN_CODE)
        Serial.print("Unknown Return Code");
}

bool connectToMqtt(bool nonBlocking)
{
    if (!client.connected())
    {
        Serial.print("MQTT Connecting ");
        if (client.connect(THINGNAME))
        {
            Serial.println("\tConnected!");
            return true;
        }
        else
        {
            Serial.println("\tFailed!");
            return false;
        }
    }
    else
        return true;
}

unsigned long previousMillis = 0;
const long interval = 5000;

void checkMQTTNonBlocking(void)
{
    if (millis() - previousMillis >= interval && !client.connected())
    {
        previousMillis = millis();
        connectToMqtt(true);
    }
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
bool sendData(String data_format, String date, String time, String nodeid, String node_type, float temp, uint8_t humiditiy, uint16_t lux, uint16_t co2, uint8_t battery_percentage)
{
    // Reference:https://github.com/bblanchon/ArduinoJson/blob/6.x/examples/JsonGeneratorExample/JsonGeneratorExample.ino
    Serial.print("Sending to AWS:\t");
    StaticJsonDocument<500> doc;
    doc["df"] = data_format; // Tells if data is from node or average data
    doc["date"] = date;
    doc["time"] = time;
    doc["nodeid"] = nodeid;
    doc["node_type"] = node_type;
    doc["temp"] = temp;
    doc["humiditiy"] = humiditiy;
    doc["lux"] = lux;
    doc["co2"] = co2;
    doc["batt"] = battery_percentage;
    char shadow[measureJson(doc) + 1];
    serializeJson(doc, shadow, sizeof(shadow));
    Serial.println((String)shadow);
    if (client.publish(AWS_IOT_PUBLISH_TOPIC, shadow, false, 0))
    {
        Serial.println("Send Success");
        return true;
    }
    else
    {
        lwMQTTErr(client.lastError());
        return false;
    }
}

#endif