#include "Sensor.h"

Sensor::Sensor() {}

Sensor::Sensor(int trigger, int echo)
    : side(side), trigger(trigger), echo(echo) {}

Sensor::~Sensor() {}
