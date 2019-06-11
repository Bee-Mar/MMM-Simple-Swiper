#include "swiper.h"
#include <fstream>

int main(int argc, char *argv[]) {

#ifdef DEBUG
  std::ifstream debugFile("debug_parameters.json");

  std::string debugArgs((std::istreambuf_iterator<char>(debugFile)),
                        (std::istreambuf_iterator<char>()));

  char *debugJSON(const_cast<char *>(debugArgs.c_str()));

#else
  if (argc < 2) { errorMsg("ERROR: No input arguments."); }
#endif

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signalCatcher);

  Sensor sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js
  parseJSON(sensor,
#ifdef DEBUG
            debugJSON
#else
            argv[1]
#endif
  );

  sensor[LEFT].setSide(LEFT);
  sensor[LEFT].setDelay(0);

  sensor[RIGHT].setSide(RIGHT);
  sensor[RIGHT].setDelay(0);

#ifdef DEBUG
  std::cout << "Sensor initialization details:" << std::endl;
  std::cout << "==============================" << std::endl;

#pragma unroll
  for (int i{0}; i < 2; i++) {
    std::cout << "Side = " << (sensor[i].side() == LEFT ? "LEFT" : "RIGHT") << std::endl;
    std::cout << "Echo pin = " << sensor[i].echoPin() << std::endl;
    std::cout << "Trigger pin = " << sensor[i].triggerPin() << std::endl;
    std::cout << "\n" << std::endl;
  }
#endif

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

