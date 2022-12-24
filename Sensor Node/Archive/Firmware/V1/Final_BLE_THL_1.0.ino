#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <Wire.h>
#include <SparkFunTSL2561.h>
#include "RTClib.h"
#include "SPIFFS.h"
#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "FS.h"

#define SERVICE_UUID "208fc8fc-64ed-4423-ba22-2230821ae406"
#define CHARACTERISTIC_UUID "e462c4e9-3704-4af8-9a20-446fa2eef1d0"

#define htu21daddress B1000000 // 0x40 in HEXA, 64 in DEC
#define htu21dtemp B11110011   // 0xF3 in HEXA, 243 in DEC
#define htu21dhumid B11110101  // 0xF5 in HEXA, 245 in DEC
#define FORMAT_SPIFFS_IF_FAILED false

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 300         /* Time ESP32 will go to sleep (in seconds) */

Preferences pref;
SFE_TSL2561 light;
RTC_DS3231 rtc;

float A, B; // A-->Temperature, B-->Humidity
double C;   // C-->Luminosity

char cmdKey = 'X';
bool cmdF = false; // Flag to indicate command received from mobile app
byte cmdBuff[4];   // Buffer to temporary store information sent from mobile App
byte i = 0;
char delm = ','; // CSV format using comma as delimeter

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

NimBLEServer *pServer = NULL;
NimBLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool bleCon = true;

// For 2561
boolean gain;    // Gain setting, 0 = X1, 1 = X16;
unsigned int ms; // Integration time in milliseconds

//---------------------------------------Function Definition-----------------------------------

void bleconf();
void luxhtu();
void rtcTime();
void rtcmodule();
void appendcFile();
void readpFile();
void deletepFile();
void deletefDate();
void deletenDate();
void renamecFile();
void firstDate();
void nextDate();

/************************************************************************************************************************************/

class MyServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    deviceConnected = true;
    NimBLEDevice::startAdvertising();
  };

  void onDisconnect(NimBLEServer *pServer)
  {
    deviceConnected = false;
  }
};

/************************************************************************************************************************************/

class MyCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
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
  }
};

/************************************************************************************************************************************/

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  rtcmodule();
  bleconf(); // calling NimBLE setup function
  luxhtu();  // calling light function
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("---- I2C Communication ----");
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
}

/************************************************************************************************************************************/

void bleconf()
{
  // Create the NimBLE Device
  NimBLEDevice::init("NimBLE_Node");

  *pServer = NimBLEDevice::createServer();           // Create the NimBLE Server
  *pService = pServer->createService(SERVICE_UUID);  // Create the NimBLE Service
  *pCharacteristic = pService->createCharacteristic( // Create a NimBLE Characteristic
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY);

  pServer->setCallbacks(new MyServerCallbacks());
  pCharacteristic->addDescriptor(new NimBLE2902()); // Create a NimBLE Descriptor
  pCharacteristic->setCallbacks(new MyCallbacks()); // for write new value
  pService->start();                                // Start the service

  // Start advertising
  // NimBLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x00); // set value to 0x00 to not advertise this parameter
  NimBLEDevice::startAdvertising();

  NimBLEService::start();
  Serial.println("Waiting a client connection to notify...");
}

/************************************************************************************************************************************/

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
  int crc = Wire.read();

  float temperature = -46.85 + 175.72 * (t / 65536.0);
  return temperature;
}

/************************************************************************************************************************************/

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
  int crc = Wire.read();

  float humidity = -6 + 125 * (h / 65536.0);
  return humidity;
}

/************************************************************************************************************************************/

double luxread()
{
  delay(2000);
  double lux;   // Resulting lux value
  boolean good; // True if neither sensor is saturated
  unsigned int data0, data1;
  if (light.getData(data0, data1))
  {
    good = light.getLux(gain, ms, data0, data1, lux);
    return lux;
  }
}

/************************************************************************************************************************************/

