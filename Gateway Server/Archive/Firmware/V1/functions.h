

/*|---------------------------------------------------------------------------------------|
                     Insterion of TAB pin.h for all the pins
  |---------------------------------------------------------------------------------------|*/

#include "pin.h"

void reset_esp(void);
IRAM_ATTR void CONFIG_INTR(void);

void SAVE_DATA_SD(void);
void RECONNECT_LOCAL(void);
void RECONNECT_GLOBAL(void);
void ESP_TO_CLOUD(void);
void MEGA_TO_ESP(void);
void STATION(void);
void HANDLE_ROOT(void);
void SAVE_CONFIG(void);
void SHARE_CONFIG();
void CONFIG_MODE(void);
void ST_TO_CLOUD(void);

/*
 * COLORS
 *
       RGB_color(255, 0, 0); // Red
       RGB_color(0, 255, 0); // Green
       RGB_color(0, 0, 255); // Blue
       RGB_color(255, 255, 125); // Raspberry
       RGB_color(0, 255, 255); // Cyan
       RGB_color(255, 0, 255); // Magenta
       RGB_color(255, 255, 0); // Yellow
       RGB_color(255, 255, 255); // White

       1. Connecting to Wifi      : blinking blue
       2. Connecting to server    : blinking blue
       3. No wifi                 : orange
       4. No server               : orange
       5. Sending data to server  : green
       6. Sending data to gateway : blinking green
       7. Established connection  : blue
          - Wifi connection
          - Server connection
*/

/*______________________Main RGB Function_____________________________________*/

void color_palette(int red_light_value, int green_light_value, int blue_light_value, int delayTime)
{
  analogWrite(red_pin, red_light_value);
  analogWrite(green_pin, green_light_value);
  analogWrite(blue_pin, blue_light_value);
}

void OFF(int delayTime)
{
  digitalWrite(red_pin, LOW);
  digitalWrite(green_pin, LOW);
  digitalWrite(blue_pin, LOW);
  delay(delayTime);
}

void show_connecting_to_wifi()
{
  color_palette(255, 255, 0, 250); // blinking blue
  OFF(250);
}

void show_connecting_to_server()
{
  color_palette(255, 255, 0, 250); // blinking blue
  OFF(250);
}

void show_no_wifi()
{
  color_palette(0, 90, 255, 500); // orange
}

void show_no_server()
{
  color_palette(0, 90, 255, 500); // orange
}

void show_data_to_server()
{
  color_palette(255, 0, 255, 500); // green
}

void show_data_to_gateway()
{
  color_palette(82, 25, 25, 250); // blinking green
  OFF(250);
}

void show_connection_established()
{
  color_palette(255, 255, 0, 500); // blue
}

/*|---------------------------------------------------------------------------------------|
                                ESP32 RESET FUNCTION
  |---------------------------------------------------------------------------------------|*/

void reset_esp()
{
  digitalWrite(RST_PIN, LOW);
}

/*|---------------------------------------------------------------------------------------|
                        Intrupt - 1 - To Configuration Mode
  |---------------------------------------------------------------------------------------|*/

void IRAM_ATTR CONFIG_INTR()
{
  intr_check = 1;
}

/*|---------------------------------------------------------------------------------------|
                    SAVE_DATA - [SD CARD] - To be edited for SD CARD
  |---------------------------------------------------------------------------------------|*/

void SAVE_DATA_SD()
{
  EEPROM.put(addr * 2, nodeID);
  addr++;
  EEPROM.put(addr * 4, temperature);
  addr++;
  EEPROM.put(addr * 4, humidity);
  addr++;
  EEPROM.put(addr * 4, VPD);
  addr++;
  EEPROM.put(addr * 4, batt_level);
  addr++;

  end_addr = addr;
}

/*|---------------------------------------------------------------------------------------|
                        Reconnet Function [In case of Wi-Fi Loss]
  |---------------------------------------------------------------------------------------|*/

void RECONNECT_LOCAL()
{

  char ssid1[sizes + 1];
  char pass1[sizep + 1];

  if (WiFi.status() != WL_CONNECTED)
  {
    presentTime = millis();
    while (millis() - presentTime < 8000) // scans for wifi for 2 seconds
    {
      WiFi.begin(ssid1, pass1);
      // show_connecting_to_wifi();
    }
    if (WiFi.status() != WL_CONNECTED)
      j = 3;
  }
  else
  {
    j = 6; // if connected exits the function altogether
    Serial.println("Wi-Fi is stable");
    // show_connection_established();
  }

  if (j == 3)
  {
    SAVE_DATA_SD();
    Serial.println("Saving data into EEPROM...");
  }
}

