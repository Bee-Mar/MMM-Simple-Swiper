#include "swiper.h"

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

  // read and parse the config passed over from MMM-simple-swiper.js, or debug_parameters.json
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
  {
    using namespace std;

    cout << "Sensor initialization details:" << endl;
    cout << "==============================" << endl;

#pragma unroll
    for (int i{0}; i < 2; i++) {
      cout << "Side = " << (sensor[i].side() == LEFT ? "LEFT" : "RIGHT") << endl;
      cout << "Echo Pin = " << sensor[i].echoPin() << endl;
      cout << "Trigger Pin = " << sensor[i].triggerPin() << endl;
      cout << "\n" << endl;
    }
  }
#endif

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].triggerPin(), OUTPUT);
  pinMode(sensor[LEFT].echoPin(), INPUT);
  digitalWrite(sensor[LEFT].triggerPin(), LOW);

  pinMode(sensor[RIGHT].triggerPin(), OUTPUT);
  pinMode(sensor[RIGHT].echoPin(), INPUT);
  digitalWrite(sensor[RIGHT].triggerPin(), LOW);

  boost::thread threads[3];

  threads[2] = boost::thread(boost::bind(&stdoutHandler));
  busyWaitForStdoutThread(); // make sure the stdout thread gets set up first

  boost::barrier barrier(2);

  {
    using namespace boost; // just to prevent having to type out boost:: for everything here

    threads[0] = thread(bind(&sensorDistance, ref(sensor[LEFT]), ref(barrier)));
    threads[1] = thread(bind(sensorDistance, ref(sensor[RIGHT]), ref(barrier)));
  }

  // realistically, this’ll never be reached, but whatever
#pragma unroll
  for (int i{0}; i < 3; i++) { threads[i].join(); }

  return 0;
}

