#include <ctype.h>
#include <math.h>
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

#define LEFT 0
#define RIGHT 1
#define NUM_SAMPLES 10

#define NUM_SECONDS 2.0
#define DEBUG 3

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, args...)                                              \
  fprintf(stderr, "DEBUG_MODE: %s:%d:%s(): " fmt, __FILE__, __LINE__,          \
          __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

struct sensor_bundle {
  float avg_dist;
  int trigger;
  int echo;
  int signal;
};

void signal_catcher(int sig) {
  // catch the signal and clean up
  printf("SIGINT caught. Shutting down swiper MMM-simple-swiper.\n");
  wait(0); // wait for cleanup
  exit(0); // the exit
}

void error_msg(char *msg) {
  printf("%s\n", msg);
  exit(-1);
}

int compare(const void *a, const void *b) { return (*(int *)a) - (*(int *)b); }

float avg(float vals[NUM_SAMPLES]) {
  int i = 0;
  float sum = 0.0, avg = 0.0;

  // only uses the first half of the values to extreme outliers
  for (i = 0; i < NUM_SAMPLES / 2; i++)
    sum += vals[i];

  return sum / (NUM_SAMPLES / 2);
}

void *sensor_distance(struct sensor_bundle *sensor) {

  int i = 0;
  float distance[NUM_SAMPLES];
  long int start = (long int)time(NULL), end = (long int)time(NULL);
  long int elapsed = 0;

  for (i = 0; i < NUM_SAMPLES; i++) {

    digitalWrite(sensor->trigger, HIGH);
    delayMicroseconds(100);
    digitalWrite(sensor->trigger, LOW);

    while (digitalRead(sensor->echo) == LOW)
      start = micros();

    while (digitalRead(sensor->echo) == HIGH)
      end = micros();

    elapsed = end - start;
    elapsed = end - start;

    distance[i] = elapsed / 58;
  }

  // sort the values, then get their average
  qsort(distance, NUM_SAMPLES, sizeof(float), compare);
  sensor->avg_dist = avg(distance);
}

void parse_JSON(struct sensor_bundle sensor[2], char *JSON, int *delay) {
  int len = strlen(JSON) + 1;
  char config_type[20], config_value[15];
  int j = 0, k = 0, i = 0; // counters
  int type, side;          // to determine the sensor and pin type
  char curr_char;

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
        *delay = atoi(config_value);
      }

      // clear the entire array so no extra chars mess it up
      memset(config_type, 0, sizeof(config_type));
      memset(config_value, 0, sizeof(config_value));
      k = j = 0;
    }
  }
}

int main(int argc, char *argv[]) {

  if (argc < 2)
    error_msg("ERROR: No input arguments.");

  // installing signal catching for Ctr-C events
  signal(SIGINT, signal_catcher);

  struct sensor_bundle sensor[2];
  int delay = 1000; // if nothing gets defined

  // read and parse the config passed over from MMM-simple-swiper.js

  parse_JSON(sensor, argv[1], &delay);

  int i = 0;
  pthread_t thread[2];

  wiringPiSetupGpio();

  pinMode(sensor[LEFT].trigger, OUTPUT);
  pinMode(sensor[RIGHT].trigger, OUTPUT);

  pinMode(sensor[LEFT].echo, INPUT);
  pinMode(sensor[RIGHT].echo, INPUT);

  digitalWrite(sensor[LEFT].trigger, LOW);
  digitalWrite(sensor[RIGHT].trigger, LOW);

  while (1) {

    // running two threads to calculate the distance of each sensor
    for (i = 0; i < 2; i++)
      pthread_create(&thread[i], NULL, (void *)sensor_distance, &sensor[i]);

    for (i = 0; i < 2; i++)
      pthread_join(thread[i], NULL);

    // flush stdout, and let the node_helper resolve motion
    printf("%f:%f\n", sensor[LEFT].avg_dist, sensor[RIGHT].avg_dist);
    fflush(stdout);

    // converted to milliseconds
    usleep(delay * 1000);
  }

  return 0;
}
