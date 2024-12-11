#include<Arduino.h>

class Buttons{
    uint8_t pin;
  public:
    enum Button {None, F, Time};
    Button state = None;
    Button lastState = None;
    Buttons(uint8_t pinButtons) : pin(pinButtons){}

    void loop(void(*buttonPressed)(char)){
      int reading = analogRead(pin);
      state = reading > 3000 ? None : reading > 1000 ? Time : F;
      if(state == None && lastState != None){      
          buttonPressed(lastState == Time ? 'T' : 'F');
          lastState = None;
          return;
      }
        
      lastState = state;             
    }
};