#ifndef OSCSerial_h
#define OSCSerial_h

#include <OSCBundle.h>
#include <SLIPEncodedSerial.h>

class OSCSerial {
public:
  OSCSerial();
  static void begin(long baudrate);
  static void send(OSCBundle& bundle);
  static bool receive();

private:
  static SLIPEncodedSerial SLIPSerial;
};

extern OSCSerial OSCSerial;

// template<typename T> struct Type;

template<typename T>
class anchor {
private:
  static const int MAX_INSTANCES = 12;

public:
  anchor(const char* addr)
    : address(addr) {
    if (numInstances < MAX_INSTANCES) {
      instances[numInstances++] = this;
    }
  }

  static int numInstances;
  static anchor* instances[MAX_INSTANCES];
  
  const char* address;
  T value;
};

template<typename T>
anchor<T>* anchor<T>::instances[MAX_INSTANCES] = { nullptr };

template<typename T>
int anchor<T>::numInstances = 0;

#endif
