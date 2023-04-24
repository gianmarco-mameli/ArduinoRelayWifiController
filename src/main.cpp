#include <SoftwareSerial.h>
#include <Arduino.h>
#include <WiFiEspAT.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>

#include <private.h>
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

const char *mqtt_broker = "192.168.1.1";
const int mqtt_port = 1883;
const char *client_id = "rpiext_extrarelays";
const char *status_topic = "rpiext_extrarelays/status";

const long interval = 2000;
unsigned long previousMillis = 0;
bool backlightState = false;
unsigned long backlightStartTime = 0;
int counter = 0;

const char *t_temperature = "modul/temperature";
float temperature;

const char *t_humidity = "modul/humidity";
int humidity;

const char *t_motion = "veranda/motion";
bool motion = 0;

const char *t_pump5 = "rpiext/pump/5";
const char *t_pump6 = "rpiext/pump/6";
const char *t_pump7 = "rpiext/pump/7";
const char *t_pump8 = "rpiext/pump/8";

int pump5 = 0;
int pump6 = 0;
int pump7 = 0;
int pump8 = 0;

char *topic;

LiquidCrystal_I2C lcd(0x27, 16, 2);
char line0[17];
char line1[17];

#define relay1 7
#define relay2 6
#define relay3 5
#define relay4 4

WiFiClient espClient;
PubSubClient client(espClient);

#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <SoftwareSerial.h>
SoftwareSerial soft(13, 12); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

unsigned long timeout = 200;
unsigned long blTimeout = 5000;
int status = WL_IDLE_STATUS;

void callback(char *topic, byte *payload, unsigned int length)
{
  char in_message[5];
  unsigned int i = 0;
  for (; i < length; i++)
  {
    in_message[i] = char(payload[i]);
  }
  in_message[i] = '\0';

  lcd.setCursor(0, 0);
  motion = 0;
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
  if (strcmp(topic, t_pump5) == 0)
  {
    pump5 = atoi(in_message);
    digitalWrite(relay1, pump5);
    if (pump5)
    {
      lcd.clear();
      lcd.print(F("Pump 5 ON"));
      counter = 0;
    }
  }
  if (strcmp(topic, t_pump6) == 0)
  {
    pump6 = atoi(in_message);
    digitalWrite(relay1, pump6);
    if (pump6)
    {
      lcd.clear();
      lcd.print(F("Pump 6 ON"));
      counter = 0;
    }
  }
  if (strcmp(topic, t_pump5) == 0)
  {
    pump7 = atoi(in_message);
    digitalWrite(relay1, pump7);
    if (pump7)
    {
      lcd.clear();
      lcd.print(F("Pump 7 ON"));
      counter = 0;
    }
  }
  if (strcmp(topic, t_pump8) == 0)
  {
    pump8 = atoi(in_message);
    digitalWrite(relay1, pump8);
    if (pump8)
    {
      lcd.clear();
      lcd.print(F("Pump 8 ON"));
      counter = 0;
    }
  }
}

void InitWiFi()
{
  // soft.begin(115200);
  // soft.write("AT+UART=9600,8,1,0,0\r\n");
  soft.begin(AT_BAUD_RATE);
  delay(500);
  WiFi.init(&soft);
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println();
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    int status = WiFi.begin(ssid, pass);
    Serial.println();

    if (status != WL_CONNECTED)
    {
      Serial.println("Failed to connect to AP");
    }
    else
    {
      Serial.println("You're connected to the network");
      Serial.println(WiFi.localIP());
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    if (client.connect(client_id))
    {
      Serial.println("MQTT broker connected");
      client.publish(status_topic, "connected");
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
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(5000);
      InitWiFi();
    }
  }
}

void InitMqtt()
{
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void setup()
{
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, 0);
  digitalWrite(relay2, 0);
  digitalWrite(relay3, 0);
  digitalWrite(relay4, 0);

  lcd.init();
  // lcd.clear();
  lcd.backlight();
  backlightState = true;
  backlightStartTime = millis();

  Serial.begin(9600);

  delay(1000);
  InitWiFi();
  delay(1000);
  InitMqtt();
}

void updateDisplay() {
  lcd.setCursor(0,0);
  lcd.print(line0);
  lcd.setCursor(0,1);
  lcd.print(line1);
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
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
      // Serial.println("BL");
      // Serial.println(motion);
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
    }
    if (backlightState == true && millis() - backlightStartTime >= blTimeout && (!pump5 && !pump6 && !pump7 && !pump8))
    {
      // lcd.clear();
      backlightState = false;
      lcd.noBacklight();
      Serial.println("NO BL");
      lcd.clear();
    }
    if (!pump5 && !pump6 && !pump7 && !pump8)
    {
      // lcd.setCursor(0, 0);
      char tBuffer[15];
      dtostrf(temperature, 2, 1, tBuffer);
      sprintf(line0, "Ext Temp: %5s%c", tBuffer, char(223));
      sprintf(line1, "Ext Hum: %6d%%", humidity);
      updateDisplay();
    }
  }
}
