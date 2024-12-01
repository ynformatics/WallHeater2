#pragma once
#include <AccelStepper.h>
class Flap : public AccelStepper{
public:
    const int openPosition = -400;
    const int midPosition = -230;
    const int closePosition = 0;
    bool _swinging = false;
    float defaultSpeed = 200.0;
    float swingSpeed = 25.0;

    Flap(uint8_t pinIN1, int8_t pinIN3, int8_t pinIN2, int8_t pinIN4) 
        : AccelStepper(AccelStepper::FULL4WIRE, pinIN1, pinIN3, pinIN2, pinIN4){
        setMaxSpeed(defaultSpeed);
	    setAcceleration(25.0);
        setCurrentPosition(closePosition);
    }
    void open(){
        moveTo(openPosition);
    }
    void close() {
        moveTo(closePosition);
    }
    void setSwinging(bool swinging){
        _swinging = swinging;
        setMaxSpeed(_swinging ? swingSpeed : defaultSpeed); 
        open();
    }
    void loop(){
        run();
        if(_swinging && distanceToGo() == 0)         
          moveTo( currentPosition() == openPosition ? midPosition : openPosition);     
    }
};