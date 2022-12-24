#include <Arduino.h>
#include "HardwareSerial.h"
#include "Pin_Connection.h"
HardwareSerial &hSerial = Serial1;

void setup_communication_between_boards(void)
{
    hSerial.begin(9600);
}

bool send_message(String msg)
{
    Serial.println("Trying to send msg");
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    delay(10);
    uint32_t last_millis_to_send_msg = millis();
    while ((millis() - last_millis_to_send_msg) < 2000)
        if (digitalRead(SENSOR_BOARD_CONTROL_PIN) == LOW)
            break;
    if (digitalRead(SENSOR_BOARD_CONTROL_PIN) == HIGH)
    {
        Serial.println("Send Failure");
        digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
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
    digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
    return true;
}

String read_message(void)
{
    Serial.println("Trying to read msg");
    digitalWrite(SERVER_BOARD_CONTROL_PIN, LOW);
    delay(10);
    String msg = "";
    if (hSerial.available())
        msg = hSerial.readStringUntil('#');
    Serial.println("Recieved Message: " + msg);
    while (digitalRead(SENSOR_BOARD_CONTROL_PIN) == LOW)
        ;
    delay(10);
    digitalWrite(SERVER_BOARD_CONTROL_PIN, HIGH);
    delay(10);
    return msg;
}