#include "swiper.h"
#include "Sensor.h"

pthread_cond_t COND;
pthread_mutex_t MUTEX;
pthread_barrier_t BARR_TOP, BARR_BOT;

bool STDOUT_THRD_READY = false;

int SENSOR_DELAY = 1250;
int THRTL_SENSOR = 1;
int THRTL_DELAY = 0;
int MAX_DELAY = 4000;

std::array<int, 2> INACT_CNT = {0, 0};
std::array<float, 2> SENSOR_OUTPUT = {-10000.0, -10000.0};

void signalCatcher(int sig) {
  // catch the signal and clean up
  std::cout << "Shutting down swiper MMM-simple-swiper." << std::endl;
  wait(0);
  exit(0);
}

void busyWaitForStdoutThread(void) {
  do {
    // only called once at the start of the program to
    // guarantee the stdout handler thread gets started first
  } while (!STDOUT_THRD_READY);
}

float average(std::array<float, NUM_SAMPLES> vals) {
  int i = 0;
  float sum = 0.0;

  // only use the first half of the values to remove extreme outliers
  for (i = 0; i < (NUM_SAMPLES / 2); i++) {
    sum += vals[i];
  }

  return (sum / (NUM_SAMPLES / 2));
}

// NEED TO TRANSLATE THIS TO C++
void stdoutHandler(void) {
  STDOUT_THRD_READY = true;

  while (1) {
    // only wake the thread when we need to actually do something
    pthread_cond_wait(&COND, &MUTEX);

    // print the output to the parent process in node_helper.js
    pthread_mutex_lock(&MUTEX);
    DEBUG_PRINT("STDOUT Thread received signal\n");

    std::cout << SENSOR_OUTPUT[0] << ":" << SENSOR_OUTPUT[1] << std::endl;

    // shouldn't need the fflush anymore with std::endl;
    // fflush(stdout);
    pthread_mutex_unlock(&MUTEX);
  }
}

// NEED TO TRANSLATE THIS TO C++
void sensorDistance(Sensor &sensor) {

  int i = 0;
  std::array<float, NUM_SAMPLES> distance;

  float prev_dist = -1000.0, curr_dist = 0;
  long int start = 0, end = 0, elapsed = 0;

  while (1) {
    start = (long int)time(NULL);
    end = (long int)time(NULL);
    elapsed = 0;

    DEBUG_PRINT("Thread %d at top barrier\n", sensor.side);

    // probably unnecessary to have this barrier
    // pthread_barrier_wait(&BARR_TOP);

    for (i = 0; i < NUM_SAMPLES; i++) {
      digitalWrite(sensor.trigger, HIGH);
      delayMicroseconds(100);
      digitalWrite(sensor.trigger, LOW);

      while (digitalRead(sensor.echo) == LOW) {
        // do nothing until the read changes
      }

      start = micros();

      while (digitalRead(sensor.echo) == HIGH) {
        // do nothing until the read changes
      }

      end = micros();
      elapsed = end - start;
      distance.at(i) = elapsed / 58;
    }

    // sort the values, then write average to global array
    std::sort(distance.begin(), distance.end());
    curr_dist = average(distance);

    // write the output value to the global array for each thread
    SENSOR_OUTPUT[sensor.side] = curr_dist;

    DEBUG_PRINT("Thread %d at bottom barrier\n", sensor.side);
    pthread_barrier_wait(&BARR_BOT);

    if (fabs(curr_dist - prev_dist) > 20.0) {
      // signal the STDOUT thread and reset the inactive counts for the threads
      pthread_cond_signal(&COND);
      INACT_CNT[sensor.side] = sensor.throttleDelay = 0;

    } else {
      INACT_CNT[sensor.side]++;

      // the max function is an inline
      if (sensor.side == RIGHT) {
        INACT_CNT[LEFT] = INACT_CNT[RIGHT] =
            std::max(INACT_CNT[LEFT], INACT_CNT[RIGHT]);
      }
    }

    prev_dist = curr_dist;

    // this whole thing might just get removed
    // make sure both threads get the same inactive count
    pthread_barrier_wait(&BARR_TOP);

    if (THRTL_SENSOR && INACT_CNT[sensor.side] > 0 &&
        INACT_CNT[sensor.side] % 10 == 0) {

      if (sensor.throttleDelay < MAX_DELAY) {
        sensor.throttleDelay += 125; // add an eighth second
      } else {
        sensor.throttleDelay = MAX_DELAY;
      }
    }

    DEBUG_PRINT("Thread %d throttleDelay = %d\n", sensor.side,
                sensor.throttleDelay);
    DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor.side,
                INACT_CNT[sensor.side]);

    // this may be removed
    if (THRTL_SENSOR && INACT_CNT[sensor.side] > 0 &&
        INACT_CNT[sensor.side] % 125 == 0) {
      // try resetting page to home page after a while in here, if they want
      DEBUG_PRINT("Resetting to home page\n");
    }

    usleep((SENSOR_DELAY + sensor.throttleDelay) * 1000);
  }
}

void parseJSON(Sensor sensor[2], char *JSON) {

  std::string configType, configValue;
  char currChar;

  int len = strlen(JSON) + 1;
  int i = 0, side;

  for (i = 0; i < len; i++) {
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
        THRTL_SENSOR = configValue.find("true") ? 1 : 0;

      } else if (configType.find("maxDelay")) {
        MAX_DELAY = std::stoi(configValue);
      }

      configType.clear();
      configValue.clear();
    }
  }
}
