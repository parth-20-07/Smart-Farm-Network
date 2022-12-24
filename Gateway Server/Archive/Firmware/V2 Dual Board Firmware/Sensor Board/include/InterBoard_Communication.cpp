#include <Arduino.h>
#include "HardwareSerial.h"
#include "Pin_Connection.h"
HardwareSerial &hSerial = Serial1;

void setup_communication_between_boards(void)
{
    hSerial.begin(9600);
    delay(1000);
    digitalWrite(SERVER_BOARD_RST_PIN, LOW);
    delay(50);
    digitalWrite(SERVER_BOARD_RST_PIN, HIGH);
    delay(50);
    Serial.println("hSerial Setup Complete");
}

bool send_message(String msg)
{
    Serial.println("Trying to send msg");
    digitalWrite(SENSOR_BOARD_CONTROL_PIN, LOW);
    delay(50);
    uint32_t lastMillis = millis();
    while ((millis() - lastMillis) < 2000)
        if (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
            break;
    if (digitalRead(SERVER_BOARD_CONTROL_PIN) == HIGH)
    {
        Serial.println("Send Failure");
        digitalWrite(SENSOR_BOARD_CONTROL_PIN, HIGH);
        return false;
    }
    msg += '#';
    uint8_t msg_length = msg.length();
    char char_msg[msg_length];
    strcpy(char_msg, msg.c_str());
    Serial.println("Writing Message: " + (String)char_msg);
    for (size_t i = 0; i < msg.length(); i++)
    {
        if (hSerial.available())
            hSerial.write(char_msg[i]);
    }
    Serial.println("Send Success");
    digitalWrite(SENSOR_BOARD_CONTROL_PIN, HIGH);
    return true;
}
String read_message(void)
{
    Serial.println("Trying to read msg");
    digitalWrite(SENSOR_BOARD_CONTROL_PIN, LOW);
    delay(50);
    String msg = "";
    if (hSerial.available())
        msg = hSerial.readStringUntil('#');
    Serial.println("Recieved Message: " + msg);
    while (digitalRead(SERVER_BOARD_CONTROL_PIN) == LOW)
        ;
    delay(10);
    digitalWrite(SENSOR_BOARD_CONTROL_PIN, HIGH);
    delay(10);
    return msg;
}