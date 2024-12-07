/*
**********************************************************************

Example for connecting LEDs to Cables.

This library connects to the Cabluino OP in Cables.

For now, the following Data Types are available:
integers
floats

2024, by Simon von Schmude

**********************************************************************
*/

#include <Cabluino.h>  // include Cabluino Library

cablesVal<int> led1("/led1");  // prepare to receive the LED's brightness values from cables as ints using the address "/led1"
cablesVal<int> led2("/led2");  // prepare to receive the LED's brightness values from cables as ints using the address "/led1"
cablesVal<int> led3("/led3");  // prepare to receive the LED's brightness values from cables as ints using the address "/led1"

const int led1Pin = 9;   // define pin number for LED. Make sure this is a PWM pin (~x)!
const int led2Pin = 10;  // define pin number for LED. Make sure this is a PWM pin (~x)!
const int led3Pin = 11;  // define pin number for LED. Make sure this is a PWM pin (~x)!

void setup() {
  Cabluino.begin(115200);   // begin connection with Cables. Make sure to use the same baud rate as in the Cabluino OP!
  Cabluino.waitingTime(5);  // maximum response time for Cables to get back to Arduino

  pinMode(led1Pin, OUTPUT);  // define LED pin as output
  pinMode(led2Pin, OUTPUT);  // define LED pin as output
  pinMode(led3Pin, OUTPUT);  // define LED pin as output
}

void loop() {
  if (Cabluino.receive()) {               // if Arduino receives a valid message from Cables...
    analogWrite(led1Pin, led1.value[0]);  // ... take the received "/led1" value and write it as the LEDs analog brightness
    analogWrite(led2Pin, led2.value[0]);  // ... take the received "/led2" value and write it as the LEDs analog brightness
    analogWrite(led3Pin, led3.value[0]);  // ... take the received "/led3" value and write it as the LEDs analog brightness
  }

  delay(10);  // delay to avoid creating unnecessary data transfers.
}