/*|---------------------------------------------------------------------------------------|
                        Reconnet Function [In case of Internet Loss]
  |---------------------------------------------------------------------------------------|*/

void RECONNECT_GLOBAL()
{
  if (!tb.connected())
  {
    presentTime = millis();
    while (millis() - presentTime < 8000) // scans for wifi for 2 seconds
    {
      show_connecting_to_server();
      tb.connect(thingsboardServer, TOKEN);
    }
    if (!tb.connected())
      j = 3;
  }
  else
  {
    j = 6; // if connected exits the function altogether
    Serial.println("Could Connection is stable");
    // show_connection_established();
  }

  if (j == 3)
  {
    SAVE_DATA_SD();
    Serial.println("Saving data into EEPROM...");
  }
}

/*|---------------------------------------------------------------------------------------|
                        Send data to cloud - Function
  |---------------------------------------------------------------------------------------|*/

void ESP_TO_CLOUD()
{
  RECONNECT_LOCAL();
  RECONNECT_GLOBAL();

  Serial.println(j);
  Serial.println(k);
  if (j == 6)
  {
    Serial.println("Sending Data..");

    tb.sendTelemetryFloat("node", nodeID);
    tb.sendTelemetryFloat("Temperature", temperature);
    tb.sendTelemetryFloat("Humidity", humidity);
    tb.sendTelemetryFloat("VPD", VPD);
    tb.sendTelemetryFloat("Battery", batt_level);
    tb.sendTelemetryFloat("ST_AC", ST_AC);
    tb.sendTelemetryFloat("ST_DE", ST_DE);

    Serial.println("Data successfully sent to Server !");
  }
}

/*|---------------------------------------------------------------------------------------|
                        RECEIVE DATA -SS [MEGA TO ESP]
  |---------------------------------------------------------------------------------------|*/

void MEGA_TO_ESP()
{
  Serial.println("Receiving data from Due");

  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, softSerial);

  if (error == DeserializationError::Ok)
  {
    nodeID = doc["n"];
    temperature = doc["t"];
    humidity = doc["h"];
    VPD = doc["v"];
    batt_level = doc["b"];

    // unix_time_c  = doc["t"];

    Serial.print("Node ID       : ");
    Serial.println(nodeID);

    Serial.print("Temperature   : ");
    Serial.println(temperature);

    Serial.print("Humidity      : ");
    Serial.println(humidity);

    Serial.print("VPD           : ");
    Serial.println(VPD);

    Serial.print("Battery Level : ");
    Serial.println(batt_level);
    ESP_TO_CLOUD();
  }
}

/*|---------------------------------------------------------------------------------------|
                        RECEIVE DATA -SS [MEGA TO ESP]
  |---------------------------------------------------------------------------------------|*/
//
// void Receive_ST()
//{
//  Serial.println("Receiving ST")
//
//  StaticJsonDocument<1000> doc;
//  DeserializationError error = deserializeJson(doc, softSerial);
//
//  if (error == DeserializationError::Ok)
//  {
//    ST_AC      = doc["ST_AC"];
//    ST_DE      = doc["ST_DE"];
//
//    ST_TO_CLOUD();
//
//  }
//}

/*|---------------------------------------------------------------------------------------|
                         STATION- Will connect to Wi-Fi -> Cloud
  |--------------------------------------------------------------------------------------|*/

void ST_TO_CLOUD()
{
  RECONNECT_LOCAL();
  RECONNECT_GLOBAL();

  Serial.print("STATUS AC: ");
  Serial.println(ST_AC);
  Serial.print("STATUS DE: ");
  Serial.println(ST_DE);
  if (j == 6)
  {
    Serial.println("Sending STATUS..");

    tb.sendTelemetryFloat("ST_AC", ST_AC);
    tb.sendTelemetryFloat("ST_DE", ST_DE);

    Serial.println("STATUS sent to Server !");
  }
}

/*|---------------------------------------------------------------------------------------|
                         STATION- Will connect to Wi-Fi -> Cloud
  |---------------------------------------------------------------------------------------|*/

