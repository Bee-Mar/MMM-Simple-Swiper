#include "swiper.h"

int main(int argc, char *argv[]) {

  using std::cout;
  using std::endl;

#ifdef DEBUG
  std::ifstream debugFile("debug_parameters.json");

  std::string debugArgs((std::istreambuf_iterator<char>(debugFile)),
                        (std::istreambuf_iterator<char>()));

  char *debugJSON(const_cast<char *>(debugArgs.c_str()));

#else
  if (argc < 2) {
    cout << "ERROR: No input arguments." << endl;
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
  cout << "Sensor initialization details:" << endl;
  cout << "==============================" << endl;

  for (int i{0}; i < 2; i++) {
    cout << "Side = " << (sensor[i].side() == LEFT ? "LEFT" : "RIGHT") << endl;
    cout << "Echo Pin = " << sensor[i].echoPin() << endl;
    cout << "Trigger Pin = " << sensor[i].triggerPin() << endl;
    cout << "\n" << endl;
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

  using boost::ref;
  using boost::thread;

  std::array<thread, 2> threads;

  threads[0] = thread(bind(&sensorDistance, ref(sensor[LEFT])));
  threads[1] = thread(bind(sensorDistance, ref(sensor[RIGHT])));

  // will never get to this point anyway
  threads[LEFT].join();
  threads[RIGHT].join();

  return 0;
}
