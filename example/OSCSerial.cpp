#include "pins_arduino.h"
// #include "Arduino.h"
#include "OSCSerial.h"
// #include <OSCBundle.h>
// #include <SLIPEncodedSerial.h>

SLIPEncodedSerial OSCSerial::SLIPSerial(Serial);

void OSCSerial::begin(long baudrate) {
  SLIPSerial.begin(baudrate);
}

bool OSCSerial::receive() {
Serial.write(0xAA);

  OSCBundle bndl;
  int size;

  if (!SLIPSerial.available()) {
    return false;
  }

  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--) {
        bndl.fill(SLIPSerial.read());
      }
    }
  }

  if (!bndl.size()) {
    return false;
  }

  if (bndl.hasError()) {
    return false;
  }

  for (int i = 0; i < anchor<int>::numInstances; i++) {
    anchor<int> *inst = anchor<int>::instances[i];
    OSCMessage msg = bndl.getOSCMessage((char *)inst->address);
    if (!msg.hasError()) {
      for (int j = 0; j < inst->getSize(); j++) {
        if (msg.isInt(j)) {
          (*inst)[j] = msg.getInt(j);
        } else if (msg.isFloat(j)) {
          (*inst)[j] = (int)msg.getFloat(j);
        }
      }
    }
  }

  for (int i = 0; i < anchor<float>::numInstances; i++) {
    anchor<float> *inst = anchor<float>::instances[i];
    OSCMessage msg = bndl.getOSCMessage((char *)inst->address);
    if (!msg.hasError()) {
      for (int j = 0; j < inst->getSize(); j++) {
        if (msg.isFloat(j)) {
          (*inst)[j] = msg.getFloat(j);
        }
      }
    }
  }

  return true;
}

void OSCSerial::send(OSCBundle &bundleOUT) {
  SLIPSerial.beginPacket();
  bundleOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
}