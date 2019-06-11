#include "swiper.h"

int main(int argc, char *argv[]) {

#if DEBUG
  std::string debugArgs(
      "{echoLeftPin: 24, triggerLeftPin: 23, echoRightPin: 26, triggerRightPin: 25, threshold: "
      "175, distanceDiff: 1.25, debug: false, delay: 500, sensorThrottling: true, maxDelay: 2000}");

  char *debugJSON = const_cast<char *>(debugArgs.c_str());

#else
  if (argc < 2) { errorMsg("ERROR: No input arguments."); }
#endif

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signalCatcher);

  Sensor sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js
  parseJSON(sensor,
#if DEBUG
            debugJSON
#else
            argv[1]
#endif
  );

  sensor[LEFT].setSide(LEFT);
  sensor[LEFT].setDelay(0);

  sensor[RIGHT].setSide(RIGHT);
  sensor[RIGHT].setDelay(0);

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].triggerPin(), OUTPUT);
  pinMode(sensor[RIGHT].triggerPin(), OUTPUT);

  pinMode(sensor[LEFT].echoPin(), INPUT);
  pinMode(sensor[RIGHT].echoPin(), INPUT);

  digitalWrite(sensor[LEFT].triggerPin(), LOW);
  digitalWrite(sensor[RIGHT].triggerPin(), LOW);

  boost::thread threads[3];

  // this thread reads the global array and prints to stdout
  threads[2] = boost::thread(boost::bind(&stdoutHandler));

  // make sure the stdout thread gets set up first
  busyWaitForStdoutThread();

  boost::barrier barrier(2);

  threads[LEFT] =
      boost::thread(boost::bind(&sensorDistance, boost::ref(sensor[LEFT]), boost::ref(barrier)));

  threads[RIGHT] =
      boost::thread(boost::bind(sensorDistance, boost::ref(sensor[RIGHT]), boost::ref(barrier)));

  // realistically, this’ll never be reached, but whatever
#pragma unroll
  for (int i{0}; i < 3; i++) { threads[i].join(); }

  return 0;
}

