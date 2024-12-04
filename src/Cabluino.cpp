#include "Cabluino.h"

SLIPEncodedSerial Cabluino::SLIPSerial(Serial);
uint16_t Cabluino::_wTime = 5;

void Cabluino::begin(long baudrate) {
  SLIPSerial.begin(baudrate);
}

void Cabluino::waitingTime(uint16_t us) {
  _wTime = us;
}

bool Cabluino::receive() {

  SLIPSerial.beginPacket();
  SLIPSerial.write(0XAA);
  SLIPSerial.endPacket();

  unsigned long _receiveTimer = micros();

  OSCBundle bndl;
  int size;

  while (micros() - _receiveTimer < _wTime && !SLIPSerial.available()) {
    if (_receiveTimer > micros()) {
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

  for (int i = 0; i < cablesVal<int>::numInstances; i++) {
    cablesVal<int> *inst = cablesVal<int>::instances[i];
    OSCMessage msg = bndl.getOSCMessage((char *)inst->address);
    if (!msg.hasError()) {
      for (int j = 0; j < inst->getSize(); j++) {
        if (msg.isInt(j)) {
          (*inst)[j] = msg.getInt(j);
        } 
      }
    }
  }

  for (int i = 0; i < cablesVal<float>::numInstances; i++) {
    cablesVal<float> *inst = cablesVal<float>::instances[i];
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

void Cabluino::send(OSCBundle &bundleOUT) {
  SLIPSerial.beginPacket();
  bundleOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
}