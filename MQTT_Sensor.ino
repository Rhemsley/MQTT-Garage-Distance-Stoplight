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

// Set Sensor Pins
unsigned char echoPin = D1;                 // Set Echo Pin
unsigned char trigPin = D2;                 // Set Trig Pin

// Needed variables for sensor and light status
long duration;                              // sensor duration measurement
int distance1 = 0;                          // Current distance measurement
int distance2 = 0;                          // Previous distance measurement 2
int distance3 = 0;                          // Previous distance measurement 3
int distance4 = 0;                          // Previous distance measurement 4
int distance5 = 0;                          // Previous distance measurement 5
int avgDistance = 0;                        // Sensor duration measurement
bool greenOn = false;                       // Green status
bool yellowOn = false;                      // Yellow status
bool redOn = false;                         // Red status
bool blinkOn = false;                       // Blink status
bool garageOpen = false;                    // Garage status

// Subscribed Message handling
void onMqttMessage(int messageSize) {
  // Received a message, print out the topic and message size
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // Save open or close garage door status
  if (mqttClient.messageTopic() == topicOpen) {
    Serial.println("success for Open");
    garageOpen = true;
  }
  else if (mqttClient.messageTopic() == topicClosed) {
    Serial.println("success for Closed");
    garageOpen = false;
  }

  // Use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
  Serial.println();
}

// Send Off request
void lightsOff() {
  mqttClient.beginMessage(topicOff);
  mqttClient.print("Off");
  mqttClient.endMessage();
  Serial.println("Off request sent");
}

// Send Green light request
void cycleGreen() {
  mqttClient.beginMessage(topicGreen);
  mqttClient.print("Green");
  mqttClient.endMessage();
  Serial.println("Green request sent");
}

// Send Yellow light request
void cycleYellow() {
  mqttClient.beginMessage(topicYellow);
  mqttClient.print("Yellow");
  mqttClient.endMessage();
  Serial.println("Yellow request sent");
}

// Send Red light request
void cycleRed() {
  mqttClient.beginMessage(topicRed);
  mqttClient.print("Red");
  mqttClient.endMessage();
  Serial.println("Red request sent");
}

// Send Blink light request
void startBlink() {
  mqttClient.beginMessage(topicBlink);
  mqttClient.print("Blink");
  mqttClient.endMessage();
  Serial.println("Blink request sent");
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

  // Set Received message action to call selector function
  mqttClient.onMessage(onMqttMessage);

  // Subscribe to desired topics, others commented out for viewing
  Serial.print("Subscribing to topics: ");
  //Serial.println(topicOff);
  //Serial.println(topicGreen);
  //Serial.println(topicYellow);
  //Serial.println(topicRed);
  //Serial.println(topicBlink);
  Serial.println(topicOpen);
  Serial.println(topicClosed);
  Serial.println();
  //mqttClient.subscribe(topicOff);
  //mqttClient.subscribe(topicGreen);
  //mqttClient.subscribe(topicYellow);
  //mqttClient.subscribe(topicRed);
  //mqttClient.subscribe(topicBlink);
  mqttClient.subscribe(topicOpen);
  mqttClient.subscribe(topicClosed);
  
  // Setup Sensor Pins
  pinMode(trigPin, OUTPUT);                 // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);                  // Sets the echoPin as an Input

  // Start with lights Off
  lightsOff();                              // Makes sure all the lights are off to start
}

void loop() {
  // Regular calls to send MQTT keep alive
  mqttClient.poll();

  // Only use sensor if Garage is open
  if (garageOpen) {
    // Sensor distance checking
    digitalWrite(trigPin, LOW);               // Clears the trigPin
    delayMicroseconds(2);                     // Small delay
    digitalWrite(trigPin, HIGH);              // Sets the trigPin on HIGH state
    delayMicroseconds(10);                    // For 10 microseconds
    digitalWrite(trigPin, LOW);               // trigPin back to LOW state
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
  
    // Calculating the measured distance in inches. Remove "* 0.393701" to get cm.
    distance1 = duration * 0.034 / 2 * 0.393701;

    // If a bad distance read, just print a ` and don't add the new distance
    if (distance1 >= 120) {
      Serial.print("`");
    }
    else {
      // add delay to be measuring roughly 9-10 times a second
      delay(100);

      // Average the previous 5 distances together to get a rolling average
      avgDistance = (distance1 + distance2 + distance3 + distance4 + distance5) / 5;

      // Shift all of the measured distances once
      distance5 = distance4;
      distance4 = distance3;
      distance3 = distance2;
      distance2 = distance1;

      // Prints the avgDistance on the Serial Monitor
      Serial.print("Average Distance (inch): ");
      Serial.println(avgDistance);
    }

    // Now to check the distance and set the appropriate LEDs with Get requests

    // Distance between 120 and 20 inches, set LED to green if not already green
    if ((avgDistance <= 120) && (avgDistance > 20) && greenOn == false) {
      // Lights Off then Green on
      lightsOff();
      cycleGreen();

      // All status false but green
      greenOn = true;
      yellowOn = false;
      redOn = false;
      blinkOn = false;
    }
    // Distance between 20 and 12 inches, set LED to yellow if not already yellow
    else if ((avgDistance <= 20) && (avgDistance > 12) && yellowOn == false) {
      // Lights Off then Yellow on
      lightsOff();
      cycleYellow();

      // All status false but yellow
      greenOn = false;
      yellowOn = true;
      redOn = false;
      blinkOn = false;
    }
    // Distance between 12 and 5 inches, set LED to red if not already red
    else if ((avgDistance <= 12) && (avgDistance > 5) && redOn == false) {
      // Lights Off then Red on
      lightsOff();
      cycleRed();

      // All status false but red
      greenOn = false;
      yellowOn = false;
      redOn = true;
      blinkOn = false;
    }
    // Distance between 5 and 0 inches, set LED to blinking red if not already blinking
    else if ((avgDistance <= 5) && (avgDistance > 0) && blinkOn == false) {
      // Lights Off then Blink Red on
      lightsOff();
      startBlink();

      // All status false but blink
      greenOn = false;
      yellowOn = false;
      redOn = false;
      blinkOn = true;
    }
  }
  // If garage is closed, turn off lights if any are on
  else if (greenOn || yellowOn || redOn || blinkOn) {
    // Lights Off
    lightsOff();
    
    // All status false, lights are off
    greenOn = false;
    yellowOn = false;
    redOn = false;
    blinkOn = false;
  }
}
