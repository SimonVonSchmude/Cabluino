#include "OSCSerial.h"

anchor<int> led1("/led1");
anchor<int> led2("/led2");
anchor<int> led3("/led3");

long timer = 0;

void setup() {
  OSCSerial.begin(115200);

  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {

  OSCSerial.receive();

  analogWrite(9, led1.value);
  analogWrite(10, led2.value);
  analogWrite(11, led3.value);

  if (millis() - timer > 16) {
    timer = millis();
    OSCBundle bndl;

    bndl.add("/poti").add(analogRead(A0));
    bndl.add("/time/time").add((long)millis());

    OSCSerial.send(bndl);
  }
}