void STATION()
{
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(100);

  sizes = EEPROM.read(1);
  sizep = EEPROM.read(2);

  if (sizes == 0 || sizep == 0)
  {
    Serial.println("    ");
    Serial.println("First Time Device Started");
    // FT_mode();
  }

  char ssid1[sizes + 1];
  char pass1[sizep + 1];

  for (int i = 0; i < sizes + 1; i++)
  {
    ssid1[i] = EEPROM.read(5 + i);
  }

  for (int i = 0; i < sizep + 1; i++)
  {
    pass1[i] = EEPROM.read(5 + i + sizes);
  }

  Serial.print("WIFI SSID:");
  Serial.println(ssid1);
  Serial.print("Password:");
  Serial.println(pass1);

  unsigned long wifiConnectStart = millis();
  Serial.println("Connecting...");
  WiFi.begin(ssid1, pass1);

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(500);
    Serial.print(".");

    // show_connecting_to_wifi();

    if (WiFi.status() == WL_CONNECT_FAILED)
    {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(1000);
    }

    if (intr_check == 1) // For configuration
    {
      CONFIG_MODE();
      intr_check = 0;
    }

    if (millis() - wifiConnectStart > 10000)
    {
      Serial.println(" WiFi Not Available ");
    }
  }

  hasWifi = true;
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Connecting to ThingsBoard ...");
  tb.connect(thingsboardServer, TOKEN);
  delay(1000);

  if (WiFi.status() != WL_CONNECTED && !tb.connected())
  {
    Serial.print("[FAILED]");
    WiFi.mode(WIFI_OFF); // TURN OFF WIFI
    ///////////-- data save---/////
  }
  if (WiFi.status() == WL_CONNECTED && tb.connected())
  {
    Serial.println("[SUCCESSFUL_CONNECTION]");
    digitalWrite(4, HIGH);
    // show_connection_established();
  }
}

/*|---------------------------------------------------------------------------------------|
                      Handle/HOST HTML Page - Handle Root Function
  |---------------------------------------------------------------------------------------|*/

