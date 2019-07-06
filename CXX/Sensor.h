#ifndef MMM_SIMPLE_SWIPER_SENSOR_H
#define MMM_SIMPLE_SWIPER_SENSOR_H
#define auto auto

class Sensor {
private:
  int sensor_delay;
  int trigger_pin_number;
  int echo_pin_number;
  int sensor_side; // left or right
  bool throttleSensor;

public:
  // constructors
  Sensor(void);
  Sensor(int trigger, int echo, int side, int delay, bool throttle);

  // setters
  auto set_trigger_pin(int trig) -> void;
  auto set_echo_pin(int echo) -> void;
  auto set_side(int side) -> void;
  auto set_delay(int delay) -> void;
  auto set_throttle_sensor(bool throttle) -> void;

  // getters
  auto trigger_pin(void) -> int;
  auto echo_pin(void) -> int;
  auto side(void) -> int;
  auto delay(void) -> int;
  auto throttle(void) -> bool;

  // destructor
  virtual ~Sensor(void);
};

#endif

