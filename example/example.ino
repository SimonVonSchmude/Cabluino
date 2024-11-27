#include "OSCSerial.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

anchor<int> led1("/led1");
anchor<int> led2("/led2");
anchor<int> led3("/led3");
anchor<int> ledStripR("/ledstripR", LED_COUNT);
anchor<int> ledStripG("/ledstripG", LED_COUNT);
anchor<int> ledStripB("/ledstripB", LED_COUNT);

uint8_t wave = 0;

long timer = 0;

void setup() {
  OSCSerial.begin(115200);
  OSCSerial.waitingTime(5);

  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  strip.begin();           
  strip.show();             
  strip.setBrightness(30);  
}

void loop() {

  if (OSCSerial.receive()) {

    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(ledStripR.value[i], ledStripG.value[i], ledStripB.value[i]));
    }
    strip.show();

    OSCBundle bndl;
    bndl.add("/value").add(led1.value[0]);
    bndl.add("/wave").add(wave);
    bndl.add("/time/time").add((long)millis());
    OSCSerial.send(bndl);

    wave++;
  }



  // analogWrite(11, led1.value[0]);
  // analogWrite(10, led2.value[0]);
  // analogWrite(9, led3.value[0]);
}
