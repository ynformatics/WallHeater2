#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Z:\secure\include\Credentials.h"

#include <ElegantOTA.h>
#include <AccelStepper.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiUDP udpClient;
WebServer server(80);

const uint8_t pinFan = D0;
const uint8_t pinHeat1 = D1;
const uint8_t pinHeat2 = D2;
const uint8_t pinBuzz = D3;
const uint8_t pinIN1 = D7;
const uint8_t pinIN2 = D8;
const uint8_t pinIN3 = D9;
const uint8_t pinIN4 = D10;
// Stepper pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper stepper(AccelStepper::FULL4WIRE, pinIN1, pinIN3, pinIN2, pinIN4);

long lastStatusMsgTime = 0;
bool fanShutdownInProgress = false;
long fanShutdownStartTime = 0;

void callback(char* topic, byte* message, unsigned int length);
void setup_wifi() ;

void setup() {
  pinMode(pinFan, OUTPUT);
  pinMode(pinHeat1, OUTPUT);
  pinMode(pinHeat2, OUTPUT);
  pinMode(pinBuzz, OUTPUT);

  digitalWrite(pinFan, LOW);
  digitalWrite(pinHeat1, LOW);
  digitalWrite(pinHeat2, LOW);
  
  stepper.setMaxSpeed(1000.0);
	stepper.setAcceleration(50.0);
	stepper.setSpeed(200);

  setup_wifi();
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! This is Homecom OTA server");
  });
 
  ElegantOTA.begin(&server);    
  server.begin();
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
}

void setup_wifi() {
  delay(10);

  WiFi.begin(WIFI_SSID , WIFI_PASSWORD );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String command(message, length);
  command.toLowerCase();

  if (String(topic) == "homecom/heat") {
    if(command == "off"){
      digitalWrite(pinHeat1, LOW);
      digitalWrite(pinHeat2, LOW);
      fanShutdownStartTime = millis();
      fanShutdownInProgress = true;
    }
    else if (command == "low" || command == "on" ){
      digitalWrite(pinFan, HIGH);
      fanShutdownInProgress = false;
      digitalWrite(pinHeat1, HIGH);
      digitalWrite(pinHeat2, LOW);
    }
     else if (command == "high"){
      digitalWrite(pinFan, HIGH);
      fanShutdownInProgress = false;
      digitalWrite(pinHeat1, HIGH);
      digitalWrite(pinHeat2, HIGH);
    }
  }
  else if (String(topic) == "homecom/cool") {
    if(command == "off"){
      if (fanShutdownInProgress) // will shutdown after 30s
        return;
      digitalWrite(pinFan, LOW);
    }
    else if (command == "on"){
      digitalWrite(pinFan, HIGH);
      fanShutdownInProgress = false;
    }
  }
  else if (String(topic) == "homecom/flap") {
    stepper.enableOutputs();
    if(command == "up")
      stepper.move(25);
    else
      stepper.move(-25);
  }
  else if (String(topic) == "homecom/beep") {
    if(command == "beep")
     tone(pinBuzz,2000,250);
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Client")) {
      mqttClient.subscribe("homecom/heat");
      mqttClient.subscribe("homecom/cool");
      mqttClient.subscribe("homecom/flap");
      mqttClient.subscribe("homecom/beep");
    } else {
      delay(5000);
    }
  }
  String message("Connected ");
  message += WiFi.localIP().toString();
  mqttClient.publish("homecom/debug", message.c_str());
}

void loop() {
  server.handleClient();

  ElegantOTA.loop();
  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();

  stepper.run();
  if(stepper.distanceToGo() == 0)
    stepper.disableOutputs();

  if(fanShutdownInProgress && millis() - fanShutdownStartTime > 30000){
    digitalWrite(pinFan, LOW);
    fanShutdownInProgress = false;
  }
  long now = millis();
  if (now - lastStatusMsgTime > 5000) {
    lastStatusMsgTime = now;
    
    String status(digitalRead(pinFan) == HIGH ? "fan:on ": "fan:off ");
    status += (digitalRead(pinHeat1) == HIGH ? "h1:on ": "h1:off ");
    status += (digitalRead(pinHeat2) == HIGH ? "h2:on ": "h2:off ");
  
    mqttClient.publish("homecom/status", status.c_str(), true);
  }
}

