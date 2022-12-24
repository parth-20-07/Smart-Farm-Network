#ifndef Pin_Connection_h
#define Pin_Connection_h

// GPIO Pins
#define ESP32_GPIO_2 2
#define ESP32_GPIO_4 4
#define ESP32_GPIO_5 5 // SD Chip Select
#define ESP32_GPIO_12 12
#define ESP32_GPIO_13 13
#define ESP32_GPIO_14 14
#define ESP32_GPIO_15 15
#define ESP32_GPIO_16 16 // RX2
#define ESP32_GPIO_17 17 // TX2
#define ESP32_GPIO_25 25
#define ESP32_GPIO_26 26
#define ESP32_GPIO_27 27
#define ESP32_GPIO_32 32
#define ESP32_GPIO_33 33
#define ESP32_GPIO_34 34 // External Wakeup Button
#define ESP32_GPIO_35 35 // Battery Input Pin
#define ESP32_GPIO_39 39
#define ESP32_GPIO_36 36

// Communication Pins
#define ESP_SPI_SCK 18  // SPI Clock
#define ESP_SPI_MISO 19 // SPI Master In Slave Out
#define ESP_SPI_MOSI 23 // SPI Master Out Slave In
#define ESP_I2C_SDA 21  // I2C Serial Data
#define ESP_I2C_SCL 22  // T2C Serial Clock

/* -------------------------------------------------------------------------- */
/*                                HTU21D SENSOR                               */
/* -------------------------------------------------------------------------- */
// VCC = 3.3V
// GND = GND
// SDA = ESP_I2C_SDA
// SCL = ESP_I2C_SCL

/* -------------------------------------------------------------------------- */
/*                              MH - Z19C SENSOR                              */
/* -------------------------------------------------------------------------- */
// VCC = 3.3V
// GND = GND
// RX2 = ESP_GPIO_16
// TX2 = ESP_GPIO_17

/* -------------------------------------------------------------------------- */
/*                               TSL2561 SENSOR                               */
/* -------------------------------------------------------------------------- */
// Hookup Guide: https://learn.sparkfun.com/tutorials/getting-started-with-the-tsl2561-luminosity-sensor
// VCC = 3.3V
// GND = GND
// SDA = ESP_I2C_SDA
// SCL = ESP_I2C_SCL
// ADDR can be connected to ground, or vdd or left floating to change the i2c address

/* -------------------------------------------------------------------------- */
/*                               SD CARD MODULE                               */
/* -------------------------------------------------------------------------- */
// 3V3	= 3.3V
// MOSI	= ESP_SPI_MOSI
// CLK	= ESP_SPI_SCK
// MISO	= ESP_SPI_MISO
// GND	= GND
#define SD_CHIP_SELECT ESP32_GPIO_5

/* -------------------------------------------------------------------------- */
/*                              BUTTON CONNECTION                             */
/* -------------------------------------------------------------------------- */
#define WAKEUP_BUTTON ESP32_GPIO_34
#define BATTERY_INPUT_ADC ESP32_GPIO_35
#endif