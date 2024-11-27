#ifndef OSCSerial_h
#define OSCSerial_h

#include <OSCBundle.h>
#include <SLIPEncodedSerial.h>

class OSCSerial {
public:
  OSCSerial();
  static void begin(long baudrate);
  static void waitingTime(uint16_t us);
  static void send(OSCBundle &bundle);
  static bool receive();

private:
  static SLIPEncodedSerial SLIPSerial;
  static uint16_t _wTime;
};

extern OSCSerial OSCSerial;

template<typename T>
class anchor {
private:
  static const int MAX_INSTANCES = 12;
  int size;

public:
  anchor(const char *addr, int arraySize = 1)
    : address(addr), size(arraySize) {
    if (numInstances < MAX_INSTANCES) {
      instances[numInstances++] = this;
    }
    value = new T[size]();
  }

  static int numInstances;
  static anchor *instances[MAX_INSTANCES];

  const char *address;
  T *value;

  int getSize() const {
    return size;
  }

  T &operator[](int index) {
    return value[index];
  }

  const T &operator[](int index) const {
    return value[index];
  }
};

template<typename T>
anchor<T> *anchor<T>::instances[MAX_INSTANCES] = { nullptr };

template<typename T>
int anchor<T>::numInstances = 0;

#endif
