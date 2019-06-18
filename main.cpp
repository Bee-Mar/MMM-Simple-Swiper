#include "swiper.h"

int main(int argc, char *argv[]) {

#ifdef DEBUG
  std::ifstream debugFile("debug_parameters.json");

  std::string debugArgs((std::istreambuf_iterator<char>(debugFile)),
                        (std::istreambuf_iterator<char>()));

  char *debugJSON(const_cast<char *>(debugArgs.c_str()));

#else
  if (argc < 2) {
    std::cout << "ERROR: No input arguments." << std::endl;
    exit(-1);
  }
#endif

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signalCatcher);

  Sensor sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js, or debug_parameters.json
  parseJSON(sensor,
#ifdef DEBUG
            debugJSON
#else
            argv[1]
#endif
  );

#ifdef DEBUG
  std::cout << "Sensor initialization details:" << std::endl;
  std::cout << "==============================" << std::endl;

#pragma unroll(2)
  for (int i{0}; i < 2; i++) {
    std::cout << "Side = " << (sensor[i].side() == LEFT ? "LEFT" : "RIGHT") << std::endl;
    std::cout << "Echo Pin = " << sensor[i].echoPin() << std::endl;
    std::cout << "Trigger Pin = " << sensor[i].triggerPin() << std::endl;
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

  std::array<boost::thread, 2> threads;

  threads[0] = boost::thread(boost::bind(&sensorDistance, boost::ref(sensor[LEFT])));
  threads[1] = boost::thread(boost::bind(sensorDistance, boost::ref(sensor[RIGHT])));

  // will never get to this point anyway
  threads[LEFT].join();
  threads[RIGHT].join();

  return 0;
}
