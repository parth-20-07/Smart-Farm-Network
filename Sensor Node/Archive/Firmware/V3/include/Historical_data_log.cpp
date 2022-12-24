#ifndef Historical_data_log_cpp
#define Historical_data_log_cpp
#include <Arduino.h>
#include "Pin_Connection.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ReadLines.h>
#include "NimBLE_Configuration.cpp"

/* -------------------------------------------------------------------------- */
/*                            VARIABLE DECLARATION                            */
/* -------------------------------------------------------------------------- */
#define BACKUP_FILE "/Backup.txt"

/* -------------------------------------------------------------------------- */
/*                             FUNCTION DEFINITION                            */
/* -------------------------------------------------------------------------- */
void setup_data_logger(void);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void deleteFile(fs::FS &fs, const char *path);
void handleEachLine(char line[], int lineIndex);
void read_historical_data(void);
void write_historical_data(String msg);

/* -------------------------------------------------------------------------- */
/*                            FUNCTION DECLARATION                            */
/* -------------------------------------------------------------------------- */
void setup_data_logger(void)
{
    if (!SD.begin(SD_CHIP_SELECT))
    {
        Serial.println("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
        Serial.println("MMC");
    else if (cardType == CARD_SD)
        Serial.println("SDSC");
    else if (cardType == CARD_SDHC)
        Serial.println("SDHC");
    else
        Serial.println("UNKNOWN");

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    Serial.println("SD Setup Complete");
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
        Serial.println("File written");
    else
        Serial.println("Write failed");
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        writeFile(SD, path, message);
        return;
    }
    if (file.print(message))
        Serial.println("Message appended");
    else
        Serial.println("Append failed");
    file.close();
}

void deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path))
        Serial.println("File deleted");
    else
        Serial.println("Delete failed");
}

void handleEachLine(char line[], int lineIndex)
{
    Serial.println((String)lineIndex + " : " + (String)line);
    if (!send_data_via_ble((String)line, false, false))
        ESP.restart();
    delay(5000);
}

void read_historical_data(void)
{
    Serial.println("Reading Historical Data");
    RL.readLines(BACKUP_FILE, &handleEachLine);
    deleteFile(SD, BACKUP_FILE);
}

void write_historical_data(String msg)
{
    Serial.println("Saving Historical Data");
    msg += "\n";
    char char_message[msg.length()];
    strcpy(char_message, msg.c_str());
    appendFile(SD, BACKUP_FILE, char_message);
}

#endif