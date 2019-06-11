#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
private:
  int triggerPinNumber;
  int echoPinNumber;
  int sensorSide; // left or right
  int sensorDelay;

public:
  // constructors
  Sensor();
  Sensor(int trigger, int echo, int side, int delay);

  // setters
  void setTriggerPin(int trig);
  void setEchoPin(int echo);
  void setSide(int side);
  void setDelay(int delay);

  // getters
  int triggerPin();
  int echoPin();
  int side();
  int delay();

  // destructor
  virtual ~Sensor();
};

#endif

