#include "OSCSerial.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3
#define LED_COUNT 6
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


anchor<int> led11("/led11");
anchor<int> ledStrip("/ledstrip", LED_COUNT);

long timer = 0;

void setup() {
  OSCSerial.begin(115200);

  pinMode(11, OUTPUT);

  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {

  OSCSerial.receive();

  analogWrite(11, led11.value[0]);

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(ledStrip.value[i], ledStrip.value[i], ledStrip.value[i]));
  }

  strip.show();

  if (millis() - timer > 16) {
    timer = millis();
    OSCBundle bndl;

    bndl.add("/poti").add(analogRead(A0));
    bndl.add("/time/time").add((long)millis());

    OSCSerial.send(bndl);
  }
}