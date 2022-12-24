// Reference: https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/Refactored_original_examples/BLE_uart/BLE_uart.ino
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    Create a BLE server that, once we receive a connection, will send periodic notifications.
    The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
    Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
    Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"
    The design of creating the BLE server is:
    1. Create a BLE Server
    2. Create a BLE Service
    3. Create a BLE Characteristic on the Service
    4. Create a BLE Descriptor on the characteristic
    5. Start the service.
    6. Start advertising.
    In this example rxValue is the data received (only accessible inside that function).
    And txValue is the data to be sent, in this example just a byte incremented every second.
*/

/** NimBLE differences highlighted in comment blocks **/

#ifndef NimBLE_Configuration_cpp
#define NimBLE_Configuration_cpp
#include <NimBLEDevice.h>
#include "Arduino.h"
#include "Variable_Declaration.h"

char recieved_message[100];
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
    /***************** New - Security handled here ********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Server PassKeyRequest");
        return 123456;
    }

    bool onConfirmPIN(uint32_t pass_key)
    {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        return true;
    }

    void onAuthenticationComplete(ble_gap_conn_desc desc)
    {
        Serial.println("Starting BLE work!");
    }
    /*******************************************************************/
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            Serial.print("\nReceived Value: ");
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
                recieved_message[i] = rxValue[i];
            }
            recieved_data_flag = true;
            Serial.println("Flag: " + (String)recieved_data_flag);
        }
    }
};

void ble_setup()
{
    String device_name = DEVICE_NAME + ' ' + NODE_ID;
    char char_device_name[device_name.length()];
    strcpy(char_device_name, device_name.c_str());
    Serial.println("BLE Device Name: " + (String)char_device_name);
    BLEDevice::init(char_device_name);   // Create the BLE Device
    pServer = BLEDevice::createServer(); // Create the BLE Server
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID); // Create the BLE Service
    pTxCharacteristic = pService->createCharacteristic(          // Create a BLE Characteristic
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::INDICATE);
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::INDICATE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();                                          // Start the service
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Start advertising
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

bool send_data_via_ble(String send_msg, bool send_mode, bool live_data)
{
    Serial.print("Sending Data via BLE");
    bool ble_data_send_success = false;
    int i = 0;
    while ((!ble_data_send_success) && (i <= 5))
    {
        Serial.print(".");
        if (!send_mode)
            i++;
        if (deviceConnected) // Device Connected
        {
            Serial.print("\tBLE Device Connected");
            Serial.println();
            String msg;
            if (live_data)
                msg += "live,";
            msg += send_msg + '#';
            uint16_t msg_length = msg.length();
            char char_msg[msg_length];
            strcpy(char_msg, msg.c_str());
            Serial.println("Msg: " + (String)char_msg);
            pTxCharacteristic->setValue((uint8_t *)&char_msg, msg_length);
            pTxCharacteristic->notify();
            ble_data_send_success = true;
            delay(50);
            Serial.println("Data Send Success");
        }

        if (!deviceConnected && oldDeviceConnected) // disconnecting
        {
            delay(500);                  // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            Serial.println("start advertising");
            oldDeviceConnected = deviceConnected;
        }
        if (deviceConnected && !oldDeviceConnected) // connecting
            oldDeviceConnected = deviceConnected;   // do stuff here on connecting}
        delay(500);
    }
    return ble_data_send_success;
}

String read_recieved_message(void)
{
    String msg = (String)recieved_message;
    recieved_data_flag = false;
    return msg;
}
#endif