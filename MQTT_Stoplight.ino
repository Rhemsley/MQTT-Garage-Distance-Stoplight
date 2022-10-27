#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoMqttClient.h>

// Needed variables for LED Pin association
unsigned char greenLed = D3;        // Create greenLed Label for pin 3
unsigned char yellowLed = D2;       // Create yellowLed Label for pin 2
unsigned char redLed = D1;          // Create redLed Label for pin 1

// Needed variables for looping and blinking
unsigned long previousTime = 0;     // Create previousTime int for looping
unsigned long startTime = 0;        // Create startTime int for looping
bool runStoplight = false;          // Create runStoplight bool for looping
bool runBlink = false;              // Create runBlink bool for blinking
bool redStatus = false;             // Create Red bool for cycling red
bool yellowStatus = false;          // Create Yellow bool for cycling yellow
bool greenStatus = false;           // Create Green bool for cycling green

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
// Set interval for sending messages (milliseconds)
const long interval = 8000;
unsigned long previousMillis = 0;
int count = 0;

// Wifi and Mqtt instantiation
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Subscribed Message handling
void onMqttMessage(int messageSize) {
  // Received a message, print out the topic and message size
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // If Topic matches a stoplight functioncall, call that function
  if (mqttClient.messageTopic() == topicOff) {
    handleOff();
  }
  else if (mqttClient.messageTopic() == topicGreen) {
    handleGreen();
  }
  else if (mqttClient.messageTopic() == topicYellow) {
    handleYellow();
  }
  else if (mqttClient.messageTopic() == topicRed) {
    handleRed();
  }
  else if (mqttClient.messageTopic() == topicBlink) {
    handleBlink();
  }

  // Use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
  Serial.println();
}

// Handle Off call and reset needed variables and LEDs
void handleOff() {
  digitalWrite(greenLed, LOW);              // Turn Green LED off
  digitalWrite(yellowLed, LOW);             // Turn Yellow LED off
  digitalWrite(redLed, LOW);                // Turn Red LED off
  greenStatus = false;                      // Indicate green is off
  yellowStatus = false;                     // Indicate yellow is off
  redStatus = false;                        // Indicate red is off
  runStoplight = false;                     // Stop stoplight looping
  runBlink = false;                         // Stop blink looping
  previousTime = 0;                         // Reset previousTime
  startTime = 0;                            // Reset startTime
  Serial.println("Turning Off LEDs");
}

// Handle Red call and cycle red LED depending on status
void handleRed() {
  // If off turn on and if on turn off
  if (redStatus == false) {
    digitalWrite(redLed, HIGH);             // Turn Red LED on
    redStatus = true;                       // Flip status
  }
  else {
    digitalWrite(redLed, LOW);              // Turn Red LED off
    redStatus = false;                      // Flip status
  }
  Serial.println("Cycling Red");
}

// Handle Yellow call and cycle yellow LED depending on status
void handleYellow() {
  // If off turn on and if on turn off
  if (yellowStatus == false) {
    digitalWrite(yellowLed, HIGH);          // Turn Yellow LED on
    yellowStatus = true;                    // Flip status
  }
  else {
    digitalWrite(yellowLed, LOW);           // Turn Yellow LED off
    yellowStatus = false;                   // Flip status
  }
  Serial.println("Cycling Yellow");
}

// Handle Green call and cycle green LED depending on status
void handleGreen() {
  // If off turn on and if on turn off
  if (greenStatus == false) {
    digitalWrite(greenLed, HIGH);           // Turn Green LED on
    greenStatus = true;                     // Flip status
  }
  else {
    digitalWrite(greenLed, LOW);            // Turn Green LED off
    greenStatus = false;                    // Flip status
  }
  Serial.println("Cycling Green");
}

