#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "passwords.h"

#define DHTTYPE DHT22

//send message every x milliseconds
int pollingRate = 2000;
const char* tempTopic = "bathroom/temperature";
const char* humidityTopic = "bathroom/humidity";

// DHT Sensor
uint8_t DHTPin = D4;
float Temperature;
float Humidity;
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
  // put your main code here, to run repeatedly:
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
  //baseTopic + value
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity();
  char tempStr[10];
  char humStr[10];
  sprintf(tempStr, "%f", Temperature);
  sprintf(humStr, "%f", Humidity);

  client.publish(tempTopic, tempStr);
  client.publish(humidityTopic, humStr);
}

//first value sendt should be the number of characters following
char* readMsg() {
  if (!isConnected()) {
    Serial.println("NO MESSAGE RECEIVED");
    return NULL;
  }

  if (!wifiClient.available()) return NULL;
  int nrOfChars = (int) wifiClient.read();

  int charCnt = 0;
  char* msg = new char[nrOfChars];
  char c;

  while (wifiClient.available() && charCnt < nrOfChars) {
    c = wifiClient.read();
    if (c == '\0') break;
    msg[charCnt] = c;
    charCnt++;
  }
  return msg;
}

bool isConnectedToWifi(char *hostname, int portnr) {
  if (WiFi.status() == WL_CONNECTED) return wifiClient.connect(hostname, portnr);
  else return true;
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
