#include <SoftwareSerial.h>
#include <Arduino.h>
#include <WiFiEspAT.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

#include <private.h>
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

unsigned long previousMillis = 0;
bool backlightState = false;
unsigned long backlightStartTime = 0;
uint8_t counter = 0;

const char *t_temperature = "modul/temperature";
float temperature;

const char *t_humidity = "modul/humidity";
uint8_t humidity;

const char *t_motion = "veranda/motion";
bool motion = 0;

const char *t_pump5 = "rpiext/pump/5";
const char *t_pump6 = "rpiext/pump/6";
const char *t_pump7 = "rpiext/pump/7";
const char *t_pump8 = "rpiext/pump/8";

bool pump5 = 0;
bool pump6 = 0;
bool pump7 = 0;
bool pump8 = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
char line0[17] = "";
char line1[17] = "";

#define relay1 PCINT23
#define relay2 PCINT22
#define relay3 PCINT21
#define relay4 PCINT20

WiFiClient espClient;
PubSubClient client(espClient);

SoftwareSerial soft(13, 12); // RX, TX

void print_pump(uint8_t pump_number)
{
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(F("Pump ON #"));
  lcd.print(pump_number);
  counter = 0;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char in_message[5];
  unsigned int i = 0;
  for (; i < length; i++)
  {
    in_message[i] = char(payload[i]);
  }
  in_message[i] = '\0';
  motion = 0;
  if (strcmp(topic, t_pump5) == 0)
  {
    pump5 = atoi(in_message);
    digitalWrite(relay1, pump5);
    if (pump5)
    {
      print_pump(5);
    }
  }
  if (strcmp(topic, t_pump6) == 0)
  {
    pump6 = atoi(in_message);
    digitalWrite(relay2, pump6);
    if (pump6)
    {
      print_pump(6);
    }
  }
  if (strcmp(topic, t_pump7) == 0)
  {
    pump7 = atoi(in_message);
    digitalWrite(relay3, pump7);
    if (pump7)
    {
      print_pump(7);
    }
  }
  if (strcmp(topic, t_pump8) == 0)
  {
    pump8 = atoi(in_message);
    digitalWrite(relay4, pump8);
    if (pump8)
    {
      print_pump(8);
    }
  }
  if (strcmp(topic, t_motion) == 0)
  {
    motion = bool(in_message);
  }
  if (strcmp(topic, t_temperature) == 0)
  {
    temperature = atof(in_message);
  }
  if (strcmp(topic, t_humidity) == 0)
  {
    humidity = atoi(in_message);
  }
}

void InitWiFi()
{
  soft.begin(9600);
  delay(500);
  WiFi.init(&soft);
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true)
      ;
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    uint8_t status = WiFi.begin(ssid, pass);
    Serial.println();

    if (status != WL_CONNECTED)
    {
      Serial.println(F("Failed to connect to AP"));
    }
    else
    {
      Serial.println(F("You're connected to the network"));
      Serial.println(WiFi.localIP());
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    if (client.connect("rpiext_extrarelays"))
    {
      Serial.println(F("MQTT broker connected"));
      client.publish(t_pump5, "0");
      client.publish(t_pump6, "0");
      client.publish(t_pump7, "0");
      client.publish(t_pump8, "0");
      client.subscribe(t_temperature);
      client.subscribe(t_humidity);
      client.subscribe(t_motion);
      client.subscribe(t_pump5);
      client.subscribe(t_pump6);
      client.subscribe(t_pump7);
      client.subscribe(t_pump8);
    }
    else
    {
      Serial.print(F("failed with state "));
      Serial.print(client.state());
      delay(2000);
      InitWiFi();
    }
  }
}

void InitMqtt()
{
  client.setServer("192.168.1.1", 1883);
  client.setCallback(callback);
  reconnect();
}

void setup()
{
  pinMode(relay1, INPUT_PULLUP);
  pinMode(relay2, INPUT_PULLUP);
  pinMode(relay3, INPUT_PULLUP);
  pinMode(relay4, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, 0);
  digitalWrite(relay2, 0);
  digitalWrite(relay3, 0);
  digitalWrite(relay4, 0);

  lcd.init();
  lcd.backlight();
  backlightState = true;
  backlightStartTime = millis();

  Serial.begin(9600);

  delay(1000);
  InitWiFi();
  delay(1000);
  InitMqtt();
}

void updateDisplay()
{
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 500)
  {
    previousMillis = currentMillis;

    if (!client.connected())
    {
      reconnect();
    }
    client.loop();

    if (motion)
    {
      lcd.backlight();
      backlightState = true;
      backlightStartTime = millis();
      Serial.println(F("BL"));
      client.publish("rpiext_extrarelays/status", "on");
    }
    if (pump5 || pump6 || pump7 || pump8)
    {
      lcd.backlight();
      backlightState = true;
      backlightStartTime = millis();
      counter++;
      lcd.setCursor(0, 1);
      lcd.print(counter);
      lcd.print(F(" seconds on"));
      client.publish("rpiext_extrarelays/status", "pump_on");
    }
    if (backlightState == true && millis() - backlightStartTime >= 8000 && (!pump5 && !pump6 && !pump7 && !pump8))
    {
      backlightState = false;
      lcd.noBacklight();
      Serial.println(F("NO BL"));
      client.publish("rpiext_extrarelays/status", "off");
    }
    if (!pump5 && !pump6 && !pump7 && !pump8)
    {
      char tBuffer[20];
      dtostrf(temperature, 2, 1, tBuffer);
      sprintf(line0, "Ext Temp: %5s%c", tBuffer, char(223));
      sprintf(line1, "Ext Hum: %6d%%", humidity);
      updateDisplay();
      client.publish("rpiext_extrarelays/status", "connected");
    }
  }
}