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
  int i{0};
  float sum{0};

  // only use the first half of the values to trim extreme outliers
  for (i; i < HALF_NUM_SAMPLES; i++) { sum += vals[i]; }

  return (sum / HALF_NUM_SAMPLES);
}

// NEED TO TRANSLATE THIS TO C++
void stdoutHandler(void) {
  STDOUT_THRD_READY = true;

  while (1) {
    // wake the thread when we need to print the output to node_helper.js
    lock.lock();
    condition.wait(lock);

    DEBUG_PRINT("STDOUT Thread received signal from sensor threads.\n");
    std::cout << SENSOR_OUTPUT[0] << ":" << SENSOR_OUTPUT[1] << std::endl;

    lock.unlock();
  }
}

// NEED TO TRANSLATE THIS TO C++
void sensorDistance(Sensor &sensor, boost::barrier &barrier) {

  std::array<float, NUM_SAMPLES> distance;

  int i{0};
  long int start{0}, end{0}, elapsed{0};
  float prev_dist{-1000.0}, curr_dist{0};

  while (1) {
    start = (long int)time(NULL);
    end = (long int)time(NULL);
    elapsed = 0;

    DEBUG_PRINT("Thread %d at top barrier\n", sensor.side);

    for (i; i < NUM_SAMPLES; i++) {
      digitalWrite(sensor.trigger, HIGH);
      delayMicroseconds(100);
      digitalWrite(sensor.trigger, LOW);

      // do nothing until the read changes
      while (digitalRead(sensor.echo) == LOW) {}
      start = micros();

      while (digitalRead(sensor.echo) == HIGH) {}
      end = micros();

      elapsed = end - start;
      distance.at(i) = (elapsed / 58);
    }

    // sort the values, then write average to global array
    std::sort(distance.begin(), distance.end());
    curr_dist = average(distance);

    // write the output value to the global array for each thread
    SENSOR_OUTPUT[sensor.side] = curr_dist;

    DEBUG_PRINT("Thread %d at bottom barrier\n", sensor.side);

    barrier.wait();

    if (std::fabs(curr_dist - prev_dist) > 20.0) {
      // signal the STDOUT thread and reset the inactive counts for the threads
      condition.notify_one();
      INACT_CNT[sensor.side] = sensor.delay = 0;

    } else {
      INACT_CNT[sensor.side]++;

      if (sensor.side == RIGHT) {
        // take the maximum of the recorded delays and assign it to both sensors
        INACT_CNT[LEFT] = INACT_CNT[RIGHT] = std::max(INACT_CNT[LEFT], INACT_CNT[RIGHT]);
      }
    }

    prev_dist = curr_dist;
    barrier.wait();

    // short circuit the if statement if the THRTL_SENSOR is set to false
    if (THRTL_SENSOR && (INACT_CNT[sensor.side] > 0) && (INACT_CNT[sensor.side] % 10 == 0)) {
      // add an eighth of a second if we haven't hit MAX_DELAY
      sensor.delay = (sensor.delay < MAX_DELAY) ? (sensor.delay + 125) : (MAX_DELAY);
    }

    DEBUG_PRINT("Thread %d throttleDelay = %d\n", sensor.side, sensor.delay);
    DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor.side, INACT_CNT[sensor.side]);

    // this may be removed entirely
    // if (THRTL_SENSOR && (INACT_CNT[sensor.side] > 0) && (INACT_CNT[sensor.side] % 125 == 0)) {
    //   // try resetting page to home page after a while in here, if they want
    //   DEBUG_PRINT("Resetting to home page\n");
    // }

    usleep((SENSOR_DELAY + sensor.delay) * 1000);
  }
}

void parseJSON(Sensor sensor[2], char *JSON) {

  std::string configType, configValue;

  int i{0}, side{0}, len(strlen(JSON) + 1);

  char currChar(tolower(JSON[i]));

  for (i; i < len; i++) {
    currChar = tolower(JSON[i]);

    if (isalpha(currChar)) {
      configType.push_back(currChar);

    } else if (isdigit(currChar)) {
      configValue.push_back(currChar);

    } else if (currChar == ',' || currChar == '}') {
      side = configType.find("right") ? RIGHT : LEFT;

      if (configType.find("trigger")) {
        sensor[side].trigger = std::stoi(configValue);

      } else if (configType.find("echo")) {
        sensor[side].echo = std::stoi(configValue);

      } else if (configType.find("delay")) {
        DEBUG_PRINT("DELAY: %d\n", atoi(configValue));
        SENSOR_DELAY = std::stoi(configValue);

      } else if (configType.find("throttleSensor")) {
        THRTL_SENSOR = configValue.find("true") ? true : false;

      } else if (configType.find("maxDelay")) {
        MAX_DELAY = std::stoi(configValue);
      }

      configType.clear();
      configValue.clear();
    }
  }
}
