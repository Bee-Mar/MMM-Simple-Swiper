#include "swiper.h"
#include "Sensor.h"

// used for synchronization between each of the threads
boost::asio::io_service service;
boost::asio::io_service::work work(service);
boost::barrier threadBarrier(2);

// inactive count/tracker for each sensor, and the recorded distances for each sensor
std::array<int, 2> INACT_CNT{0, 0};
std::array<float, 2> SENSOR_OUTPUT{-10'000.0, -10'000.0};

int THRTL_DELAY{0};
int SENSOR_DELAY{1'250};
int MAX_DELAY{4'000};

// inline functions in header file: substringExists & errorMessage

auto signalCatcher(int sig) -> void {
  // catch the signal and clean up
  std::cout << "Shutting down swiper MMM-simple-swiper." << std::endl;
  wait(0);
  exit(0);
}

auto average(std::array<float, NUM_SAMPLES> vals) -> float {
  float sum{0};

  // only use the first half of the values to trim extreme outliers
  for (int i{0}; i < HALF_NUM_SAMPLES; i++) { sum += vals[i]; }

  return (sum / HALF_NUM_SAMPLES);
}

auto stdoutHandler() -> void {
  std::cout << SENSOR_OUTPUT[LEFT] << ":" << SENSOR_OUTPUT[RIGHT] << std::endl;
}

auto sensorDistance(Sensor &sensor) -> void {
  std::array<float, NUM_SAMPLES> distance;

  long int start{0}, end{0}, elapsed{0};

  float prev_dist{-1'000.0}, curr_dist{0};

  while (true) {
    start = end = static_cast<long int>(time(NULL));
    elapsed = 0;

    for (int i{0}; i < NUM_SAMPLES; i++) {
      digitalWrite(sensor.triggerPin(), HIGH);

      delayMicroseconds(100);

      digitalWrite(sensor.triggerPin(), LOW);

      // do nothing until the read changes
      while (digitalRead(sensor.echoPin()) == LOW) {}

      start = micros();

      while (digitalRead(sensor.echoPin()) == HIGH) {}

      end = micros();

      elapsed = (end - start);
      distance.at(i) = (elapsed / 58);
    }

    // sort the values, then write average to global array
    std::sort(distance.begin(), distance.end());
    curr_dist = average(distance);

    // write the output value to the global array for each thread
    SENSOR_OUTPUT[sensor.side()] = curr_dist;

    threadBarrier.wait();

    if (std::fabs(curr_dist - prev_dist) > 20.0) {
      if (sensor.side() == RIGHT) { // print the contents of the global array to stdout
        service.post(boost::bind(&stdoutHandler));
        service.run_one();
      }

      INACT_CNT[sensor.side()] = 0;
      sensor.setDelay(0);

    } else {
      INACT_CNT[sensor.side()]++;

      if (sensor.side() == RIGHT) {
        // take the maximum of the recorded delays and assign it to both sensors
        INACT_CNT[LEFT] = INACT_CNT[RIGHT] = std::max(INACT_CNT[LEFT], INACT_CNT[RIGHT]);
      }
    }

    prev_dist = curr_dist;
    threadBarrier.wait();

    // short circuit the if statement if the sensor throttle is set to false
    if (sensor.throttle() && INACT_CNT[sensor.side()] > 0 && (INACT_CNT[sensor.side()] % 10 == 0)) {
      // add an eighth of a second if we haven't hit MAX_DELAY
      sensor.setDelay((sensor.delay() < MAX_DELAY) ? (sensor.delay() + 125) : (MAX_DELAY));
    }

    DEBUG_PRINT("Thread %d throttleDelay = %d\n", sensor.side(), sensor.delay());
    DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor.side(), INACT_CNT[sensor.side()]);

    // this may be removed entirely
    // if (THRTL_SENSOR && (INACT_CNT[sensor.side] > 0) && (INACT_CNT[sensor.side] % 125 == 0)) {
    //   // try resetting page to home page after a while in here, if they want
    //   DEBUG_PRINT("Resetting to home page\n");
    // }

    usleep((SENSOR_DELAY + sensor.delay()) * 1'000);
  }
}

auto parseJSON(Sensor sensor[2], char *JSON) -> void {
  std::string configType, configValue;

  const int len(strlen(JSON) + 1);
  int side{0};

  sensor[LEFT].setSide(LEFT);
  sensor[RIGHT].setSide(RIGHT);

  for (int i{0}; i < len; i++) {
    const char currChar(tolower(JSON[i]));

    if (isalpha(currChar)) {
      configType.push_back(currChar);

    } else if (isdigit(currChar)) {
      configValue.push_back(currChar);

    } else if (currChar == ',' || currChar == '}') {
      side = (substringExists(configType.find("right")) ? RIGHT : LEFT);

      if (substringExists(configType.find("trigger"))) {
        sensor[side].setTriggerPin(std::stoi(configValue));

      } else if (substringExists(configType.find("echo"))) {
        sensor[side].setEchoPin(std::stoi(configValue));

      } else if (substringExists(configType.find("delay"))) {
        SENSOR_DELAY = std::stoi(configValue);

      } else if (substringExists(configType.find("throttleSensor"))) {
        sensor[side].setThrottleSensor(substringExists(configType.find("true")));

      } else if (substringExists(configType.find("maxDelay"))) {
        MAX_DELAY = std::stoi(configValue);
      }

      configType.clear();
      configValue.clear();
    }
  }
}
