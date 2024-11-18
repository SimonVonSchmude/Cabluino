// #include "Arduino.h"
#include "OSCSerial.h"
// #include <OSCBundle.h>
// #include <SLIPEncodedSerial.h>

SLIPEncodedSerial OSCSerial::SLIPSerial(Serial);

void OSCSerial::begin(long baudrate) {
  SLIPSerial.begin(baudrate);
}

bool OSCSerial::receive() {
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
    OSCMessage msg = bndl.getOSCMessage((char*)anchor<int>::instances[i]->address);
    if (!msg.hasError() && msg.isInt(0)) {
      int _val = msg.getInt(0);
      anchor<int>::instances[i]->value = _val;
    }
  }

  for (int i = 0; i < anchor<float>::numInstances; i++) {
    OSCMessage msg = bndl.getOSCMessage((char*)anchor<float>::instances[i]->address);
    if (!msg.hasError() && msg.isFloat(0)) {
      float _val = msg.getFloat(0);
      anchor<float>::instances[i]->value = _val;
    }
  }

  return true;
}

void OSCSerial::send(OSCBundle& bundleOUT) {
  SLIPSerial.beginPacket();
  bundleOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
}