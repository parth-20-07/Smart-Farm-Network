#include <NimBLEDevice.h>
#include <Wire.h>
#include <SparkFunTSL2561.h>
#include "RTClib.h"
#include "SPIFFS.h"
#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "FS.h"
#include "SPI.h"

/* -------------------------------------------------------------------------- */
/*                            VARIABLE DECLARATION                            */
/* -------------------------------------------------------------------------- */
static char Device_Name[] = "BLE_Node"; // Enter the device name here
#define SERVICE_UUID "208fc8fc-64ed-4423-ba22-2230821ae406"
#define CHARACTERISTIC_UUID "e462c4e9-3704-4af8-9a20-446fa2eef1d0"
#define htu21daddress B1000000 // 0x40 in HEXA, 64 in DEC
#define htu21dtemp B11110011   // 0xF3 in HEXA, 243 in DEC
#define htu21dhumid B11110101  // 0xF5 in HEXA, 245 in DEC
#define FORMAT_SPIFFS_IF_FAILED false
#define S_TO_uS_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 300         /* Time ESP32 will go to sleep (in seconds) */
Preferences pref;
SFE_TSL2561 light;
RTC_DS3231 rtc;
char cmdKey = 'X';
bool cmdF = false; // Flag to indicate command received from mobile app
byte cmdBuff[4];   // Buffer to temporary store information sent from mobile App
byte i = 0;
char buf[500];
char timeStamp[17];
char txdata[53];
char cDate[11];
char nDate[11];
char fDate[11];
char readnextDate[11];
char Ntype[12];
String id = "0000001";
const char *cvalue = "/Current_Value.txt";
const char *pvalue = "/Previous_Value.txt";
const char *fdate = "/First_Date.txt";
const char *ndate = "/Next_Date.txt";
int tBytes = 0;
uint16_t fday = 0;
uint16_t fmonth = 0;
uint16_t fyear = 0;
uint16_t cday = 0;
uint16_t cmonth = 0;
uint16_t cyear = 0;
uint16_t nday = 0;
uint16_t nmonth = 0;
uint16_t nyear = 0;
static NimBLEServer *pServer;
NimBLEService *pService = NULL;
NimBLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool bleCon = true; // For 2561
boolean gain;       // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;    // Integration time in milliseconds

/* -------------------------------------------------------------------------- */
/*                            FUNCTION DECLARATION                            */
/* -------------------------------------------------------------------------- */
void print_wakeup_reason();
void renamecFile();
void deletenDate();
void deletefDate();
void deletepFile();
void readpFile();
void rtcmodule();
void appendcFile();
void nextDate();
String readnDate();
void rtcTime();
void firstDate();
void bleconf();
float readhtu21dTemp();
float readhtu21dHumidity();
double luxread();
void luxhtu();

