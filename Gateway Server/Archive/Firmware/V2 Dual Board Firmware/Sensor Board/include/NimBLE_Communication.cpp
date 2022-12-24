#ifndef NimBLE_Communication_cpp
#define NimBLE_Communication_cpp

#include <Arduino.h>
#include "NimBLEDevice.h"
#include "Variable_Declaration.h"

static BLEUUID serviceUUID(SERVICE_UUID);        // The remote service we wish to connect to.
static BLEUUID charUUID(CHARACTERISTIC_UUID_TX); // The characteristic of the remote service we are interested in.

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    char rec_data[length];
    for (size_t i = 0; i < length; i++)
        rec_data[i] = pData[i];
    String incoming_msg = (String)rec_data;
    char token = '#';
    incoming_ble_msg = "";
    for (size_t i = 0; i < incoming_msg.length(); i++)
        if (incoming_msg[i] == token)
            incoming_ble_msg = incoming_msg.substring(0, i);
    Serial.println("Incoming Data: " + incoming_ble_msg);
    recieved_data_flag = true;
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
    }
    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        Serial.println("onDisconnect");
    }
    /***************** New - Security handled here ********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Client PassKeyRequest");
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

bool connectToServer()
{
    // Serial.print("Forming a connection to ");
    // Serial.println(myDevice->getAddress().toString().c_str());
    BLEClient *pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    if (pRemoteCharacteristic->canRead()) // Read the value of the characteristic.
        std::string value = pRemoteCharacteristic->readValue();

    if (pRemoteCharacteristic->canNotify())
        pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice *advertisedDevice)
    {
        // Serial.print("BLE Advertised Device found: ");
        // Serial.println(advertisedDevice->toString().c_str());
        if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID))
        {
            BLEDevice::getScan()->stop();
            myDevice = advertisedDevice; /** Just save the reference now, no need to copy the object */
            doConnect = true;
            doScan = true;
        } // Found our server
    }     // onResult
};        // MyAdvertisedDeviceCallbacks

void setup_ble(void)
{
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init(BLE_DEVICE_NAME);
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(10000);
    pBLEScan->setWindow(500);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(2, false);
}

void read_msg(void)
{
    setup_ble();
    if (doConnect == true)
    {
        Serial.println("----------------------------------------------------");
        if (connectToServer())
            Serial.println("We are now connected to the BLE Server.");
        else
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
        doConnect = false;
    }
    // if (connected)
    //     ;
    // else if (doScan)
    //     BLEDevice::getScan()->start(2); // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
}

#endif