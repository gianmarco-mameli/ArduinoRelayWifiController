#include <stdio.h>
#include <string.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFiEsp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <private.h>
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

const char *mqtt_broker = "mosquitto.docker.local";
const int mqtt_port = 1883;
String client_id = "rpiext_extrarelays";
String status_topic = client_id + "/status";

char in_message[100];

String t_temperature = "modul/temperature";
const char *temperature;

String pump_base_t = "rpiext/pump/";
String t_pump5 = pump_base_t + "5";
String t_pump6 = pump_base_t + "6";
String t_pump7 = pump_base_t + "7";
String t_pump8 = pump_base_t + "8";

int pump5 = 0;
int pump6 = 0;
int pump7 = 0;
int pump8 = 0;

#define LCD_ROWS 2
#define LCD_COLS 16

#define relay1 7
#define relay2 6
#define relay3 5
#define relay4 4

WiFiEspClient espClient;
PubSubClient client(espClient);

SoftwareSerial soft(13, 12); // RX, TX

unsigned long timeout = 200;
unsigned long screensaver = timeout;
int status = WL_IDLE_STATUS;

void callback(char *topic, byte *payload, unsigned int length)
{

  // String msg;
  // Serial.println("Message arrived in topic: ");
  // for (unsigned int i = 0; i < length; i++) {
  //   msg = msg + (char) payload[i];  // convert *byte to string
  // }
  Serial.println(topic);
  Serial.println((char*) payload);
  // payload[length] = '\0';
  // float msg = bytesToFloat(payload[0],payload[1],payload[2],payload[3]);
  unsigned int i = 0;
  for (; i < length; i++)
  {
    in_message[i] = char(payload[i]);
  }
  in_message[i] = '\0';

  // if (strcmp(topic, (char *)t_temperature.c_str()) == 0)
  // {
  //   temperature = ((char *)in_message);
  //   Serial.println(temperature);
  // }
  // if (strcmp(topic, (char *)t_pump5.c_str()) == 0)
  // {
  //   pump5 = atoi(in_message);
  //   Serial.println(pump5);
  // }
  // if (strcmp(topic, (char *)t_pump6.c_str()) == 0)
  // {
  //   pump6 = atoi(in_message);
  //   Serial.println(pump6);
  // }
  // if (strcmp(topic, (char *)t_pump7.c_str()) == 0)
  // {
  //   pump7 = atoi(in_message);
  //   Serial.println(pump7);
  // }
  // if (strcmp(topic, (char *)t_pump8.c_str()) == 0)
  // {
  //   pump8 = atoi(in_message);
  //   Serial.println(pump8);
  // }
}

void InitWiFi()
{
  soft.begin(115200);
  soft.write("AT+UART=9600,8,1,0,0\r\n");
  soft.begin(9600);
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
    }
  }
}

void InitMqtt()
{
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect((char *)client_id.c_str()))
    {
      Serial.println("MQTT connected");
      client.publish((char *)status_topic.c_str(), "connected");
      client.publish((char *)t_pump5.c_str(), "0");
      client.publish((char *)t_pump6.c_str(), "0");
      client.publish((char *)t_pump7.c_str(), "0");
      client.publish((char *)t_pump8.c_str(), "0");
      // delay(500);
      client.subscribe((char *)t_temperature.c_str());
      client.loop();
      client.subscribe((char *)t_pump5.c_str());
      client.loop();
      client.subscribe((char *)t_pump6.c_str());
      client.loop();
      client.subscribe((char *)t_pump7.c_str());
      client.loop();
      client.subscribe((char *)t_pump8.c_str());
      client.loop();
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(relay1, OUTPUT); // connected to Relay 1
  pinMode(relay2, OUTPUT); // connected to Relay 2
  pinMode(relay3, OUTPUT); // connected to Relay 3
  pinMode(relay4, OUTPUT); // connected to Relay 4
  digitalWrite(relay1, 0);
  digitalWrite(relay2, 0);
  digitalWrite(relay3, 0);
  digitalWrite(relay4, 0);
  Serial.begin(9600);
  delay(1000);
  InitWiFi();
  delay(1000);
  InitMqtt();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  // delay(2000);
  client.loop();

  // Serial.println(temperature);
  // digitalWrite(relay1, pump5);
  // digitalWrite(relay2, pump6);
  // digitalWrite(relay3, pump7);
  // digitalWrite(relay4, pump8);
}