#include "OSCSerial.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3
#define LED_COUNT 14
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


anchor<int> led11("/led11");
anchor<int> ledStripR("/ledstripR", LED_COUNT);
anchor<int> ledStripG("/ledstripG", LED_COUNT);
anchor<int> ledStripB("/ledstripB", LED_COUNT);

long timer = 0;

void setup() {
  OSCSerial.begin(128000);

  pinMode(11, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(30);  // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {

OSCSerial.receive();

analogWrite(11, led11.value[0]);


  // return;

  // if (OSCSerial.receive()) {
  //   digitalWrite(LED_BUILTIN, HIGH);

  //   for (int i = 0; i < LED_COUNT; i++) {
  //     strip.setPixelColor(i, strip.Color(ledStripR.value[i], ledStripG.value[i], ledStripB.value[i]));
  //   }

  //   strip.show();

  // } else {
  //   digitalWrite(LED_BUILTIN, LOW);
  // }

  if (millis() - timer > 16) {
    timer = millis();

    OSCBundle bndl;
    bndl.add("/poti").add(analogRead(A0));
    bndl.add("/value").add(led11.value[0]);
    bndl.add("/time/time").add((long)millis());
    OSCSerial.send(bndl);
  }
}