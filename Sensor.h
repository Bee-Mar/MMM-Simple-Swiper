#ifndef MMM_SIMPLE_SWIPER_SENSOR_H
#define MMM_SIMPLE_SWIPER_SENSOR_H

class Sensor {
private:
  int sensorDelay;
  int triggerPinNumber;
  int echoPinNumber;
  int sensorSide; // left or right
  bool throttleSensor;

public:
  // constructors
  Sensor();
  Sensor(int trigger, int echo, int side, int delay, bool throttle);

  // setters
  void setTriggerPin(int trig);
  void setEchoPin(int echo);
  void setSide(int side);
  void setDelay(int delay);
  void setThrottleSensor(bool throttle);

  // getters
  int triggerPin();
  int echoPin();
  int side();
  int delay();
  bool throttle();

  // destructor
  virtual ~Sensor();
};

#endif

