#include "Arduino.h"
#include "Pass_Configure.h"
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "Arduino.h"
#include "Variable_Declaration.h"

/* --------------------------- FUNCTION DEFINITION -------------------------- */
void update_firmware(void)
{
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf(" HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println(" HTTP_UPDATE_NO_UPDATES");
        break;
    case HTTP_UPDATE_OK:
        Serial.println(" HTTP_UPDATE_OK");
        break;
    }
}

bool check_for_new_firmware(void)
{
    String payload;
    int httpCode;
    String fwurl = "";
    fwurl += URL_fw_Version;
    fwurl += "?";
    fwurl += String(rand());
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client)
    {
        client->setCACert(rootCACertificate);
        HTTPClient https;
        if (https.begin(*client, fwurl))
        {
            delay(10);
            httpCode = https.GET();
            delay(10);
            if (httpCode == HTTP_CODE_OK)
                payload = https.getString();
            else
            {
                Serial.print(" Error in downloading version file: ");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }
    if (httpCode == HTTP_CODE_OK)
    {
        payload.trim();
        if (payload.equals(firmware_version))
        {
            Serial.printf(" The Device is already running on latest firmware version:%s\n", firmware_version);
            return false;
        }
        else
        {
            Serial.print("New firmware found: ");
            Serial.println(payload);
            Serial.println("Uploading new firmware");
            firmware_version = payload;
            return true;
        }
    }
    return false;
}