void HANDLE_ROOT()
{
  Serial.println("HTML page started");
  String IPaddress = myIP.toString();
  String HTML = "<!DOCTYPE html><html><head><title>City Greens</title><style>body {background-color: #42D597;font-family: Georgia, 'Times New Roman', Times, serif;Color: rgb(33, 36, 39);}.header {font-family: Consolas;color: rgb(33, 36, 39);}.main {border: 1px solid rgb(255, 255, 255);border-radius: 20px;padding: 50px;width: 60%;height: auto;}label {text-align: right;padding: 10px;}.textbox, .check {border-radius: 3px;border: 1px solid rgb(255, 251, 251);margin: 10px;transition: all 0.5s;height: 20px;outline: none;padding: 5px;}.textbox:hover, .check:hover {box-shadow: 0px 0px 10px rgb(254, 255, 238);}.button {border-radius: 5px;background: rgb(254, 255, 237);border: 1px solid rgb(255, 254, 212);padding: 5px;margin: 10px;display: block;color: rgb(255, 255, 227);font-size: 16px;transition: all 0.5s;padding: 5px 10px 5px 10px;align-self: center;}.button:hover {background: rgb(255, 253, 227);color: rgba(248, 255, 145, 0.363);}input[type=checkbox] {width: 10px;height: 10px;border: 1px solid rgba(255, 245, 108, 0.349);background: rgba(244, 255, 97, 0.212);}</style> </head><body> <center>  <div class=\"header\"><h1>CITY GREENS</h1></div><div class=\"main\"><h3>Enter your details</h3><form action=\"http://" + IPaddress + "\" method=\"POST\"> <table class=\"table\"><tr><td><label for=\"ssid\">SSID:</label></td><td><input type=\"text\" name=\"ssid\" class=\"textbox\" placeholder=\"[Your Wi-Fi Name]\" SSID\" id=\"ssid\"></td></tr><tr><td><label for=\"password\">Password: </label></td><td><input type=\"password\" name=\"password\" class=\"textbox\" placeholder=\"[Your Wi-Fi Password]\" Password\" id=\"password\"></td> </tr><tr> <td><label for=\"interval\">Interval: </label></td><td><input type=\"text\" name=\"Interval\" class=\"textbox\" placeholder=\"[Desired Interval]\" Interval\" id=\"Interval\"> </td> </tr> </table> <table><h4>Alerts</h4><tr><td><label for = \"Parameters\">Parameters</label></td><td><label for = \"Minimum\">Minimum</label> </td><td><label for = \"Maximum\">Maximum</label></td></tr><tr><td><label for = \"Parameters\">Temperature</label></td><td><input type = \"text\" name=\"temperature_min\" class=\"textbook\" placeholder=\"[Temp: 15-50]\" id=\"temperature_min\"</label></td><td><input type = \"text\" name=\"temperature_max\" class=\"textbook\" placeholder=\"[Temp: 15-50]\" id=\"temperature_max\"</label> </td></tr><tr><td><label>Humidity</label></td><td><input type = \"text\" name=\"humidity_min\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id=\"humidity_min\"</label></td><td><input type = \"text\" name=\"humidity_max\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id=\"humidity_max\"</label></td></tr><tr><td><label for = \"Parameters\">CO2</label></td><td><input type = \"text\" name=\"CO2_min\" class=\"textbook\" placeholder=\"[Co2: 1-100]\" id=\"CO2_min\"</label></td><td><input type = \"text\" name=\"CO2_max\" class=\"textbook\" placeholder=\"[Co2: 1-100]\" id=\"CO2_max\"</label></td></tr><tr><td><label for = \"Parameters\">PAR</label></td><td><input type = \"text\" name=\"PAR_min\" class=\"textbook\" placeholder=\"[PAR: 1-100]\" id=\"PAR_min\"</label></td><td><input type = \"text\" name=\"PAR_max\" class=\"textbook\" placeholder=\"[PAR: 1-100]\" id=\"PAR_max\"</label></td></tr><tr><td><label>VPD</label></td><td><input type = \"text\" name=\"VPD_min\" class=\"textbook\" placeholder=\"[VPD: 0-8]\" id=\"VPD_min\"</label></td><td><input type = \"text\" name=\"VPD_max\" class=\"textbook\" placeholder=\"[VPD: 0-8]\" id=\"VPD_max\"</label></td></tr></table><table><h4>Automation</h4><tr><td><label>Devices</label></td><td><label>ON Threshold</label></td><td><label>OFF Threshold</label></td></tr></tr><tr><td><label>Fan </label></td><td><input type = \"text\" name=\"fan_min\" class=\"textbook\" placeholder=\"[TEMP: 15-50]\" id=\"fan_min\"</label></td><td><input type = \"text\" name=\"fan_max\" class=\"textbook\" placeholder=\"[TEMP: 15-50]\" id=\"fan_max\"</label></td></tr><tr><td><label>Pad</label></td><td><input type = \"text\" name=\"pad_min\" class=\"textbook\" placeholder=\"[TEMP: 15-50]\" id=\"pad_min\"</label></td><td><input type = \"text\" name=\"pad_max\" class=\"textbook\" placeholder=\"[TEMP: 15-50]\" id=\"pad_max\"</label></td></tr><tr><td><label>Fogger</label></td><td><input type = \"text\" name=\"fogger_min\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id=\"fogger_min\"</label></td><td><input type = \"text\" name=\"fogger_max\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id</label></td></tr><tr><td><label>Dehumidifier</label><td><input type = \"text\" name=\"dehum_min\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id=\"dehum_min\"</label></td><td><input type = \"text\" name=\"dehum_max\" class=\"textbook\" placeholder=\"[HUMI: 1-100]\" id=\"dehum_max\"</label></td></tr><tr><td><label>Circular Fan</label></td><td><input type = \"text\" name=\"cf_min\" class=\"textbook\" placeholder=\"[MIN: 1-255]\" id=\"cf_min\"</label></td><td><input type = \"text\" name=\"cf_max\" class=\"textbook\" placeholder=\"[MIN: 1-255]\" id=\"cf_max\"</label></td></tr><tr><td><label>Exhaust Fan</label></td><td><input type = \"text\" name=\"ON\" class=\"textbook\" placeholder=\"[ON: 1-255 MIN]\" id=\"ON\"</label></td><td><input type = \"text\" name=\"every\" class=\"textbook\" placeholder=\"[ON: 1-255 MIN]\" id=\"every\"</label></td></tr></table><button type=\"submit\" value=\"Submit\" class=\"button\">Submit</button></form></div></center></body></html>";
  server.send(200, "text/html", HTML);
  int hold = 0;

  if (server.args() > 0)
  {
    Serial.println("Entered");

    // Network Details
    sssid = server.arg(0);
    pass = server.arg(1);
    interval = server.arg(2);

    // Alerts
    temp_min = server.arg(3);
    temp_max = server.arg(4);
    hum_min = server.arg(5);
    hum_max = server.arg(6);
    co2_min = server.arg(7);
    co2_max = server.arg(8);
    par_min = server.arg(9);
    par_max = server.arg(10);
    vpd_min = server.arg(11);
    vpd_max = server.arg(12);

    // Automation parameters
    fan_on = server.arg(13);
    fan_off = server.arg(14);
    pad_on = server.arg(15);
    pad_off = server.arg(16);
    fogger_on = server.arg(17);
    fogger_off = server.arg(18);
    dehum_on = server.arg(19);
    dehum_off = server.arg(20);
    cirFan_on = server.arg(21);
    cirFan_off = server.arg(22);
    ex_fan_on = server.arg(23);
    ex_fan_off = server.arg(24);

    delay(100);

    // Printing the entered inputs from the web-page
    Serial.print("SSID:");
    Serial.println(sssid);
    Serial.print("Password:");
    Serial.println(pass);
    Serial.print("Interval:");
    Serial.println(interval);

    // Alerts
    Serial.println();
    Serial.println("Temperature");
    Serial.print("Min : ");
    Serial.println(temp_min);
    Serial.print("Max : ");
    Serial.println(temp_max);

    Serial.println();
    Serial.println("Humidity");
    Serial.print("Min : ");
    Serial.println(hum_min);
    Serial.print("Max : ");
    Serial.println(hum_max);

    Serial.println();
    Serial.println("CO2");
    Serial.print("Min : ");
    Serial.println(co2_min);
    Serial.print("Max : ");
    Serial.println(co2_max);

    Serial.println();
    Serial.println("PAR");
    Serial.print("Min : ");
    Serial.println(par_min);
    Serial.print("Max : ");
    Serial.println(par_max);

    Serial.println();
    Serial.println("VPD");
    Serial.print("Min : ");
    Serial.println(vpd_min);
    Serial.print("Max : ");
    Serial.println(vpd_max);

    // Automation Paratemerts

    Serial.println();
    Serial.println("Fan ");
    Serial.print("ON : ");
    Serial.println(fan_on);
    Serial.print("OFF : ");
    Serial.println(fan_off);

    Serial.println();
    Serial.println("Pad ");
    Serial.print("ON : ");
    Serial.println(pad_on);
    Serial.print("OFF : ");
    Serial.println(pad_off);

    Serial.println();
    Serial.println("Fogger");
    Serial.print("ON : ");
    Serial.println(fogger_on);
    Serial.print("OFF : ");
    Serial.println(fogger_off);

    Serial.println();
    Serial.println("Dehumidifier");
    Serial.print("ON : ");
    Serial.println(dehum_on);
    Serial.print("OFF : ");
    Serial.println(dehum_off);

    Serial.println();
    Serial.println("Circular Fan");
    Serial.print("ON : ");
    Serial.println(cirFan_on);
    Serial.print("OFF : ");
    Serial.println(cirFan_off);

    Serial.println();
    Serial.println("Exhaust Fan ");
    Serial.print("ON : ");
    Serial.println(ex_fan_on);
    Serial.print("OFF : ");
    Serial.println(ex_fan_off);

    //         //Alerts
    //   config_data[1]      = temp_min.toInt();
    //   config_data[2]      = temp_max.toInt();
    //   config_data[3]      = hum_min.toInt();
    //   config_data[4]      = hum_max.toInt();
    //   config_data[5]      = co2_min.toInt();
    //   config_data[6]      = co2_max.toInt();
    //   config_data[7]      = par_min.toInt();
    //   config_data[8]      = par_max.toInt();
    //   config_data[9]      = vpd_min.toInt();
    //   config_data[10]     = vpd_max.toInt();
    //
    //        //Automation
    //   config_data[11]     = fanPad_on.toInt();
    //   config_data[12]     = fanPad_off.toInt();
    //   config_data[13]     = fogger_on.toInt();
    //   config_data[14]     = fogger_off.toInt();
    //   config_data[15]     = dehum_on.toInt();
    //   config_data[16]     = dehum_off.toInt();
    //   config_data[17]     = cirFan_on.toInt();
    //   config_data[18]     = cirFan_off.toInt();

    INTERVAL = interval.toInt();
    TEMP_MIN = temp_min.toInt();
    TEMP_MAX = temp_max.toInt();
    HUMI_MIN = hum_min.toInt();
    HUMI_MAX = hum_max.toInt();
    CO2_MIN = co2_min.toInt();
    CO2_MAX = co2_max.toInt();
    PAR_MIN = par_min.toInt();
    PAR_MAX = par_max.toInt();
    VPD_MIN = vpd_min.toInt();
    VPD_MAX = vpd_max.toInt();

    // Automation
    FAN_ON = fan_on.toInt();
    FAN_OFF = fan_off.toInt();
    PAD_ON = pad_on.toInt();
    PAD_OFF = pad_off.toInt();
    FOG_ON = fogger_on.toInt();
    FOG_OFF = fogger_off.toInt();
    DEHUM_ON = dehum_on.toInt();
    DEHUM_OFF = dehum_off.toInt();
    CIR_FAN_ON = cirFan_on.toInt();
    CIR_FAN_OFF = cirFan_off.toInt();
    EX_FAN_ON = ex_fan_on.toInt();
    EX_FAN_OFF = ex_fan_on.toInt();

    // SAVE_CONFIG();

    Serial.println("Saving Data to EEPROM");

    char ssid1[sssid.length() + 1];
    sssid.toCharArray(ssid1, sssid.length() + 1);
    char pass1[pass.length() + 1];
    pass.toCharArray(pass1, pass.length() + 1);

    EEPROM.write(1, sizeof(ssid1));
    EEPROM.commit();
    EEPROM.write(2, sizeof(pass1));
    EEPROM.commit();

    for (int i = 0; i < sizeof(ssid1); i++)
    {
      EEPROM.write(5 + i, ssid1[i]);
      Serial.print(ssid1[i]);
      EEPROM.commit();
    }
    for (int i = 0; i < sizeof(pass1); i++)
    {
      EEPROM.write(5 + sizeof(ssid1) + i, pass1[i]);
      Serial.print(pass1[i]);
      EEPROM.commit();
    }

    delay(100);

    EEPROM.write(111, INTERVAL);
    EEPROM.write(112, TEMP_MIN);
    EEPROM.write(113, TEMP_MAX);
    EEPROM.write(114, HUMI_MIN);
    EEPROM.write(115, HUMI_MAX);
    EEPROM.write(116, CO2_MIN);
    EEPROM.write(117, CO2_MAX);
    EEPROM.write(118, PAR_MIN);
    EEPROM.write(119, PAR_MAX);
    EEPROM.write(120, VPD_MIN);
    EEPROM.write(121, VPD_MAX);
    EEPROM.write(122, FAN_ON);
    EEPROM.write(123, FAN_OFF);
    EEPROM.write(124, PAD_ON);
    EEPROM.write(125, PAD_OFF);
    EEPROM.write(126, FOG_ON);
    EEPROM.write(127, FOG_OFF);
    EEPROM.write(128, DEHUM_ON);
    EEPROM.write(129, DEHUM_OFF);
    EEPROM.write(130, CIR_FAN_ON);
    EEPROM.write(131, CIR_FAN_OFF);
    EEPROM.write(132, EX_FAN_ON);
    EEPROM.write(133, EX_FAN_OFF);
    EEPROM.write(134, 1);
    EEPROM.commit();

    Serial.println(" ");
    Serial.println("[DONE] Saving Data to EEPROM");

    delay(1000);
    // reset_esp();
    ESP.restart();
  }
}

