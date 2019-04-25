#define _DEFAULT_SOURCE // that damn usleep macro requirement

#include <ctype.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#define READ 0
#define WRITE 1
#define LEFT 0
#define RIGHT 1
#define NUM_SAMPLES 10

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, args...)                                              \
  fprintf(stderr, "DEBUG_MODE: %s:%d:%s(): " fmt, __FILE__, __LINE__,          \
          __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

pthread_cond_t COND;
pthread_mutex_t MUTEX;
pthread_barrier_t BARR_TOP, BARR_BOT;

int STDOUT_THRD_READY = 0;
int SENSOR_DELAY = 1250, THRTL_SENSOR = 1, THRTL_DELAY = 0;
int MAX_DELAY = 4000;

int INACT_CNT[2] = {0, 0};
float SENSOR_OUTPUT[2] = {-10000.0, -10000.0};

struct sensor_bundle {
  int trigger;
  int echo;
  int signal;
  int side;
  int thrtl_delay;
};

void signal_catcher(int sig) {
  // catch the signal and clean up
  printf("SIGINT caught. Shutting down swiper MMM-simple-swiper.\n");
  wait(0); // wait for cleanup
  exit(0);
}

void error_msg(char *msg) {
  printf("%s\n", msg);
  exit(-1);
}

int max(int a, int b) { return a > b ? a : b; }

int compare(const void *a, const void *b) { return (*(int *)a) - (*(int *)b); }

float avg(float vals[NUM_SAMPLES]) {
  int i = 0;
  float sum = 0.0;

  // only use the first half of the values to remove extreme outliers
  for (i = 0; i < (NUM_SAMPLES / 2); i++) {
    sum += vals[i];
  }

  return (sum / (NUM_SAMPLES / 2));
}

void stdout_handler() {

  STDOUT_THRD_READY = 1;

  while (1) {
    // only wake the thread when we need to actually do something
    pthread_cond_wait(&COND, &MUTEX);

    // print the output to the parent process in node_helper.js
    pthread_mutex_lock(&MUTEX);
    DEBUG_PRINT("STDOUT Thread received signal\n");

    printf("%f:%f\n", SENSOR_OUTPUT[0], SENSOR_OUTPUT[1]);

    fflush(stdout);
    pthread_mutex_unlock(&MUTEX);
  }
}

void sensor_distance(struct sensor_bundle *sensor) {

  int i = 0;

  float distance[NUM_SAMPLES];
  float prev_dist = -1000.0, curr_dist = 0;
  long int start = 0, end = 0, elapsed = 0;

  while (1) {

    start = (long int)time(NULL);
    end = (long int)time(NULL);
    elapsed = 0;

    DEBUG_PRINT("Thread %d at top barrier\n", sensor->side);

    // probably unnecessary to have this barrier
    // pthread_barrier_wait(&BARR_TOP);

    for (i = 0; i < NUM_SAMPLES; i++) {

      digitalWrite(sensor->trigger, HIGH);
      delayMicroseconds(100);
      digitalWrite(sensor->trigger, LOW);

      while (digitalRead(sensor->echo) == LOW) {
        // do nothing until the read changes
      }

      start = micros();

      while (digitalRead(sensor->echo) == HIGH) {
        // do nothing until the read changes
      }

      end = micros();

      elapsed = end - start;
      distance[i] = elapsed / 58;
    }

    // sort the values, then write average to global array
    qsort(distance, NUM_SAMPLES, sizeof(float), compare);
    curr_dist = avg(distance);

    // write the output value to the global array for each thread
    SENSOR_OUTPUT[sensor->side] = curr_dist;

    DEBUG_PRINT("Thread %d at bottom barrier\n", sensor->side);
    pthread_barrier_wait(&BARR_BOT);

    if (fabs(curr_dist - prev_dist) > 20.0) {
      // signal the STDOUT thread and reset the inactive counts for the threads
      pthread_cond_signal(&COND);
      INACT_CNT[sensor->side] = sensor->thrtl_delay = 0;

    } else {
      INACT_CNT[sensor->side]++;

      if (sensor->side == RIGHT) {
        INACT_CNT[LEFT] = INACT_CNT[RIGHT] =
            max(INACT_CNT[LEFT], INACT_CNT[RIGHT]);
      }
    }

    prev_dist = curr_dist;

    // this whole thing might just get removed
    // make sure both threads get the same inactive count
    pthread_barrier_wait(&BARR_TOP);

    if (THRTL_SENSOR && INACT_CNT[sensor->side] > 0 &&
        INACT_CNT[sensor->side] % 10 == 0) {

      if (sensor->thrtl_delay < MAX_DELAY) {
        sensor->thrtl_delay += 125; // add an eighth second
      } else {
        sensor->thrtl_delay = MAX_DELAY;
      }
    }

    DEBUG_PRINT("Thread %d thrtl_delay = %d\n", sensor->side,
                sensor->thrtl_delay);
    DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor->side,
                INACT_CNT[sensor->side]);

    // this may be removed
    if (THRTL_SENSOR && INACT_CNT[sensor->side] > 0 &&
        INACT_CNT[sensor->side] % 125 == 0) {
      // try resetting page to home page after a while in here, if they want
      DEBUG_PRINT("Resetting to home page\n");
    }

    usleep((SENSOR_DELAY + sensor->thrtl_delay) * 1000);
  }
}

