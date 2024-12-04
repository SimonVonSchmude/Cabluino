/*
**********************************************************************

Example for sending TOF distance sensor data to Cables.

This library connects to the Cabluino OP in Cables.

2024, by Simon von Schmude

**********************************************************************
*/

#include <Cabluino.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();  // create VL53L0X object with the name "lox"

void setup() {
  Cabluino.begin(115200);  // begin connection with Cables. Make sure to use the same baud rate as in the Cabluino OP!

  if (!lox.begin()) {  // try to start the distance sensor. if it fails...
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);  // ... turn on the built in led to let us know
  }

  /*
  Here you can configure the sensor based on whats most important to you:

  VL53L0X_SENSE_DEFAULT
  VL53L0X_SENSE_LONG_RANGE
  VL53L0X_SENSE_HIGH_SPEED
  VL53L0X_SENSE_HIGH_ACCURACY

  replace the value behind the :: in the function call below with one of the above
  */
  lox.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;  // create var to save measurement data

  lox.rangingTest(&measure, false);  // measure the distance and save the result in the measure var

  if (measure.RangeStatus != 4) {                 // If the sensor's status indicates valid data...
    uint16_t distance = measure.RangeMilliMeter;  // ... get the distance in millimeters

    OSCBundle bndl;                   // create OSC bundle to be sent to Cables
    bndl.add("/dist").add(distance);  // add the value of distance to the address "/dist" (the shorter the address, the better)
    Cabluino.send(bndl);              // send the bundle with all its data to Cables
  }
}