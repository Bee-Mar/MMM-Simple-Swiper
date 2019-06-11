#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
private:
  int triggerPinNumber;
  int echoPinNumber;
  int sensorSide; // left or right
  int sensorDelay;

public:
  Sensor();
  Sensor(int trigger, int echo, int side, int delay);

  void setTriggerPin(int trig);
  void setEchoPin(int echo);
  void setSide(int side);
  void setDelay(int delay);

  int triggerPin();
  int echoPin();
  int side();
  int delay();

  virtual ~Sensor();
};

#endif

