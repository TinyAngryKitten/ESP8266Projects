#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "passwords.h"

#define DHTTYPE DHT22

//send message every x milliseconds
int pollingRate = 60000;
const char* site = "bathroom";
const char* topic = "sensors";

// DHT Sensor
uint8_t DHTPin = D4;
int Temperature;
int Humidity;
DHT dht(DHTPin, DHTTYPE);

//wifi info
char *ssid = "WeFi";

const char* mqttServer = "10.0.0.95";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);

  //start listening on the dht pin
  pinMode(DHTPin, INPUT);
  dht.begin();

  WiFiConnect();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  MQTTConnect();

}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connected()) {
      sendMeasurements();
      client.loop();
      delay(pollingRate);
    } else {
      delay(100);
      Serial.println("Not connected to MQTT, reconnecting");
      MQTTConnect();
    }
  } else {
    Serial.println("not connected to wifi");
    delay(100);
    WiFiConnect();
  }

}
void sendMeasurements() {
  Temperature = (int) dht.readTemperature(); // Gets the values of the temperature
  Humidity = (int) dht.readHumidity();
  char tempStr[50];
  char humStr[50];
  sprintf(tempStr, "temp, site=%s value=%d",site, Temperature);
  sprintf(humStr, "humidity, site=%s value=%d ",site, Humidity);

  client.publish(topic, tempStr);
  client.publish(topic, humStr);
  Serial.println(humStr);
  Serial.println(topic);
}

bool isConnected() {
  return wifiClient.connected();
}

void WiFiConnect() {
  Serial.println("starting..");
  Serial.print("connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for connection...");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTTConnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");
    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
}
