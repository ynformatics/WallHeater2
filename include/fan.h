#pragma once
#include <Arduino.h>
#include <flap.h>
class Fan{
public:
    bool fanShutdownInProgress = false;
    long fanShutdownStartTime = 0;
    uint8_t pin;
    Flap& _flap;

    Fan(uint8_t pinFan, Flap& flap) : _flap(flap){
        pin = pinFan;
      
    }
    void on(){
        _flap.open();
        digitalWrite(pin, HIGH);
        fanShutdownInProgress = false;
    }
    void off(){
        if (fanShutdownInProgress) // loop() will shutdown after 30s
            return;
        digitalWrite(pin, LOW);
        _flap.close();
    }
    void shutdown(){
        if(digitalRead(pin) == HIGH && !fanShutdownInProgress){
            fanShutdownStartTime = millis();
            fanShutdownInProgress = true;
        }
    }
    void loop(){
        if(fanShutdownInProgress && millis() - fanShutdownStartTime > 30000){
            digitalWrite(pin, LOW);
            _flap.close();
            fanShutdownInProgress = false;
        }
    }
};