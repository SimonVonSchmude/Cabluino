/*
**********************************************************************

Example for sending analog sensor data to Cables.

This library connects to the Cabluino OP in Cables.

2024, by Simon von Schmude

**********************************************************************
*/

#include <Cabluino.h>

const int sensor = A0;  // connect analog sensor to pin A0 of Arduino Uno

const int fps = 60;       // specify how many times per second the Arduino should send the data to cables at max. Depending on the code, the resulting sends per second could be lower.
int timePerFrame;         // var to save the needed time for the timer function below
unsigned long timer = 0;  // create var to occasionally save the current time

void setup() {
  Cabluino.begin(115200);  // begin connection with Cables. Make sure to use the same baud rate as in the Cabluino OP!

  pinMode(sensor, INPUT);  // define the sensor pin as an input

  timePerFrame = 1000 / fps;
}

void loop() {
  /*
    the sensor reading and data sending is wrapped in this if function
    to avoid overwhelming cables with too much data. It is not
    necessary in this example, but a good technique to know.

    Adjust the "fps" variable according to what speed you want/need.
    */
  if (millis() - timer > timePerFrame) {  // check if enough time till the next frame has passed. millis() gives the time passed since turning the Arduino on in milliseconds
    timer = millis();                     // save current time for the next check

    int sensorReading = analogRead(sensor);  // read sensor and save data in newly created var

    OSCBundle bndl;                      // create OSC bundle to be sent to Cables
    bndl.add("/ar").add(sensorReading);  // add the value of sensorReading to the address "/ar" (the shorter the address, the better)
    Cabluino.send(bndl);                 // send the bundle with all its data to Cables
  }
}