void parse_JSON(struct sensor_bundle sensor[2], char *JSON) {

  char config_type[20], config_value[15], curr_char;

  int len = strlen(JSON) + 1;
  int j = 0, k = 0, i = 0; // counters and shit
  int side;

  for (i = 0; i < len; i++) {
    curr_char = tolower(JSON[i]);

    if (isalpha(curr_char)) {
      config_type[j++] = curr_char;

    } else if (isdigit(curr_char)) {
      config_value[k++] = curr_char;

    } else if (curr_char == ',' || curr_char == '}') {

      side = strstr(config_type, "right") ? RIGHT : LEFT;

      if (strstr(config_type, "trigger")) {
        sensor[side].trigger = atoi(config_value);

      } else if (strstr(config_type, "echo")) {
        sensor[side].echo = atoi(config_value);

      } else if (strstr(config_type, "delay")) {
        DEBUG_PRINT("DELAY: %d\n", atoi(config_value));
        SENSOR_DELAY = atoi(config_value);

      } else if (strstr(config_type, "throttleSensor")) {
        THRTL_SENSOR = (strstr(config_value, "true") ? 1 : 0);

      } else if (strstr(config_type, "maxDelay")) {
        MAX_DELAY = atoi(config_value);
      }

      // clear the entire array so no extra chars f--k it up
      memset(config_type, 0, sizeof(config_type));
      memset(config_value, 0, sizeof(config_value));

      k = j = 0;
    }
  }
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    error_msg("ERROR: No input arguments.");
  }

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signal_catcher);

  struct sensor_bundle sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js
  parse_JSON(sensor, argv[1]);

  // yeah, this seems dumb, but it’s used within the "sensor_distance" function
  sensor[LEFT].side = LEFT;
  sensor[RIGHT].side = RIGHT;
  sensor[LEFT].thrtl_delay = 0;
  sensor[RIGHT].thrtl_delay = 0;

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].trigger, OUTPUT);
  pinMode(sensor[RIGHT].trigger, OUTPUT);

  pinMode(sensor[LEFT].echo, INPUT);
  pinMode(sensor[RIGHT].echo, INPUT);

  digitalWrite(sensor[LEFT].trigger, LOW);
  digitalWrite(sensor[RIGHT].trigger, LOW);

  pthread_t thread[3];

  pthread_cond_init(&COND, NULL);
  pthread_mutex_init(&MUTEX, NULL);
  pthread_barrier_init(&BARR_TOP, NULL, 2);
  pthread_barrier_init(&BARR_BOT, NULL, 2);

  // this thread reads the global array and prints to stdout
  pthread_create(&thread[2], NULL, (void *)stdout_handler, NULL);

  do {
    // to guarantee the stdout handler thread gets started first
  } while (!STDOUT_THRD_READY);

  // running two threads indefinitely to calculate the distance of each sensor
  pthread_create(&thread[LEFT], NULL, (void *)sensor_distance, &sensor[LEFT]);
  pthread_create(&thread[RIGHT], NULL, (void *)sensor_distance, &sensor[RIGHT]);

  // i mean, realistically, this’ll probably never be reached, but whatever
  int i;

  for (i = 0; i < 3; i++) {
    pthread_join(thread[i], NULL);
  }

  pthread_barrier_destroy(&BARR_TOP);
  pthread_barrier_destroy(&BARR_BOT);

  return 0;
}
