#include<Arduino.h>

class Keys{
    uint8_t pin;
    bool keyPressed = false;
    int  lastReading = 0;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 10;

    public:
    enum Button {None, F, Time};
    Button State = None;

    Keys(uint8_t pinKeys) : pin(pinKeys){}

    void loop()
    {
    int reading = analogRead(pin);
  
  
  // If the switch changed, due to noise or pressing:
  if (reading != lastReading) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // if ((millis() - lastDebounceTime) > debounceDelay) {
  //   // whatever the reading is at, it's been there for longer
  //   // than the debounce delay, so take it as the actual current state:

  //   State = reading < 250 ? None : reading > 750 ? Time : F;
  //   // if the button state has changed:
  //   if (reading != buttonState) {
  //     buttonState = reading;

  //     // only toggle the LED if the new button state is HIGH
  //     if (buttonState > 200) {
  //       ledState = !ledState;
        
  //     }
  //   }
  // }

  // // set the LED:
  // digitalWrite(ledPin, ledState);

  // // save the reading.  Next time through the loop,
  // // it'll be the lastButtonState:
  // lastButtonState = reading;

  //   }
};