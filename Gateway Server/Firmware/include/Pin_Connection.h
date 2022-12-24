#ifndef Pin_Connection_h
#define Pin_Connection_h
#include "Arduino.h"

// GPIO Pins
#define ESP32_GPIO_2 2
#define ESP32_GPIO_4 4
#define ESP32_GPIO_5 5 // SD Card Chip Select
#define ESP32_GPIO_12 12
#define ESP32_GPIO_13 13 // RST Pin for Server Board
#define ESP32_GPIO_14 14
#define ESP32_GPIO_15 15
#define ESP32_GPIO_16 16
#define ESP32_GPIO_17 17
#define ESP32_GPIO_25 25
#define ESP32_GPIO_26 26
#define ESP32_GPIO_27 27
#define ESP32_GPIO_32 32 // Configuration Button
#define ESP32_GPIO_33 33
#define ESP32_GPIO_34 34 // Battery Input Pin
#define ESP32_GPIO_35 35 // Charger Input Pin
#define ESP32_GPIO_39 39
#define ESP32_GPIO_36 36

// Communication Pins
#define ESP_SPI_SCK 18  // SPI Clock
#define ESP_SPI_MISO 19 // SPI Master In Slave Out
#define ESP_SPI_MOSI 23 // SPI Master Out Slave In
#define ESP_I2C_SDA 21  // I2C Serial Data
#define ESP_I2C_SCL 22  // T2C Serial Clock

/* ----------------------------- SD CARD MODULE ----------------------------- */
// 3V3	= 3.3V
// MOSI	= ESP_SPI_MOSI
// CLK	= ESP_SPI_SCK
// MISO	= ESP_SPI_MISO
// GND	= GND
#define SD_CHIP_SELECT ESP32_GPIO_5

/* ---------------------------- INTERFACE BUTTON ---------------------------- */
#define CONFIGURATION_BUTTON ESP32_GPIO_32

/* ---------------------------- POWER CONNECTION ---------------------------- */
#define BATTERY_INPUT ESP32_GPIO_34
#define CHARGER_INPUT ESP32_GPIO_35

void setup_pins(void)
{
    pinMode(BATTERY_INPUT, INPUT);
    pinMode(CHARGER_INPUT, INPUT);
    pinMode(CONFIGURATION_BUTTON, INPUT);
    Serial.println("Pins Setup Complete");
}

#endif