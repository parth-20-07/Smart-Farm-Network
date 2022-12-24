/*|---------------------------------------------------------------------------------------|
                    Insterion of TAB Function.h for all the Functions
  |---------------------------------------------------------------------------------------|*/
#include "functions.h"

/*|---------------------------------------------------------------------------------------|
                                SETUP STARTS HERE
  |---------------------------------------------------------------------------------------|*/

void setup()
{
  Serial.begin(115200);
  softSerial.begin(9600); // baud rate for Software Serial
  EEPROM.begin(4096);

  /*------ 1.Defining the interrupt------*/

  pinMode(INTR_PIN, INPUT);
  attachInterrupt(INTR_PIN, CONFIG_INTR, RISING);

  /*------- 2.Setting up the OTA------------*/
  /*
  sr.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
   });

  AsyncElegantOTA.begin(&sr);    // Start ElegantOTA
  sr.begin();
  Serial.println("HTTP server started");
  */

  /*------- 3.RST_PIN i.e reset pin------------*/

  pinMode(RST_PIN, OUTPUT);
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  digitalWrite(RST_PIN, HIGH);

  /*------- 4. Interrupt Calling for Config Mode------------*/

  if (intr_check == 1) // For configuration
  {
    CONFIG_MODE();
    intr_check = 0;
  }

  /*------- 5. Configuration Sending for Mega ------------*/

  //  int x = EEPROM.read(134);
  //  if (x == 1)
  //  {
  Serial.println("SENDING CONFIG TO MEGA");
  while (millis() < 1000)
  {
    SHARE_CONFIG();
    count++;
  }
  //}

  Serial.println("NO CONFIG FOR MEGA");
  EEPROM.write(134, 0);
  EEPROM.commit();

  hasWifi = false;
  STATION();
  if (!hasWifi)
  {
    return;
  }
}

/*|---------------------------------------------------------------------------------------|
                              LOOP FUNCTION STARTS HERE
  |---------------------------------------------------------------------------------------|*/

void loop()
{
  // AsyncElegantOTA.loop();    // For OTA
  //
  if (intr_check == 1) // For configuration
  {
    CONFIG_MODE();
    intr_check = 0;
  }

  if (softSerial.available()) // Node data from Due
  {
    MEGA_TO_ESP();
  }

  else
  {
    delay(500);
    Serial.print(".");
  }
}

/*|---------------------------------------------------------------------------------------|
                              CODE ENDS HERE
  |---------------------------------------------------------------------------------------|*/
