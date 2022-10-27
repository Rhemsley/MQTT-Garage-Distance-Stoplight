#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoMqttClient.h>

// WiFi config
const char* ssid = "RYLANSLAPTOP";          // Set Wifi Name
const char* password = "Onyourleft100";     // Set Password

// Mqtt config
const char broker[] = "192.168.137.1";
int        port     = 1883;
const char topicOff[]  = "Off";
const char topicGreen[]  = "Green";
const char topicYellow[]  = "Yellow";
const char topicRed[]  = "Red";
const char topicBlink[]  = "Blink";
const char topicOpen[]  = "GarageOpen";
const char topicClosed[]  = "GarageClosed";

// Wifi and Mqtt instantiation
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Garage Door status
bool garageOpen = false;

// Send Open Status 
void sendGarageOpen() {
  mqttClient.beginMessage(topicOpen);
  mqttClient.print("Open");
  mqttClient.endMessage();
  Serial.println("Open status sent");
}

// Send Closed Status 
void sendGarageClosed() {
  mqttClient.beginMessage(topicClosed);
  mqttClient.print("Closed");
  mqttClient.endMessage();
  Serial.println("Closed status sent");
}

void setup() {
  // Setup Serial Monitor with 9600
  Serial.begin(9600);

  // Create Wifi connection and delay until connected
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {    // Loop until connected to WiFi
    delay(500);
    Serial.print(".");
  }

  // If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());           // Show your assigned IP address

  // Connect to MQTT
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  // If unable to connect, output error
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  
  // Setup Magnet pin
  pinMode(D5, INPUT_PULLUP);

  // Send starting value
  garageOpen = digitalRead(D5);
  if (garageOpen) {
      Serial.println("Garage Open");
      sendGarageOpen();
      delay(100);
    }
    else {
      Serial.println("Garage Closed");
      sendGarageClosed();
      delay(100);
    }
}

void loop() {
  // Regular calls to send MQTT keep alive
  mqttClient.poll();

  // Get Magnet/Garage door input status
  int pinValue = digitalRead(D5);
  delay(10);

  // If it has changed status
  if (pinValue != garageOpen) {
    // Set new status
    garageOpen = pinValue;
    // If Open, send open, if closed, send closed
    if (garageOpen) {
      Serial.println("Garage Open");
      sendGarageOpen();
      delay(100);
    }
    else {
      Serial.println("Garage Closed");
      sendGarageClosed();
      delay(100);
    }
  }
}