/* -------------------------------------------------------------------------- */
/*                               BLE DECLARATION                              */
/* -------------------------------------------------------------------------- */
class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    Serial.println("Client connected");
    Serial.println("Multi-connect support: start advertising");
    NimBLEDevice::startAdvertising();
  };
  /** Alternative onConnect() method to extract details of the connection.
   *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
   */
  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
  {
    Serial.print("Client address: ");
    Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
    /** We can use the connection handle here to ask for different connection parameters.
     *  Args: connection handle, min connection interval, max connection interval
     *  latency, supervision timeout.
     *  Units; Min/Max Intervals: 1.25 millisecond increments.
     *  Latency: number of intervals allowed to skip.
     *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
     */
    pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
  };
  void onDisconnect(NimBLEServer *pServer)
  {
    Serial.println("Client disconnected - start advertising");
    NimBLEDevice::startAdvertising();
  };
  void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
  {
    Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
  };

  /********************* Security handled here **********************
  ****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest()
  {
    Serial.println("Server Passkey Request");
    /** This should return a random 6 digit number for security
     *  or make your own static passkey as done here.
     */
    return 123456;
  };

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.print("The passkey YES/NO number: ");
    Serial.println(pass_key);
    /** Return false if passkeys don't match. */
    return true;
  };

  void onAuthenticationComplete(ble_gap_conn_desc *desc)
  {
    /** Check that encryption was successful, if not we disconnect the client */
    if (!desc->sec_state.encrypted)
    {
      NimBLEDevice::getServer()->disconnect(desc->conn_handle);
      Serial.println("Encrypt connection failed - disconnecting client");
      return;
    }
    Serial.println("Starting BLE work!");
  };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onRead(NimBLECharacteristic *pCharacteristic)
  {
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    Serial.print(": onRead(), value: ");
    Serial.println(pCharacteristic->getValue().c_str());
  };

  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    Serial.print(pCharacteristic->getUUID().toString().c_str());
    std::string value = pCharacteristic->getValue();
    /*
     * Check for commands received from Mobile App
     *  Factory Reset command    : F
     *  Restart command          : R
     *  RTC Adjust command       : C
     */
    if (value.length() > 0)
    {
      pref.begin("store", false);
      cmdF = true;
      char cmd[(value.length()) + 1];
      uint8_t cdelm = 0;
      i = 0;
      while (i <= value.length())
      {
        cmd[i] = value[i];
        i++;
      }
      cmd[i] = '\0';
      Serial.println("-----------");
      i = 0;
      //------------------------------------------------------//
      // Parsing CSV command data to extract useful information
      //------------------------------------------------------//
      if (cmd[i] == 'F' && cmd[i + 1] == '\0')
      { //[Factory Reset Command]
        cmdKey = 'F';
        Serial.println("Factory Reset Command"); // Command Format: {'F','\0'}
        nvs_flash_erase();                       // Erase Non-Voltage Storage (NVS) which stores state variables
        nvs_flash_init();

        bool formatted = SPIFFS.format();

        if (formatted)
        {
          Serial.println("\n\nSuccess formatting");
        }
        else
        {
          Serial.println("\n\nError formatting");
        } // Delete data point file stored in SPIFFS FileSystem
        ESP.restart();
      }
      else if (cmd[i] == 'R' && cmd[i + 1] == '\0')
      { //[Restart Command]
        cmdKey = 'R';
        Serial.println("Restart Command"); // Command Format: {'R','\0'}
        ESP.restart();
      }
      else if (cmd[i] == 'C') //[RTC Time Adjust Command]
      {                       // Command Format: C,2021,09,27,11,51 for AM
        cmdKey = 'C';
        Serial.println("RTC Time Adjust Command"); // Command Format: C,2021,09,27,13,51 for PM
        i++;
        unsigned int yrr, mnth, dy, hrr, mnn, n;
        char buf[5];
        char delm = ','; // CSV format using comma as delimeter
        if (cmd[i] == delm)
        {
          i++;
          for (n = 0; n < 4; n++) // Year
            buf[n] = cmd[i++];
          buf[n] = NULL;
          yrr = atoi(buf);
          i++;
          for (n = 0; n < 2; n++) // Month
            buf[n] = cmd[i++];
          buf[n] = NULL;
          mnth = atoi(buf);
          i++;
          for (n = 0; n < 2; n++) // Day
            buf[n] = cmd[i++];
          buf[n] = NULL;
          dy = atoi(buf);
          i++;
          for (n = 0; n < 2; n++) // Hr
            buf[n] = cmd[i++];
          buf[n] = NULL;
          hrr = atoi(buf);
          i++;
          for (n = 0; n < 2; n++) // Min
            buf[n] = cmd[i++];
          buf[n] = NULL;
          mnn = atoi(buf);

          rtc.adjust(DateTime(yrr, mnth, dy, hrr, mnn, 0));
          Serial.println("RTC Time Adjusted.");
          pCharacteristic->setValue("RTC Time Adjusted.");
          pCharacteristic->notify();
          firstDate();
          nextDate();
        }
        else
        {
          Serial.println("Invalid Command");
          pCharacteristic->setValue("[Error]Invalid Command");
          pCharacteristic->notify();
        }
      }
      cmdF = false;
      pref.end();
    }
  };
  /** Called before notification or indication is sent,
   *  the value can be changed here before sending if desired.
   */
  void onNotify(NimBLECharacteristic *pCharacteristic)
  {
    Serial.println("Sending notification to clients");
  };

  /** The status returned in status is defined in NimBLECharacteristic.h.
   *  The value returned in code is the NimBLE host return code.
   */
  void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
  {
    String str = ("Notification/Indication status code: ");
    str += status;
    str += ", return code: ";
    str += code;
    str += ", ";
    str += NimBLEUtils::returnCodeToString(code);
    Serial.println(str);
  };

  void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
  {
    String str = "Client ID: ";
    str += desc->conn_handle;
    str += " Address: ";
    str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
    switch (subValue)
    {
    case 0:
      str += " Unsubscribed to ";
      break;

    case 1:
      str += " Subscribed to notfications for ";
      break;

    case 2:
      str += " Subscribed to indications for ";
      break;

    case 3:
      str += " Subscribed to notifications and indications for ";
      break;

    default:
      break;
    }
    str += std::string(pCharacteristic->getUUID()).c_str();

    Serial.println(str);
  };
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks
{
  void onWrite(NimBLEDescriptor *pDescriptor)
  {
    std::string dscVal((char *)pDescriptor->getValue(), pDescriptor->getLength());
    Serial.print("Descriptor witten value:");
    Serial.println(dscVal.c_str());
  };

  void onRead(NimBLEDescriptor *pDescriptor)
  {
    Serial.print(pDescriptor->getUUID().toString().c_str());
    Serial.println(" Descriptor read");
  };
};

