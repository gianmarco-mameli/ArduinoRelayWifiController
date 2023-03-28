#include <SoftwareSerial.h>
#include <Arduino.h>
#include <WiFiEsp.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>

#include <private.h>
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

const char *mqtt_broker = "mosquitto.docker.local";
const int mqtt_port = 1883;
const char *client_id = "rpiext_extrarelays";
const char *status_topic = "rpiext_extrarelays/status";

const long interval = 1000;
unsigned long previousMillis = 0;

const char *t_temperature = "modul/temperature";
char *temperature;

const char *t_motion = "veranda/motion";
char *motion;

const char *t_pump5 =  "rpiext/pump/5";
const char *t_pump6 =  "rpiext/pump/6";
const char *t_pump7 =  "rpiext/pump/7";
const char *t_pump8 =  "rpiext/pump/8";

String pump = "0";
char *topic;

LiquidCrystal_I2C lcd(0x27, 16, 2);

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
  if (strcmp(topic,t_temperature)==0) {
    temperature = ((char*)payload);
    Serial.println(temperature);
  }
  // if (strcmp(topic,t_humidity)==0) {
  //   humidity = ((char*)payload);
  //   Serial.println(humidity);
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
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void InitMqtt()
{
  client.setServer(mqtt_broker, mqtt_port);
  // client.setCallback(callback);
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
  lcd.clear();
  lcd.backlight();

  Serial.begin(9600);

  delay(1000);
  InitWiFi();
  delay(1000);
  InitMqtt();

  client.subscribe(t_temperature);
  client.subscribe(t_pump5);
  client.subscribe(t_pump6);
  client.subscribe(t_pump7);
  client.subscribe(t_pump8);

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

    lcd.setCursor(0, 0);
    // lcd.print("Ext Temp " + temperature);
    // lcd.print("123456789123456");
    // lcd.setCursor(5, 0);
    lcd.print(temperature);

    lcd.setCursor(0, 1);

    if (strcmp(topic, t_pump5) == 0)
    {
      digitalWrite(relay1, pump.toInt());
      lcd.print(topic + pump);
      lcd.backlight();
    }
    if (strcmp(topic, t_pump6) == 0)
    {
      digitalWrite(relay2, pump.toInt());
      lcd.print(topic + pump);
      lcd.backlight();
    }
    if (strcmp(topic, t_pump7) == 0)
    {
      digitalWrite(relay3, pump.toInt());
      lcd.print(topic + pump);
      lcd.backlight();
    }
    if (strcmp(topic, t_pump8) == 0)
    {
      digitalWrite(relay4, pump.toInt());
      lcd.print(topic + pump);
      lcd.backlight();
    }
    // if (payload == 0)

    // Serial.println(temperature);
  }
}
