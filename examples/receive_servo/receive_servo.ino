/*
**********************************************************************

Example for connecting a servo to Cables.

This library connects to the Cabluino OP in Cables.

For now, the following Data Types are available:
integers
floats

2024, by Simon von Schmude

**********************************************************************
*/

#include <Cabluino.h>  // include Cabluino Library
#include <Servo.h>     // include Servo Library

Servo myServo;  // create servo object with the name "myServo"

cablesVal<int> servoAngle("/servo");  // prepare to receive the servo's angle values from cables as ints using the address "/servo"

void setup() {
  myServo.attach(9);  // connect servo to pin 9 on the Arduino

  Cabluino.begin(115200);   // begin connection with Cables. Make sure to use the same baud rate as in the Cabluino OP!
  Cabluino.waitingTime(5);  // maximum response time for Cables to get back to Arduino
}

void loop() {
  if (Cabluino.receive()) {              // if Arduino receives a valid message from Cables...
    myServo.write(servoAngle.value[0]);  // ... take the received "/servo" value and write it as an angle to the servo motor
  }

  delay(10);  // delay for giving the servo time to reach its destination
}