/** Handler class for descriptor actions */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;
// class DescriptorCallbacks : public NimBLEDescriptorCallbacks
// {
//   void onWrite(NimBLEDescriptor *pDescriptor)
//   {
//     std::string dscVal((char *)pDescriptor->getValue(), pDescriptor->getLength());
//     Serial.print("Descriptor witten value:");
//     Serial.println(dscVal.c_str());
//   };

//   void onRead(NimBLEDescriptor *pDescriptor)
//   {
//     Serial.print(pDescriptor->getUUID().toString().c_str());
//     Serial.println(" Descriptor read");
//   };
// };

/************************************************* FUNCTION DECLARATION ***********************************************************************************/
void print_wakeup_reason()
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
}

void renamecFile()
{
  Serial.printf("Rename file %s to %s\r\n", cvalue, pvalue);
  if (SPIFFS.rename(cvalue, pvalue))
    Serial.println("Data File renamed succesfully");
  else
  {
    Serial.println("Failed to rename data file");
    pCharacteristic->setValue("[Error]RCF"); // Rename current file
    pCharacteristic->notify();
  }
}

void deletenDate()
{
  Serial.printf("Deleting file: %s\r\n", ndate);
  if (SPIFFS.remove(ndate))
    Serial.println("Succesfully deleted next date file");
  else
  {
    Serial.println("Failed to delete next date file");
    pCharacteristic->setValue("[Error]DNDF"); // Delete first date file
    pCharacteristic->notify();
  }
}

void deletefDate()
{
  Serial.printf("Deleting file: %s\r\n", fdate);
  if (SPIFFS.remove(fdate))
    Serial.println("Succesfully deleted first date file");
  else
  {
    Serial.println("Failed to delete first date file");
    pCharacteristic->setValue("[Error]DFDF"); // Delete first date file
    pCharacteristic->notify();
  }
}

void deletepFile()
{
  Serial.printf("Deleting file: %s\r\n", pvalue);
  if (SPIFFS.remove(pvalue))
    Serial.println("Succesfully deleted previous data file");
  else
  {
    Serial.println("Failed to delete previous data file");
    pCharacteristic->setValue("[Error]DPF"); // delete previous file
    pCharacteristic->notify();
  }
}

