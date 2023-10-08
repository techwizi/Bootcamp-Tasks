#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Constants
#define WIFI_SSID "vivo T1"
#define WIFI_PASS "devil12345"

#define MQTT_SERVER "192.168.205.119 2409:4063:2214:3417:8399:c1b1:b55b:c5c9"
#define MQTT_PORT 1883

#define MQTT_PUB "ultrasonic/data"
#define MQTT_SUB "ultrasonic/ctrl"

// NodeMCU pins
const int ledPin = 3;  // Built-in LED
const int trigPin = 4;           // D2 - Ultrasonic sensor trigger pin
const int echoPin = 5;           // D1 - Ultrasonic sensor echo pin


// Global variables
unsigned long previousMillis = 0;
const long interval = 1000 * 5;  // 5 seconds

// Setup WiFi and MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void reconnect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect("ESP8266Client")) {
            Serial.println("connected");
            mqttClient.subscribe(MQTT_SUB);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");

            delay(5000);
        }
    }
}

float distance_threshold = 20;

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    String message = "";
    for (int i = 0; i < (int)length; i++) {
        message += (char)payload[i];
    }

    Serial.println(message);

    if (String(topic) == MQTT_SUB) {
        if (message == "on") {
            digitalWrite(ledPin, HIGH);
        } else if (message == "off") {
            digitalWrite(ledPin, LOW);
        }
    }
     else if (String(topic) == "ultrasonic/threshold") {
        // Received a new distance threshold from Raspberry Pi
        distance_threshold = message.toFloat();
        Serial.println("Updated distance threshold: " + String(distance_threshold));
    }
}

void setup() {
    Serial.begin(9600);

    // Set pin modes
    pinMode(ledPin, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");

    // Connect to MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Read ultrasonic sensor data
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH);
        float distance = duration * 0.0343 / 2;  // Calculate distance in cm

        Serial.println("Distance: " + String(distance) + " cm");

        String message = String(distance);

        mqttClient.publish(MQTT_PUB, message.c_str());
         if (distance < distance_threshold) {
            mqttClient.publish(MQTT_SUB, "on");  // Publish "on" to control the LED
        } else {
            mqttClient.publish(MQTT_SUB, "off");  // Publish "off" to control the LED
        }
    }

    mqttClient.loop();
}