// Handle Loop call and setup needed variables
void handleLoop() {
  digitalWrite(greenLed, LOW);              // Turn Green LED off
  digitalWrite(yellowLed, LOW);             // Turn Yellow LED off
  digitalWrite(redLed, LOW);                // Turn Red LED off

  greenStatus = false;                      // Indicate green is off
  yellowStatus = false;                     // Indicate yellow is off
  redStatus = false;                        // Indicate red is off
  
  runStoplight = true;                      // Start stoplight looping
  previousTime = millis();                  // Set previousTime to current runtime
  startTime = millis();                     // Then set startTime to current runtime
  Serial.println("Starting Stoplight Loop");
}

void handleBlink() {
  digitalWrite(greenLed, LOW);              // Turn Green LED off
  digitalWrite(yellowLed, LOW);             // Turn Yellow LED off
  digitalWrite(redLed, LOW);                // Turn Red LED off

  greenStatus = false;                      // Indicate green is off
  yellowStatus = false;                     // Indicate yellow is off
  redStatus = false;                        // Indicate red is off
  runBlink = true;                          // Start blink looping
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
  Serial.println(topicOff);
  Serial.println(topicGreen);
  Serial.println(topicYellow);
  Serial.println(topicRed);
  Serial.println(topicBlink);
  //Serial.println(topicOpen);
  //Serial.println(topicClosed);
  Serial.println();
  mqttClient.subscribe(topicOff);
  mqttClient.subscribe(topicGreen);
  mqttClient.subscribe(topicYellow);
  mqttClient.subscribe(topicRed);
  mqttClient.subscribe(topicBlink);
  //mqttClient.subscribe(topicOpen);
  //mqttClient.subscribe(topicClosed);
  
  // Setup all 3 LED pins and make sure they are off
  pinMode(greenLed, OUTPUT);                // Set Green LED to Output pin
  pinMode(yellowLed, OUTPUT);               // Set Yellow LED to Output pin
  pinMode(redLed, OUTPUT);                  // Set Red LED to Output pin
  digitalWrite(greenLed, LOW);              // Turn Green LED off
  digitalWrite(yellowLed, LOW);             // Turn Yellow LED off
  digitalWrite(redLed, LOW);                // Turn Red LED off
}

void loop() {
  // Regular calls to send MQTT keep alive
  mqttClient.poll();
  
  // Stoplight Controlling only runs when set to true
  if (runStoplight == true) {
    // compare last run to starting time, turn to seconds, then 
    // check if within first second of three second loop
    if ((((previousTime - startTime) / 1000)%3) == 0) {
        digitalWrite(greenLed, HIGH);         // Turn Green LED on
        digitalWrite(yellowLed, LOW);         // Turn Yellow LED off
        digitalWrite(redLed, LOW);            // Turn Red LED off
        Serial.println("Loop Green ON");
        previousTime = millis();              // Set previous time to current time
    }
    // compare last run to starting time, turn to seconds, then 
    // check if within second second of three second loop
    else if ((((previousTime - startTime) / 1000)%3) == 1) {
        digitalWrite(greenLed, LOW);          // Turn Green LED off
        digitalWrite(yellowLed, HIGH);        // Turn Yellow LED on
        digitalWrite(redLed, LOW);            // Turn Red LED off
        Serial.println("Loop Yellow ON");
        previousTime = millis();              // Set previous time to current time
    }
    // compare last run to starting time, turn to seconds, then 
    // check if within third second of three second loop
    else if ((((previousTime - startTime) / 1000)%3) == 2) {
        digitalWrite(greenLed, LOW);          // Turn Green LED off
        digitalWrite(yellowLed, LOW);         // Turn Yellow LED off
        digitalWrite(redLed, HIGH);           // Turn Red LED on
        Serial.println("Loop Red ON");
        previousTime = millis();              // Set previous time to current time
    } 
  }

  // Blink operation only runs when set to true
  if (runBlink == true) {
    // If off turn on and if on turn off
    if (redStatus == false) {
      digitalWrite(redLed, HIGH);             // Turn Red LED on
      redStatus = true;                       // Flip status
    }
    else {
      digitalWrite(redLed, LOW);              // Turn Red LED off
      redStatus = false;                      // Flip status
    }
    Serial.println("Cycling Red");
    delay(100);                               // Delay for 100ms
  }
}