void readpFile()
{
  const char *pvalue = "/Previous_Value.txt";
  Serial.printf("Reading file: %s\r\n", pvalue);
  size_t len;
  size_t bytesRead = 0;
  File file = SPIFFS.open(pvalue);
  if (!file || file.isDirectory())
  {
    Serial.println("− failed to open file for reading");
    return;
  }
  Serial.println("Syncing Data....:");
  len = file.size();
  while (file.available())
  {
    int l = file.readBytesUntil('\n', buf, (sizeof(buf) - 1));
    bytesRead += (l + 1);
    buf[l] = 0;
    Serial.println(buf);
    delay(200);
  }
  tBytes = SPIFFS.totalBytes();
  Serial.print("Total bytes: ");
  Serial.println(tBytes);
  Serial.print("File Size: ");
  Serial.println(file.size());
  file.close();
}

void rtcmodule()
{
  while (!rtc.begin())
    Serial.println("Couldn't find RTC");
  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2021, 9, 27, 11, 55, 50));
}

void appendcFile()
{
  const char *cvalue = "/Current_Value.txt";
  Serial.printf("Appending to file: %s\r\n", cvalue);
  File file = SPIFFS.open(cvalue, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open");
    return;
  }
  if (file.println(txdata))
  {
    Serial.print("Data saved in cfile: ");
    Serial.println(txdata);
  }
  else
    Serial.println("Data saved failed!");
  file.close();
}

void nextDate()
{
  // DateTime now = rtc.now();
  nday = fday;
  if (fmonth == 12)
    nmonth = 01;
  else
    nmonth = fmonth + 1;
  if (fmonth == 12)
    nyear = fyear + 1;
  else
    nyear = fyear;
  sprintf(nDate, "%02d/%02d/%02d", nday, nmonth, nyear);
  // String nDateInfo = String(nday)+"/"+String(nmonth)+"/"+String(nyear);
  // nDateInfo.toCharArray(nDate, 11);
  Serial.print("Next Date Written in File: ");
  Serial.println(nDate);

  Serial.printf("Writing to file: %s\r\n", ndate);
  File file = SPIFFS.open(ndate, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open next date file");
    pCharacteristic->setValue("[Error]FoNDF"); // Failed to open next date file
    pCharacteristic->notify();
    return;
  }
  if (file.println(nDate))
    Serial.println("Next date write success");
  else
  {
    Serial.println("Next date write failed");
    pCharacteristic->setValue("[Error]NDWF"); // First date write failed
    pCharacteristic->notify();
  }
  file.close();
}

String readnDate()
{
  const char *ndate = "/Next_Date.txt";
  Serial.printf("Reading file: %s\r\n", ndate);
  File file = SPIFFS.open(ndate);
  if (!file || file.isDirectory())
  {
    Serial.println("− failed to open file for reading");
    pCharacteristic->setValue("[Error]ND Read");
    pCharacteristic->notify();
  }
  Serial.println("Syncing Next Date....");
  pCharacteristic->setValue("Syncing Next Date...");
  pCharacteristic->notify();
  if (file.available())
  {
    int r = file.readBytesUntil('\n', readnextDate, (sizeof(readnextDate) - 1));
    readnextDate[r] = 0;
    Serial.println(readnextDate);
    pCharacteristic->setValue(readnextDate);
    pCharacteristic->notify();
    delay(10);
  }
  else
  {
    Serial.println("setting first and next date");
    pCharacteristic->setValue("[Error]setting first and next date.");
    pCharacteristic->notify();
    firstDate();
    nextDate();
  }
  file.close();
  return readnextDate;
}

