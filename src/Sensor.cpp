#include "Sensor.h"

// constructors
Sensor::Sensor() {}

Sensor::Sensor(int trigger, int echo, int side, int delay)
    : triggerPinNumber(trigger), echoPinNumber(echo), sensorSide(side), sensorDelay(delay) {}

// setters
void Sensor::setTriggerPin(int trigger) { triggerPinNumber = trigger; }
void Sensor::setEchoPin(int echo) { echoPinNumber = echo; }
void Sensor::setSide(int side) { sensorSide = side; }
void Sensor::setDelay(int delay) { sensorDelay = delay; }

// getters
int Sensor::triggerPin() { return triggerPinNumber; }
int Sensor::echoPin() { return echoPinNumber; }
int Sensor::side() { return sensorSide; }
int Sensor::delay() { return sensorDelay; }

// destructors
Sensor::~Sensor() {}
