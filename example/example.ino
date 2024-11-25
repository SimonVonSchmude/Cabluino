#include "OSCSerial.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3
#define LED_COUNT 14
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


anchor<int> led1("/led1");
anchor<int> led2("/led2");
anchor<int> led3("/led3");
anchor<int> ledStripR("/ledstripR", LED_COUNT);
anchor<int> ledStripG("/ledstripG", LED_COUNT);
anchor<int> ledStripB("/ledstripB", LED_COUNT);

long timer = 0;

void setup() {
  OSCSerial.begin(115200);

  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(30);  // Set BRIGHTNESS to about 1/5 (max = 255)jjhjh
}

void loop() {

  if (millis() - timer > 8) {
    if(OSCSerial.receive()){

    analogWrite(11, led1.value[0]);
    analogWrite(10, led2.value[0]);
    analogWrite(9, led3.value[0]);

    timer = millis();

    OSCBundle bndl;
    bndl.add("/value").add(led1.value[0]);
    bndl.add("/time/time").add((long)millis());
    OSCSerial.send(bndl);
    }
}
}