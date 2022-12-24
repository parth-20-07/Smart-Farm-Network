// Reference (ESP32 Hotspot): https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
// Reference (ESP32 Server): https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
#include "Arduino.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "HTML_PAGE.h"
#include "Variable_Declaration.h"
#include "Display_Control.cpp"

/* ---------------------------- BASIC DEFINITION ---------------------------- */
AsyncWebServer server(80);
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "password";
// const char *PARAM_INPUT[NUMBER_OF_PARAMS][30] = {
String PARAM_INPUT[NUMBER_OF_PARAMS] = {
    "min_temp",
    "max_temp",
    "min_humi",
    "max_humi",
    "min_lux",
    "max_lux",
    "min_co2",
    "max_co2",
};

/* --------------------------- FUNCTION DEFINITION -------------------------- */
/**
 * @brief Function is used when there is error while creating Web Server
 * @param request
 */
void notFound(AsyncWebServerRequest *request)
{
    Serial.println("Cannot Create Server");
    request->send(404, "text/plain", "Not found");
}

/**
 * @brief Creates a Web Server which is used to enter new configuration settings like:
 * SSID
 * Password
 */
void launch_webserver(void)
{
    Serial.println("\nDevice Configuration Details Unavailable\nConfiguring WebServer");
    delay(10);
    WiFi.disconnect();
    uint32_t data_refresh_last_millis = millis();
    bool first_data_load = true;
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.mode(WIFI_AP);
    char temp_ssid[] = WEBSERVER_SSID;
    char temp_pwd[] = WEBSERVER_PASSWORD;
    WiFi.softAP(temp_ssid, temp_pwd);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(10);
    Serial.println("Connect to:");
    Serial.println("SSID: " + (String)temp_ssid);
    Serial.println("Password: " + (String)temp_pwd);
    Serial.println("IP address: 192.168.1.1");

    // Send web page with input fields to client
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1))
    {
        inputMessage = request->getParam(PARAM_INPUT_1)->value();
        inputParam = PARAM_INPUT_1;
        Serial.println("SSID: " + inputMessage);
        memset(ssid,0,SSID_CHAR_LENGTH);
        for (size_t i = 0; i < inputMessage.length(); i++)
            ssid[i] = inputMessage[i];
    }

    if (request->hasParam(PARAM_INPUT_2))
    {
        inputMessage = request->getParam(PARAM_INPUT_2)->value();
        inputParam = PARAM_INPUT_2;
        Serial.println("Password: " + inputMessage);
        memset(password,0,PASSWORD_CHAR_LENGTH);
        for (size_t i = 0; i < inputMessage.length(); i++)
            password[i] = inputMessage[i];
    }

    for (size_t i = 0; i < NUMBER_OF_PARAMS; i++)
        if (request->hasParam(PARAM_INPUT[i]))
        {
            inputMessage = request->getParam(PARAM_INPUT[i])->value();
            inputParam = PARAM_INPUT[i];
            Serial.println(PARAM_INPUT[i] + ": " + inputMessage);
            char char_msg[inputMessage.length()];
            strcpy(char_msg,inputMessage.c_str());
            params_range[i] = atoi(char_msg);
        }
    else
    {
        inputMessage = "No message sent";
        inputParam = "none";
    } });
    server.onNotFound(notFound);
    server.begin();
    while (1)
    {
        yield();

        if (((millis() - data_refresh_last_millis) > 1000) || (first_data_load))
        {
            load_alert_data_on_configuration_screen();
            data_refresh_last_millis = millis();
            first_data_load = false;
        }
        String msg = read_nextion_input_message();
        if (msg == "back")
            break;
    }
}