void rtcTime()
{
  DateTime now = rtc.now();
  sprintf(timeStamp, "%02d/%02d/%02d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
  // Serial.print("Date/Time: ");
  // Serial.println(timeStamp);
}

String currentDate()
{
  DateTime now = rtc.now();
  cday = now.day();
  cmonth = now.month();
  cyear = now.year();

  sprintf(cDate, "%02d/%02d/%02d", cday, cmonth, cyear);
  // String cDateInfo = String(cday)+"/"+String(cmonth)+"/"+String(cyear);
  // cDateInfo.toCharArray(cDate, 11);
  Serial.print("Current Date: ");
  Serial.println(cDate);
  return cDate;
}

void firstDate()
{
  DateTime now = rtc.now();
  fday = now.day();
  fmonth = now.month();
  fyear = now.year();

  sprintf(fDate, "%02d/%02d/%02d", fday, fmonth, fyear);
  // String fDateInfo = String(fday)+"/"+String(fmonth)+"/"+String(fyear);
  // fDateInfo.toCharArray(fDate, 11);
  Serial.print("First Date Written in File: ");
  Serial.println(fDate);

  Serial.printf("Writing to file: %s\r\n", fdate);
  File file = SPIFFS.open(fdate, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open first date file");
    pCharacteristic->setValue("[Error]FoFDF"); // Failed to open first date file
    pCharacteristic->notify();
    return;
  }
  if (file.println(fDate))
    Serial.println("First date write success");
  else
  {
    Serial.println("First date write failed");
    pCharacteristic->setValue("[Error]FDWF"); // First date write failed
    pCharacteristic->notify();
  }
  file.close();
}

void bleconf()
{
  // Create the NimBLE Device
  NimBLEDevice::init(Device_Name);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

  // Setting up the server
  pServer = NimBLEDevice::createServer(); // Create the NimBLE Server
  pServer->setCallbacks(new ServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);                                                                                                 // Create the NimBLE Service
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY); // Create a NimBLE Characteristic
  pCharacteristic->setValue(SERVICE_UUID);
  pCharacteristic->setCallbacks(&chrCallbacks);

  NimBLE2904 *pBeef2904 = (NimBLE2904 *)pCharacteristic->createDescriptor("2904");
  pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
  pBeef2904->setCallbacks(&dscCallbacks);
  pService->start(); /** Start the services when finished creating all Characteristics and Descriptors */

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); /** Add the services to the advertisment data **/
  /** If your device is battery powered you may consider setting scan response
   *  to false as it will extend battery life at the expense of less data sent.
   */
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x00); // set value to 0x00 to not advertise this parameter
  pAdvertising->start();
  NimBLEDevice::startAdvertising();

  Serial.println("Waiting a client connection to notify...");
}

float readhtu21dTemp()
{
  Wire.beginTransmission(htu21daddress);
  Wire.write(htu21dtemp);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(htu21daddress, 3); // Wire.requestFrom(address, quantity)

  byte msb = Wire.read();
  byte lsb = Wire.read();
  int t = (msb << 8) + lsb;
  // int crc = Wire.read();
  Wire.read();
  return (-46.85 + 175.72 * (t / 65536.0));
}

float readhtu21dHumidity()
{
  Wire.beginTransmission(htu21daddress);
  Wire.write(htu21dhumid);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(htu21daddress, 3); // Wire.requestFrom(address, quantity in byte)

  byte msb = Wire.read();
  byte lsb = Wire.read();
  int h = (msb << 8) + lsb;
  // int crc = Wire.read();
  Wire.read();

  return (-6 + 125 * (h / 65536.0));
}

double luxread()
{
  delay(2000);
  double lux;   // Resulting lux value
  boolean good; // True if neither sensor is saturated
  unsigned int data0, data1;
  if (light.getData(data0, data1))
    return (light.getLux(gain, ms, data0, data1, lux));
  return 0;
}

