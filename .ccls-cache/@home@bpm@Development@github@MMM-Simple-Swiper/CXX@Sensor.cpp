#include "Sensor.h"

// constructors
Sensor::Sensor() : sensorDelay(0) {
}

Sensor::Sensor(int delay, int trigger, int echo, int side, bool throttle)
    : sensorDelay(delay), triggerPinNumber(trigger), echoPinNumber(echo), sensorSide(side),
      throttleSensor(throttle) {
}

// setters
auto Sensor::setTriggerPin(int trigger) -> void {
  triggerPinNumber = trigger;
}

auto Sensor::setEchoPin(int echo) -> void {
  echoPinNumber = echo;
}

auto Sensor::setSide(int side) -> void {
  sensorSide = side;
}

auto Sensor::setDelay(int delay) -> void {
  sensorDelay = delay;
}

auto Sensor::setThrottleSensor(bool throttle) -> void {
  throttleSensor = throttle;
}

// getters
auto Sensor::triggerPin() -> int {
  return triggerPinNumber;
}

auto Sensor::echoPin() -> int {
  return echoPinNumber;
}

auto Sensor::side() -> int {
  return sensorSide;
}

auto Sensor::delay() -> int {
  return sensorDelay;
}

auto Sensor::throttle() -> bool {
  return throttleSensor;
}

// destructor
Sensor::~Sensor() {
}
