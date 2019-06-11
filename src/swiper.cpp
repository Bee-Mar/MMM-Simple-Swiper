#include "swiper.h"
#include "Sensor.h"
// used for synchronization between each of the threads
boost::mutex mutex;
boost::unique_lock<boost::mutex> lock(mutex);
boost::condition_variable condition;

// inactive count/tracker for each sensor, and the recorded distances for each sensor
std::array<int, 2> INACT_CNT = {0, 0};
std::array<float, 2> SENSOR_OUTPUT = {-10000.0, -10000.0};

bool THRTL_SENSOR{false};
bool STDOUT_THRD_READY{false};

int THRTL_DELAY{0}; // haven't used yet
int SENSOR_DELAY{1250};
int MAX_DELAY{4000};

void signalCatcher(int sig) {
  // catch the signal and clean up
  std::cout << "Shutting down swiper MMM-simple-swiper." << std::endl;
  wait(0);
  exit(0);
}

void busyWaitForStdoutThread(void) {
  // used once to guarantee the stdout handler thread gets started first
  while (!STDOUT_THRD_READY) {}
}

float average(std::array<float, NUM_SAMPLES> vals) {
  float sum{0};

  // only use the first half of the values to trim extreme outliers
#pragma unroll
  for (int i{0}; i < HALF_NUM_SAMPLES; i++) { sum += vals[i]; }

  return (sum / HALF_NUM_SAMPLES);
}

void stdoutHandler(void) {
  STDOUT_THRD_READY = true;

  while (true) {
    // wake the thread when we need to print the output to node_helper.js
    lock.lock();
    condition.wait(lock);

    DEBUG_PRINT("STDOUT Thread received signal from sensor threads.\n");
    std::cout << SENSOR_OUTPUT[0] << ":" << SENSOR_OUTPUT[1] << std::endl;

    lock.unlock();
  }
}

void sensorDistance(Sensor &sensor, boost::barrier &barrier) {

  std::array<float, NUM_SAMPLES> distance;

  long int start{0}, end{0}, elapsed{0};
  float prev_dist{-1000.0}, curr_dist{0};

  while (true) {
    start = (long int)time(NULL);
    end = (long int)time(NULL);
    elapsed = 0;

    DEBUG_PRINT("Thread %d at top barrier\n", sensor.side);

    for (int i{0}; i < NUM_SAMPLES; i++) {
      digitalWrite(sensor.triggerPin(), HIGH);
      delayMicroseconds(100);
      digitalWrite(sensor.triggerPin(), LOW);

      // do nothing until the read changes
      while (digitalRead(sensor.echoPin()) == LOW) {}
      start = micros();

      while (digitalRead(sensor.echoPin()) == HIGH) {}
      end = micros();

      elapsed = end - start;
      distance.at(i) = (elapsed / 58);
    }

    // sort the values, then write average to global array
    std::sort(distance.begin(), distance.end());
    curr_dist = average(distance);

    // write the output value to the global array for each thread
    SENSOR_OUTPUT[sensor.side()] = curr_dist;

    DEBUG_PRINT("Thread %d at bottom barrier\n", sensor.side());

    barrier.wait();

    if (std::fabs(curr_dist - prev_dist) > 20.0) {
      // signal the STDOUT thread and reset the inactive counts for the threads
      condition.notify_one();
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
    barrier.wait();

    // short circuit the if statement if the THRTL_SENSOR is set to false
    if (THRTL_SENSOR && (INACT_CNT[sensor.side()] > 0) && (INACT_CNT[sensor.side()] % 10 == 0)) {
      // add an eighth of a second if we haven't hit MAX_DELAY
      if (sensor.delay() < MAX_DELAY) {
        sensor.setDelay(sensor.delay() + 125);
      } else {
        sensor.setDelay(MAX_DELAY);
      }
    }

    DEBUG_PRINT("Thread %d throttleDelay = %d\n", sensor.side, sensor.delay);
    DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor.side, INACT_CNT[sensor.side]);

    // this may be removed entirely
    // if (THRTL_SENSOR && (INACT_CNT[sensor.side] > 0) && (INACT_CNT[sensor.side] % 125 == 0)) {
    //   // try resetting page to home page after a while in here, if they want
    //   DEBUG_PRINT("Resetting to home page\n");
    // }

    usleep((SENSOR_DELAY + sensor.delay()) * 1000);
  }
}

void parseJSON(Sensor sensor[2], char *JSON) {

  std::string configType, configValue;

  int side{0}, len(strlen(JSON) + 1);

  char currChar;

  for (int i{0}; i < len; i++) {
    currChar = tolower(JSON[i]);

    if (isalpha(currChar)) {
      configType.push_back(currChar);

    } else if (isdigit(currChar)) {
      configValue.push_back(currChar);

    } else if (currChar == ',' || currChar == '}') {
      side = (configType.find("right") != std::string::npos) ? RIGHT : LEFT;

      if (configType.find("trigger") != std::string::npos) {
        sensor[side].setTriggerPin(std::stoi(configValue));

      } else if (configType.find("echo") != std::string::npos) {
        sensor[side].setEchoPin(std::stoi(configValue));

      } else if (configType.find("delay") != std::string::npos) {
        DEBUG_PRINT("DELAY: %d\n", atoi(configValue));
        SENSOR_DELAY = std::stoi(configValue);

      } else if (configType.find("throttleSensor") != std::string::npos) {
        THRTL_SENSOR = (configType.find("true") != std::string::npos) ? true : false;

      } else if (configType.find("maxDelay") != std::string::npos) {
        MAX_DELAY = std::stoi(configValue);
      }

      configType.clear();
      configValue.clear();
    }
  }
}
