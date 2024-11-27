#include "OSCSerial.h"
// #include <OSCBundle.h>
// #include <SLIPEncodedSerial.h>

SLIPEncodedSerial OSCSerial::SLIPSerial(Serial);
uint16_t OSCSerial::_wTime = 5;

void OSCSerial::begin(long baudrate) {
  SLIPSerial.begin(baudrate);
}

void OSCSerial::waitingTime(uint16_t us) {
  _wTime = us;
}

bool OSCSerial::receive() {

  SLIPSerial.beginPacket();
  SLIPSerial.write(0XAA);
  SLIPSerial.endPacket();

  unsigned long _receiveTimer = micros();

  OSCBundle bndl;
  int size;

  while (micros() - _receiveTimer < _wTime && !SLIPSerial.available()) {
    if(_receiveTimer > micros()){
      _receiveTimer = micros();
    }
  }

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
        }else if (msg.isInt(j)) {
          (*inst)[j] = (float)msg.getInt(j);
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