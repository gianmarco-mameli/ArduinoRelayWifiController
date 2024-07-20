#include <SoftwareSerial.h>
#include <Arduino.h>
// #include <WiFiEspAT.h>
#include <WiFiS3.h>
#include <ArduinoMqttClient.h>
#include <LiquidCrystal_I2C.h>

#include <private.h>
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const long interval = 1000;
unsigned long previousMillis = 0;
uint8_t counter = 0;

const char broker[] = "192.168.31.100";
unsigned int port = 1883;
const char t_pump5[] = "pump/5";
const char t_pump6[] = "pump/6";
const char t_pump7[] = "pump/7";
const char t_pump8[] = "pump/8";
const char willTopic[] = "rpiext_extrarelays/status";
int pump5 = 0;
char pump6 = 0;
char pump7 = 0;
char pump8 = 0;
#define relay1 PIN_D7
#define relay2 PIN_D6
#define relay3 PIN_D5
#define relay4 PIN_D4

LiquidCrystal_I2C lcd(0x27, 16, 2);
char line0[17] = "";
char line1[17] = "";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

SoftwareSerial soft(13, 12); // RX, TX

void print_pump(uint8_t pump_number)
{
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(F("Pump ON #"));
  lcd.print(pump_number);
  counter = 0;
}

void onMqttMessage(int messageSize)
{
  const char *topic = mqttClient.messageTopic().c_str();
  char message;
  Serial.print(topic);
  Serial.print(" ");
  while (mqttClient.available()) {
    message = (char)mqttClient.read();
  }
  if (strcmp(topic, t_pump5) == 0)
  {
    pump5 = (int)message - 48;
    Serial.println(pump5);
    digitalWrite(relay1, pump5);
    if (pump5)
    {
      print_pump(5);
    }
  }
  if (strcmp(topic, t_pump6) == 0)
  {
    pump6 = (int)message - 48;
    Serial.println(pump6);
    digitalWrite(relay2, pump6);
    if (pump6)
    {
      print_pump(6);
    }
  }
  if (strcmp(topic, t_pump7) == 0)
  {
    pump7 = (int)message - 48;
    Serial.println(pump7);
    digitalWrite(relay3, pump7);
    if (pump7)
    {
      print_pump(7);
    }
  }
  if (strcmp(topic, t_pump8) == 0)
  {
    pump8 = (int)message - 48;
    Serial.println(pump8);
    digitalWrite(relay4, pump8);
    if (pump8)
    {
      print_pump(8);
    }
  }
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

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  lcd.init();
  lcd.backlight();

  String willPayload = "disconnected";

  mqttClient.setId("rpiext_extrarelays");
  mqttClient.beginWill(willTopic, willPayload.length(), false, 1);
  mqttClient.print(willPayload);
  mqttClient.endWill();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  mqttClient.onMessage(onMqttMessage);
  String payload = "0";
  unsigned int subscribeQos = 1;

  mqttClient.beginMessage(t_pump5, payload.length(), false, 1, false);
  mqttClient.print(payload);
  mqttClient.endMessage();
  mqttClient.beginMessage(t_pump6, payload.length(), false, 1, false);
  mqttClient.print(payload);
  mqttClient.endMessage();
  mqttClient.beginMessage(t_pump7, payload.length(), false, 1, false);
  mqttClient.print(payload);
  mqttClient.endMessage();
  mqttClient.beginMessage(t_pump8, payload.length(), false, 1, false);
  mqttClient.print(payload);
  mqttClient.endMessage();

  digitalWrite(relay1, 0);
  digitalWrite(relay2, 0);
  digitalWrite(relay3, 0);
  digitalWrite(relay4, 0);

  mqttClient.subscribe(t_pump5, subscribeQos);
  mqttClient.subscribe(t_pump6, subscribeQos);
  mqttClient.subscribe(t_pump7, subscribeQos);
  mqttClient.subscribe(t_pump8, subscribeQos);
}

// void updateDisplay()
// {
//   lcd.setCursor(0, 0);
//   lcd.print(line0);
//   lcd.setCursor(0, 1);
//   lcd.print(line1);
// }

void loop()
{
  mqttClient.poll();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (pump5 || pump6 || pump7 || pump8)
    {
      lcd.backlight();
      counter++;
      lcd.setCursor(0, 1);
      lcd.print(counter);
      lcd.print(F(" seconds on"));
      String payload = "pump_on";
      mqttClient.beginMessage(willTopic, payload.length(), false, 1, false);
      mqttClient.print(payload);
      mqttClient.endMessage();
      Serial.println("pump_on");
    }
    if (!pump5 && !pump6 && !pump7 && !pump8)
    {
      lcd.init();
      lcd.noBacklight();
      String payload = "connected";
      mqttClient.beginMessage(willTopic, payload.length(), false, 1, false);
      mqttClient.print(payload);
      mqttClient.endMessage();
      Serial.println("connected");
    }
  }
}