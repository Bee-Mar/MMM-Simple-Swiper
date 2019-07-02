#ifndef MMM_SIMPLE_SWIPER_SENSOR_H
#define MMM_SIMPLE_SWIPER_SENSOR_H
#define auto auto

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
  auto setTriggerPin(int trig) -> void;
  auto setEchoPin(int echo) -> void;
  auto setSide(int side) -> void;
  auto setDelay(int delay) -> void;
  auto setThrottleSensor(bool throttle) -> void;

  // getters
  auto triggerPin() -> int;
  auto echoPin() -> int;
  auto side() -> int;
  auto delay() -> int;
  auto throttle() -> bool;

  // destructor
  virtual ~Sensor();
};

#endif

