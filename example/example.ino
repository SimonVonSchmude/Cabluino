#include "Cabluino.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

cablesVal<int> led1("/led1");
cablesVal<int> led2("/led2");
cablesVal<int> led3("/led3");
cablesVal<int> ledStripR("/ledstripR", LED_COUNT);
cablesVal<int> ledStripG("/ledstripG", LED_COUNT);
cablesVal<int> ledStripB("/ledstripB", LED_COUNT);

uint8_t wave = 0;

long timer = 0;

void setup() {
  Cabluino.begin(115200);
  Cabluino.waitingTime(5);

  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  strip.begin();
  strip.show();
  strip.setBrightness(30);
}

void loop() {

  if (Cabluino.receive()) {

    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(ledStripR.value[i], ledStripG.value[i], ledStripB.value[i]));
    }
    strip.show();

    OSCBundle bndl;
    bndl.add("/value").add(led1.value[0]);
    bndl.add("/wave").add(wave);
    bndl.add("/time/time").add((long)millis());
    Cabluino.send(bndl);

    wave++;
  }

  // analogWrite(11, led1.value[0]);
  // analogWrite(10, led2.value[0]);
  // analogWrite(9, led3.value[0]);
}
