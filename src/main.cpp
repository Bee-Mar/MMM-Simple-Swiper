#include "swiper.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    errorMsg("ERROR: No input arguments.");
  }

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signalCatcher);

  Sensor sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js
  parseJSON(sensor, argv[1]);

  sensor[LEFT].side = LEFT;
  sensor[RIGHT].side = RIGHT;
  sensor[LEFT].throttleDelay = sensor[RIGHT].throttleDelay = 0;

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].trigger, OUTPUT);
  pinMode(sensor[RIGHT].trigger, OUTPUT);

  pinMode(sensor[LEFT].echo, INPUT);
  pinMode(sensor[RIGHT].echo, INPUT);

  digitalWrite(sensor[LEFT].trigger, LOW);
  digitalWrite(sensor[RIGHT].trigger, LOW);

  std::thread threads[3];

  // NEED TO TRANSLATE THIS TO C++

  // pthread_cond_init(&COND, NULL);
  // pthread_mutex_init(&MUTEX, NULL);
  // pthread_barrier_init(&BARR_TOP, NULL, 2);
  // pthread_barrier_init(&BARR_BOT, NULL, 2);

  // this thread reads the global array and prints to stdout
  // pthread_create(&thread[2], NULL, (void *)stdoutHandler, NULL);

  threads[2] = std::thread(stdoutHandler);

  busyWaitForStdoutThread();

  threads[LEFT] = std::thread(sensorDistance, std::ref(sensor[LEFT]));
  threads[RIGHT] = std::thread(sensorDistance, std::ref(sensor[RIGHT]));

  // i mean, realistically, this’ll never be reached, but whatever

  for (int i = 0; i < 3; i++) {
    threads[i].join();
  }

  // NEED TO TRANSLATE THIS TO C++

  // pthread_barrier_destroy(&BARR_TOP);
  // pthread_barrier_destroy(&BARR_BOT);

  return 0;
}