/*|---------------------------------------------------------------------------------------|
                          SAVING ALL CONFIG TO EEPROM OF ESP
  |---------------------------------------------------------------------------------------|*/
/*
void SAVE_CONFIG()
{
  Serial.println("Saving Data to EEPROM");
  char ssid1[sssid.length() + 1];
  sssid.toCharArray(ssid1, sssid.length() + 1);
  char pass1[pass.length() + 1];
  pass.toCharArray(pass1, pass.length() + 1);

  EEPROM.write(1, sizeof(ssid1));
  EEPROM.commit();
  EEPROM.write(2, sizeof(pass1));
  EEPROM.commit();

  for (int i = 0; i < sizeof(ssid1); i++)
  { EEPROM.write(5 + i, ssid1[i]);
    Serial.print(ssid1[i]);
    EEPROM.commit();
  }
  for (int i = 0; i < sizeof(pass1); i++)
  { EEPROM.write(5 + sizeof(ssid1) + i, pass1[i]);
    Serial.print(pass1[i]);
    EEPROM.commit();
  }

  delay(100);

  EEPROM.write(111, INTERVAL);
  EEPROM.write(112, TEMP_MIN);
  EEPROM.write(113, TEMP_MAX);
  EEPROM.write(114, HUMI_MIN);
  EEPROM.write(115, HUMI_MAX);
  EEPROM.write(116, CO2_MIN);
  EEPROM.write(117, CO2_MAX);
  EEPROM.write(118, PAR_MIN);
  EEPROM.write(119, PAR_MAX);
  EEPROM.write(120, VPD_MIN);
  EEPROM.write(121, VPD_MAX);
  EEPROM.write(122, FAN_ON);
  EEPROM.write(123, FAN_OFF);
  EEPROM.write(124, PAD_ON);
  EEPROM.write(125, PAD_OFF);
  EEPROM.write(126, FOG_ON);
  EEPROM.write(127, FOG_OFF);
  EEPROM.write(128, DEHUM_ON);
  EEPROM.write(129, DEHUM_OFF);
  EEPROM.write(130, CIR_FAN_ON);
  EEPROM.write(131, CIR_FAN_OFF);
  EEPROM.write(132, EX_FAN_ON);
  EEPROM.write(133, EX_FAN_OFF);
  EEPROM.write(134, 1);
  EEPROM.commit();

  Serial.println(" ");
  Serial.println("[DONE] Saving Data to EEPROM");

  delay(1000);
  reset_esp();
}
*/
/*|---------------------------------------------------------------------------------------|
                        Share Config to Mega from ESP - SS Comm
  |---------------------------------------------------------------------------------------|*/