void luxhtu()
{
  // Initialize the SFE_TSL2561 library

  // You can pass nothing to light.begin() for the default I2C address (0x39),

  light.begin();

  // Get factory ID from sensor:

  unsigned char ID;

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

void loop()
{
  print_wakeup_reason(); // Print the wakeup reason for ESP32

  rtcTime();

  A = readhtu21dTemp();
  char txtemp[6];
  dtostrf(A, 1, 2, txtemp);

  B = readhtu21dHumidity();
  char txhumi[6];
  dtostrf(B, 1, 2, txhumi);

  C = luxread();
  char txlux[8];
  dtostrf(C, 1, 2, txlux);

  sprintf(Ntype, "THL %s", id);
  String t = String(Ntype) + ' ' + String(timeStamp) + ' ' + String(txtemp) + ' ' + String(txhumi) + ' ' + String(txlux);
  t.toCharArray(txdata, 53);

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
  {
    appendcFile();
  }

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
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
  delay(10000);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // ESP32 wakes up every 5 seconds

  Serial.println("Going to deep-sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

/************************************************************************************************************************************/

void rtcTime()
{
  DateTime now = rtc.now();
  sprintf(timeStamp, "%02d/%02d/%02d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
  // Serial.print("Date/Time: ");
  // Serial.println(timeStamp);
}

/************************************************************************************************************************************/

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

/************************************************************************************************************************************/

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
  {
    Serial.println("First date write success");
  }
  else
  {
    Serial.println("First date write failed");
    pCharacteristic->setValue("[Error]FDWF"); // First date write failed
    pCharacteristic->notify();
  }
  file.close();
}

/************************************************************************************************************************************/

void nextDate()
{
  // DateTime now = rtc.now();
  nday = fday;
  if (fmonth == 12)
  {
    nmonth = 01;
  }
  else
  {
    nmonth = fmonth + 1;
  }
  if (fmonth == 12)
  {
    nyear = fyear + 1;
  }
  else
  {
    nyear = fyear;
  }
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
  {
    Serial.println("Next date write success");
  }
  else
  {
    Serial.println("Next date write failed");
    pCharacteristic->setValue("[Error]NDWF"); // First date write failed
    pCharacteristic->notify();
  }
  file.close();
}

/************************************************************************************************************************************/

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

/************************************************************************************************************************************/

void rtcmodule()
{
  while (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
  }
  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:

  // rtc.adjust(DateTime(2021, 9, 27, 11, 55, 50));
}

/************************************************************************************************************************************/

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
  {
    Serial.println("Data saved failed!");
  }
  file.close();
}

/************************************************************************************************************************************/

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

/************************************************************************************************************************************/

void deletepFile()
{
  Serial.printf("Deleting file: %s\r\n", pvalue);
  if (SPIFFS.remove(pvalue))
  {
    Serial.println("Succesfully deleted previous data file");
  }
  else
  {
    Serial.println("Failed to delete previous data file");
    pCharacteristic->setValue("[Error]DPF"); // delete previous file
    pCharacteristic->notify();
  }
}

/************************************************************************************************************************************/

void deletefDate()
{
  Serial.printf("Deleting file: %s\r\n", fdate);
  if (SPIFFS.remove(fdate))
  {
    Serial.println("Succesfully deleted first date file");
  }
  else
  {
    Serial.println("Failed to delete first date file");
    pCharacteristic->setValue("[Error]DFDF"); // Delete first date file
    pCharacteristic->notify();
  }
}

/************************************************************************************************************************************/

void deletenDate()
{
  Serial.printf("Deleting file: %s\r\n", ndate);
  if (SPIFFS.remove(ndate))
  {
    Serial.println("Succesfully deleted next date file");
  }
  else
  {
    Serial.println("Failed to delete next date file");
    pCharacteristic->setValue("[Error]DNDF"); // Delete first date file
    pCharacteristic->notify();
  }
}

/************************************************************************************************************************************/

void renamecFile()
{
  Serial.printf("Rename file %s to %s\r\n", cvalue, pvalue);
  if (SPIFFS.rename(cvalue, pvalue))
  {
    Serial.println("Data File renamed succesfully");
  }
  else
  {
    Serial.println("Failed to rename data file");
    pCharacteristic->setValue("[Error]RCF"); // Rename current file
    pCharacteristic->notify();
  }
}

/************************************************************************************************************************************/

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
