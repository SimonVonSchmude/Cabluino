/*
*****************************************************/Users/simon/Documents/gitlab/Cabluino/src/Cabluino.cpp*****************

Example for sending and receiving data between an Arduino and Cables 
using the Cabluino Library.

This library connects to the Cabluino OP in Cables.

For now, the following Data Types are available:
integers
floats

2024, by Simon von Schmude & Lion Beck

**********************************************************************
*/

#include <Cabluino.h>

/*
Specify the type of data you want to receive, the var name you want to 
associate with it and its address with the cablesVal class as follows:

cablesVal<dataType> varName("/dataAddress");

The leading "/" is important!

Try to keep the addresses as short as possible as they fill up 
the bandwidth!
*/
cablesVal<int> led1("/led1");  // When cables sends an object with the key "led1", the value of the int "led1" will be updated
cablesVal<int> led2("/led2");
cablesVal<int> led3("/led3");
cablesVal<float> mousePos("/mp");

int pinLed1 = 9;
int pinLed2 = 10;
int pinLed3 = 11;

int pinPoti = A0;

void setup() {
  /*
  Cabluino.begin(baudRate) is similar to Serial.begin(). As such, 
  make sure to use the same baudrate in the Cabluino OP in cables!

  Higher baud rates allow you to send more data, but increases the 
  chance of data corruption.
  */
  Cabluino.begin(115200);

  /*
  A timeout time specified in us (micro seconds) that gives Cables
  some time to respond to the send request by the Arduino.

  Default is 5us
  */
  Cabluino.waitingTime(5);

  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  pinMode(pinLed3, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(pinPoti, INPUT);
}

void loop() {
  /*
  Cabluino.receive() returns a bool that is true when valid data 
  has been received. Calling this function also requests Cables
  to send data, so call it whenever you need your vars to be 
  up to date!
  */
  if (Cabluino.receive()) {
    /*
    The Cabluino.receive() function updates all the previously
    created cablesVal instances' values. These values can be 
    accessed by varName.value[index]. Since receiving arrays
    is possible, using indexes is necessary. In case of receiving
    a single value per address, simply use varName.value[0]
    */
    analogWrite(pinLed1, led1.value[0]);
    analogWrite(pinLed2, led2.value[0]);
    analogWrite(pinLed3, led3.value[0]);

    if (mousePos.value[0] >= 0.5) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }

    int potiValue = analogRead(pinPoti);

    /*
    For now, this uses the OSC lib send methods, but will be updated
    with custom ones.
    */
    OSCBundle bndl;                              // Create a bundle of data to be sent
    bndl.add("/poti").add(potiValue);            // Add address "/poti" and add the value of the poti to it to be sent to the Cables patch
    bndl.add("/echo").add(led1.value[0]);        // Echo for data integrity check
    bndl.add("/time/time").add((long)millis());  // Some time sending, just for fun!
    Cabluino.send(bndl);                         // Finally, after adding all data, send the bundle to Cables
  }
}