void SHARE_CONFIG()
{
  Serial.println("READING CONFIG DATA FROM EEPROM");
  INTERVAL = EEPROM.read(111); // INTERVAL FOR EEPROM
  TEMP_MIN = EEPROM.read(112); // Minimum Temperature
  TEMP_MAX = EEPROM.read(113); // Maximum Temperature
  HUMI_MIN = EEPROM.read(114); // Minimum Humidity
  HUMI_MAX = EEPROM.read(115); // Maximum Humidity
  CO2_MIN = EEPROM.read(116);  // Minimum CO2
  CO2_MAX = EEPROM.read(117);  // Maximum CO2
  PAR_MIN = EEPROM.read(118);  // Minimum PAR
  PAR_MAX = EEPROM.read(119);  // Maximum PAR
  VPD_MIN = EEPROM.read(120);  // Minimum VPD
  VPD_MAX = EEPROM.read(121);  // Maximum VPD

  // Automation Thresholds
  FAN_ON = EEPROM.read(122);      // ON Fan Pad when greater than temperature
  FAN_OFF = EEPROM.read(123);     // OFF Fan Pad when less than temperature
  PAD_ON = EEPROM.read(124);      // ON  Pad when greater than temperature
  PAD_OFF = EEPROM.read(125);     // OFF Pad when less than temperature
  FOG_ON = EEPROM.read(126);      // ON Fogger when less than humidity
  FOG_OFF = EEPROM.read(127);     // OFF Fogger when greater than humidity
  DEHUM_ON = EEPROM.read(128);    // ON Dehumidifier when greater than humidity
  DEHUM_OFF = EEPROM.read(129);   // OFF Dehumidifier when less than humidity
  CIR_FAN_ON = EEPROM.read(130);  // ON Circular Fan when greater than temperature
  CIR_FAN_OFF = EEPROM.read(131); // OFF Circular Fan when less than temperature
  EX_FAN_ON = EEPROM.read(132);   // ON timer exhaust fan when greater than temperature
  EX_FAN_OFF = EEPROM.read(133);  // Off timer exhaust fan when greater than temperature

  delay(1000);

  Serial.println("Values Read from EEPROM Going to Mega");
  Serial.println(" ");
  Serial.print("Interval : ");
  Serial.println(INTERVAL);
  Serial.print("TEMP_MIN : ");
  Serial.println(TEMP_MIN);
  Serial.print("TEMP_MAX : ");
  Serial.println(TEMP_MAX);
  Serial.print("HUMI_MIN : ");
  Serial.println(HUMI_MIN);
  Serial.print("HUMI_MAX : ");
  Serial.println(HUMI_MAX);
  Serial.print("CO2_MIN : ");
  Serial.println(CO2_MIN);
  Serial.print("CO2_MAX : ");
  Serial.println(CO2_MAX);
  Serial.print("PAR_MIN : ");
  Serial.println(PAR_MIN);
  Serial.print("PAR_MAX : ");
  Serial.println(PAR_MAX);
  Serial.print("VPD_MIN : ");
  Serial.println(VPD_MIN);
  Serial.print("VPD_MAX : ");
  Serial.println(VPD_MAX);

  Serial.print("FAN ON : ");
  Serial.println(FAN_ON);
  Serial.print("FAN OFF : ");
  Serial.println(FAN_OFF);
  Serial.print("PAD ON: ");
  Serial.println(PAD_ON);
  Serial.print("PAD OFF: ");
  Serial.println(PAD_OFF);
  Serial.print("FOG ON: ");
  Serial.println(FOG_ON);
  Serial.print("FOG OFF: ");
  Serial.println(FOG_OFF);
  Serial.print("DEHUMIDIFIER ON : ");
  Serial.println(DEHUM_ON);
  Serial.print("DEHUMIDIFIER OFF : ");
  Serial.println(DEHUM_OFF);
  Serial.print("CIRCULAR FAN ON : ");
  Serial.println(CIR_FAN_ON);
  Serial.print("CIRCULAR FAN OFF");
  Serial.println(CIR_FAN_OFF);

  Serial.print("EXHAUST FAN ON: ");
  Serial.println(EX_FAN_ON);
  Serial.print("EXHAUST FAN OFF:");
  Serial.println(EX_FAN_OFF);

  Serial.println("Sending Above Configuration to Mega...");

  // DynamicJsonDocument doc(4000);
  StaticJsonDocument<2000> doc;

  doc["i"] = INTERVAL;     // interval
  doc["ti"] = TEMP_MIN;    // temperature_min
  doc["ta"] = TEMP_MAX;    // temperature_max
  doc["hi"] = HUMI_MIN;    // humidity_min
  doc["ha"] = HUMI_MAX;    // humidity_max
  doc["ci"] = CO2_MIN;     // CO2_min
  doc["ca"] = CO2_MAX;     // Co2_max
  doc["pi"] = PAR_MIN;     // par_min
  doc["pa"] = PAR_MAX;     // par_max
  doc["vi"] = VPD_MIN;     // vpd_min
  doc["va"] = VPD_MAX;     // vpd_max
  doc["f1"] = FAN_ON;      // fan_on
  doc["f0"] = FAN_OFF;     // fan_off
  doc["p1"] = PAD_ON;      // pad_on
  doc["p0"] = PAD_OFF;     // pad_on
  doc["fg1"] = FOG_ON;     // fogger_on
  doc["fg0"] = FOG_OFF;    // fogger_off
  doc["dh1"] = DEHUM_ON;   // dehum_on
  doc["dh0"] = DEHUM_OFF;  // dehum_off
  doc["cf1"] = DEHUM_OFF;  // cirFan_on
  doc["cf0"] = DEHUM_OFF;  // cirFan_off
  doc["ef1"] = EX_FAN_ON;  // Time Based
  doc["ef0"] = EX_FAN_OFF; // Time Based
  // presentTime = millis();
  // while(millis()-presentTime < 2000)
  serializeJson(doc, softSerial);
  doc.clear();

  Serial.println("Configuration Sent to Mega");
}

/*|---------------------------------------------------------------------------------------|
                   Config_Mode - Program Goes in to This On Interupt
  |---------------------------------------------------------------------------------------|*/

void CONFIG_MODE()
{
  intr_check = 0;
  Serial.println();
  Serial.println("Setting up Access Point");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  // Generating IP address through SoftAP for hosting the web-page
  myIP = WiFi.softAPIP();
  Serial.print("AP IP Address is : ");
  Serial.println(myIP);

  server.begin();
  server.on("/", HANDLE_ROOT); // hosts the webpage

  while (1)
    server.handleClient();
}

/*|---------------------------------------------------------------------------------------|
                                  END OF FUNCTIONS.H
  |---------------------------------------------------------------------------------------|*/
