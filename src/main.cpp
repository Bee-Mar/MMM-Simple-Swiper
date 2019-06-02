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

  boost::thread threads[3];

  // NEED TO TRANSLATE THIS TO C++

  // pthread_cond_init(&COND, NULL);
  // pthread_mutex_init(&MUTEX, NULL);

  // this thread reads the global array and prints to stdout
  threads[2] = boost::thread(boost::bind(&stdoutHandler));

  busyWaitForStdoutThread();

  boost::barrier barrier(2);

  threads[LEFT] = boost::thread(boost::bind(
      &sensorDistance, boost::ref(sensor[LEFT]), boost::ref(barrier)));

  threads[RIGHT] = boost::thread(boost::bind(
      sensorDistance, boost::ref(sensor[RIGHT]), boost::ref(barrier)));

  // i mean, realistically, this’ll never be reached, but whatever
  for (int i = 0; i < 3; i++) {
    threads[i].join();
  }

  return 0;
}