void luxhtu()
{
  // Initialize the SFE_TSL2561 library
  light.begin();    // You can pass nothing to light.begin() for the default I2C address (0x39),
  unsigned char ID; // Get factory ID from sensor:
  if (light.getID(ID))
  {
    Serial.print("Got factory ID: 0X");
    Serial.print(ID, HEX);
    Serial.println(", should be 0X5X");
  }
  // The light sensor has a default integration time of 402ms,
  // and a default gain of low (1X).
  gain = 0;
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration
  unsigned char time = 2;
  // setTiming() will set the third parameter (ms) to the
  // requested integration time in ms (this will be useful later):
  Serial.println("Set timing...");
  light.setTiming(gain, time, ms);
  // To start taking measurements, power up the sensor:
  Serial.println("Powerup...");
  light.setPowerUp();
  // The sensor will now gather light during the integration time.
  // After the specified time, you can retrieve the result from the sensor.
  // Once a measurement occurs, another integration period will start.
}

/************************************************************************************************************************************/
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  rtcmodule();
  bleconf(); // calling NimBLE setup function
  luxhtu();  // calling light function
  Serial.println("---- I2C Communication ----");
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
}

void loop()
{
  print_wakeup_reason(); // Print the wakeup reason for ESP32
  rtcTime();

  // Read Sensors
  float float_temp = readhtu21dTemp();
  float float_humi = readhtu21dHumidity();
  double double_lum = luxread();
  char txtemp[6], txlux[8], txhumi[6];
  dtostrf(float_temp, 1, 2, txtemp);
  dtostrf(float_humi, 1, 2, txhumi);
  dtostrf(double_lum, 1, 2, txlux);
  sprintf(Ntype, "THL %s", id);
  String ble_message = String(Ntype) + ' ' + String(timeStamp) + ' ' + String(txtemp) + ' ' + String(txhumi) + ' ' + String(txlux);
  Serial.println("Message: " + ble_message);
  ble_message.toCharArray(txdata, ble_message.length());

  if (currentDate() == readnDate())
  {
    Serial.println("Today Current Date and Next Date are equal");
    Serial.println("First, delete previous data file from storage");
    deletepFile();
    delay(10);
    Serial.println("Now, Rename current data file into previous data file");
    renamecFile();
    delay(10);
    Serial.println("Now, Delete First date file from storage");
    deletefDate();
    delay(10);
    Serial.println("Now, Delete Next date file from storage");
    deletenDate();
    delay(10);
    Serial.println("Now, Assign First date file into storage");
    firstDate();
    delay(10);
    Serial.println("Now, Assign Next date file into storage");
    nextDate();
  }
  else
    appendcFile();

  if (deviceConnected)
  {
    if (bleCon)
    {
      Serial.println("APP Connected with Node Successfuly");
      Serial.println("Now It's time to rename current data file to previous data file");
      renamecFile();
      delay(10);
      Serial.println("Now It's time to read previous file");
      readpFile();
      delay(10);
      Serial.println("Previous file read Successfuly!! Now its time to delete file");
      deletepFile();
      Serial.println("Previous file delete Successfuly");
      delay(10);
      /*
      pCharacteristic->setValue(fDate);
      pCharacteristic->notify();
      Serial.print("NimBLE First Date: ");
      Serial.println(fDate);
      delay(200);
      */
      pCharacteristic->setValue(readnextDate);
      pCharacteristic->notify();
      Serial.print("NimBLE Next Date: ");
      Serial.println(readnextDate);
      delay(10);
      bleCon = false;
    }

    Serial.print("NimBLE Transmitted Data: ");
    Serial.println(txdata);
    pCharacteristic->setValue(txdata);
    pCharacteristic->notify();
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(10);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start Advertising Again.......");
    oldDeviceConnected = deviceConnected;
    bleCon = true;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected)
    oldDeviceConnected = deviceConnected; // do stuff here on connecting
  delay(2000);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * S_TO_uS_FACTOR); // ESP32 wakes up every 5 seconds
  Serial.println("Going to deep-sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}
