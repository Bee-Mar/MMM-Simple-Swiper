#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
  int trigger;
  int echo;
  int signal;
  int side;
  int throttleDelay;

  Sensor();
  Sensor(int trigger, int echo);

  virtual ~Sensor();
};

#endif

