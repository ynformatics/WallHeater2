#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Z:\secure\include\Credentials.h"

#include <ElegantOTA.h>
#include <AccelStepper.h>
#include <fan.h>
#include <flap.h>
#include <et1616.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiUDP udpClient;
WebServer server(80);

const uint8_t pinFan = D0;
const uint8_t pinKeys = A1;
const uint8_t pinIN4 = D2;
const uint8_t pinIN3 = D3;
const uint8_t pinIN2 = D4;
const uint8_t pinIN1 = D5;
const uint8_t pinHeat2 = D6;
const uint8_t pinHeat1 = D7;
const uint8_t pinSTB = D8;
const uint8_t pinCLK = D9;
const uint8_t pinDIN = D10;

long lastStatusMsgTime = 0;

void callback(char* topic, byte* message, unsigned int length);
void setup_wifi() ;

// Stepper pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Flap flap(pinIN1, pinIN3, pinIN2, pinIN4);
Fan fan(pinFan, flap);
ET1616 display(pinCLK, pinDIN, pinSTB);

void setup() {
  pinMode(pinFan, OUTPUT);
  pinMode(pinHeat1, OUTPUT);
  pinMode(pinHeat2, OUTPUT);
  // pinMode(pinSTB, OUTPUT);
  // pinMode(pinCLK, OUTPUT);
  // pinMode(pinDIN, OUTPUT);

  digitalWrite(pinFan, LOW);
  digitalWrite(pinHeat1, LOW);
  digitalWrite(pinHeat2, LOW);
  
  setup_wifi();
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! This is Homecom2 OTA server");
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

  if (String(topic) == "homecom2/heat") {
    if(command == "off"){
      digitalWrite(pinHeat1, LOW);
      digitalWrite(pinHeat2, LOW);
      fan.shutdown();    
    }
    else if (command == "low" || command == "on" ){   
      fan.on();
      digitalWrite(pinHeat1, HIGH);
      digitalWrite(pinHeat2, LOW);
    }
     else if (command == "high"){
      fan.on();
      digitalWrite(pinHeat1, HIGH);
      digitalWrite(pinHeat2, HIGH);
    }
  }
  else if (String(topic) == "homecom2/cool") {
    if(command == "off"){
      fan.shutdown();
    }
    else if (command == "on"){
      fan.on();
    }
  }
  else if (String(topic) == "homecom2/swing") {
    if(command == "on")
      flap.setSwinging(true); 
    else if(command == "off")
       flap.setSwinging(false);
  }
  else if (String(topic) == "homecom2/display") {   
    if(command == "test")
      display.test();
    else
      display.setDisplayData(command.c_str());
  }
   mqttClient.publish("homecom2/debug", command.c_str());
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("")) {
      mqttClient.subscribe("homecom2/heat");
      mqttClient.subscribe("homecom2/cool");
      mqttClient.subscribe("homecom2/swing");
      mqttClient.subscribe("homecom2/display");
    } else {
      delay(5000);
    }
  }
  String message("Connected ");
  message += WiFi.localIP().toString();
  mqttClient.publish("homecom2/debug", message.c_str(), true);
}

void loop() {
  server.handleClient();

  ElegantOTA.loop();
  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();
  fan.loop();
  flap.loop();

  
  long now = millis();
  if (now - lastStatusMsgTime > 5000) {
    lastStatusMsgTime = now;
    
    String status(digitalRead(pinFan) == HIGH ? "fan:on ": "fan:off ");
    status += fan.fanShutdownInProgress ? ",shutdown " : " ";
    status += (digitalRead(pinHeat1) == HIGH ? "h1:on ": "h1:off ");
    status += (digitalRead(pinHeat2) == HIGH ? "h2:on ": "h2:off ");
    status += "flap:"; status += flap.currentPosition();
  
    mqttClient.publish("homecom2/status", status.c_str(), true);
  }
}
