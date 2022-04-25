/* SingleSweepServo class
 *   - used for single sweep of servo by specifying initial/final 
 *     positions and speed via the update interval
 */

#include <Servo.h>

class SingleSweepServo {
  Servo servo;                    // the servo
  int pos;                        // current servo position 
  int posInit;                    // inital servo position
  int posSweep;                   // sweeped servo position
  int increment;                  // increment to move for each interval
  int updateInterval;             // interval between updates (ms)
  unsigned long lastUpdate = 0;   // last update of position
  bool halfSweepComplete = false; // if servo has reached posSweep position
  bool sweepComplete = false;     // if servo has completed full sweep
 
public: 
  SingleSweepServo(int interval, int pos1, int pos2) {
    updateInterval = interval;
    posSweep = pos1;
    posInit = pos2;
    increment = 1;
  }
  
  void Attach(int pin) {
    servo.attach(pin);
  }

  void Initialize() {
    servo.write(posInit); // start at initial position
    pos = posInit;
  }
  
  void Detach() {
    servo.detach();
  }
  
  void Update() {
    if((millis() - lastUpdate) > updateInterval) {   // time to update
      lastUpdate = millis();
      if ((pos >= posInit)||(pos <= posSweep)) {
        increment = -increment;  // reverse direction when bounds reached
      }
      pos += increment;
      servo.write(pos);
      Serial.println(pos);
      
      // checks for a completed sweep
      if (pos <= posSweep) {
        halfSweepComplete = true;
      }
      else if (pos >= posInit) {
        if (halfSweepComplete) {
          sweepComplete = true;
        }
        else {
          sweepComplete = false;
        }
        halfSweepComplete = false;
      }
      else {
        sweepComplete = false;
      }   
    }
  }
  
  bool isSweepComplete() {
    return sweepComplete;
  }
  
  bool isAtInitialPos() {
    return (pos >= posInit);
  }
}; //class SingleSweepServo
