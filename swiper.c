#define _BSD_SOURCE // that damn usleep macro requirement

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

#define READ 0
#define WRITE 1
#define LEFT 0
#define RIGHT 1
#define NUM_SAMPLES 10
#define DEBUG 3
#define CLEAR -10000.0

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, args...)                                              \
  fprintf(stderr, "DEBUG_MODE: %s:%d:%s(): " fmt, __FILE__, __LINE__,          \
          __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

float sensor_output[2] = {CLEAR, CLEAR};

pthread_barrier_t barrier;

int STDOUT_THREAD_READY = 0;

struct sensor_bundle {
  int trigger;
  int echo;
  int signal;
  float delay;
  int side;
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

int compare(const void *a, const void *b) { return (*(int *)a) - (*(int *)b); }

float avg(float vals[NUM_SAMPLES]) {
  int i = 0;
  float sum = 0.0;

  // only uses the first half of the values to extreme outliers
  for (i = 0; i < NUM_SAMPLES / 2; i++)
    sum += vals[i];

  return sum / (NUM_SAMPLES / 2);
}

void stdout_handler() {

  STDOUT_THREAD_READY = 1;

  while (1) {
    if (sensor_output[LEFT] != CLEAR && sensor_output[RIGHT] != CLEAR) {

      printf("%f:%f", sensor_output[0], sensor_output[1]);
      fflush(stdout);

      sensor_output[LEFT] = sensor_output[RIGHT] = CLEAR;
    }
  }
}

void sensor_distance(struct sensor_bundle *sensor) {

  int i = 0; // loop counter

  int delay = sensor->delay; // to prevent re-reading the same struct
  int sensor_side = sensor->side;

  float distance[NUM_SAMPLES];
  long int start = 0, end = 0, elapsed = 0;

  while (1) {

    start = (long int)time(NULL);
    end = (long int)time(NULL);
    elapsed = 0;

    for (i = 0; i < NUM_SAMPLES; i++) {

      digitalWrite(sensor->trigger, HIGH);
      delayMicroseconds(100);
      digitalWrite(sensor->trigger, LOW);

      while (digitalRead(sensor->echo) == LOW) {
        start = micros();
      }

      while (digitalRead(sensor->echo) == HIGH) {
        end = micros();
      }

      elapsed = end - start;
      elapsed = end - start;

      distance[i] = elapsed / 58;
    }

    // sort the values, write average to global array, then nap
    qsort(distance, NUM_SAMPLES, sizeof(float), compare);
    sensor_output[sensor_side] = avg(distance);

    pthread_barrier_wait(&barrier); // sync those threads yo

    usleep(delay * 1000); // in milliseconds
  }
}

void parse_JSON(struct sensor_bundle sensor[2], char *JSON) {
  int len = strlen(JSON) + 1;
  char config_type[20], config_value[15], curr_char;
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
        sensor[side].delay = atoi(config_value);
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

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].trigger, OUTPUT);
  pinMode(sensor[RIGHT].trigger, OUTPUT);

  pinMode(sensor[LEFT].echo, INPUT);
  pinMode(sensor[RIGHT].echo, INPUT);

  digitalWrite(sensor[LEFT].trigger, LOW);
  digitalWrite(sensor[RIGHT].trigger, LOW);

  pthread_t thread[3]; // array of threads
  pthread_barrier_init(&barrier, NULL, 2);

  // this thread reads the global array and prints to stdout
  pthread_create(&thread[2], NULL, (void *)stdout_handler, NULL);

  // to guarantee the stdout handler thread gets started first
  do { /* pass */
  } while (!STDOUT_THREAD_READY);

  // running two threads indefinitely to calculate the distance of each sensor
  pthread_create(&thread[0], NULL, (void *)sensor_distance, &sensor[0]);
  pthread_create(&thread[1], NULL, (void *)sensor_distance, &sensor[1]);

  int i;

  for (i = 0; i < 3; i++) {
    pthread_join(thread[i], NULL);
  }

  pthread_barrier_destroy(&barrier);

  return 0;
}
