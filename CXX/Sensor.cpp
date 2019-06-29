#include "Sensor.h"

// constructors
Sensor::Sensor() : sensorDelay(0) {
}

Sensor::Sensor(int delay, int trigger, int echo, int side, bool throttle)
    : sensorDelay(delay), triggerPinNumber(trigger), echoPinNumber(echo), sensorSide(side),
      throttleSensor(throttle) {
}

// setters
void Sensor::setTriggerPin(int trigger) {
  triggerPinNumber = trigger;
}

void Sensor::setEchoPin(int echo) {
  echoPinNumber = echo;
}

void Sensor::setSide(int side) {
  sensorSide = side;
}

void Sensor::setDelay(int delay) {
  sensorDelay = delay;
}

void Sensor::setThrottleSensor(bool throttle) {
  throttleSensor = throttle;
}

// getters
int Sensor::triggerPin() {
  return triggerPinNumber;
}

int Sensor::echoPin() {
  return echoPinNumber;
}

int Sensor::side() {
  return sensorSide;
}

int Sensor::delay() {
  return sensorDelay;
}

bool Sensor::throttle() {
  return throttleSensor;
}

// destructor
Sensor::~Sensor() {
}
