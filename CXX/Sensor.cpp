#include "Sensor.h"

// constructors
Sensor::Sensor(void) : sensor_delay(0) {
}

Sensor::Sensor(int delay, int trigger, int echo, int side, bool throttle)
    : sensor_delay(delay), trigger_pin_number(trigger), echo_pin_number(echo), sensor_side(side),
      throttleSensor(throttle) {
}

// setters
auto Sensor::set_trigger_pin(int trigger) -> void {
  trigger_pin_number = trigger;
}

auto Sensor::set_echo_pin(int echo) -> void {
  echo_pin_number = echo;
}

auto Sensor::set_side(int side) -> void {
  sensor_side = side;
}

auto Sensor::set_delay(int delay) -> void {
  sensor_delay = delay;
}

auto Sensor::set_throttle_sensor(bool throttle) -> void {
  throttleSensor = throttle;
}

// getters
auto Sensor::trigger_pin(void) -> int {
  return trigger_pin_number;
}

auto Sensor::echo_pin(void) -> int {
  return echo_pin_number;
}

auto Sensor::side(void) -> int {
  return sensor_side;
}

auto Sensor::delay(void) -> int {
  return sensor_delay;
}

auto Sensor::throttle(void) -> bool {
  return throttleSensor;
}

// destructor
Sensor::~Sensor(void) {
}
