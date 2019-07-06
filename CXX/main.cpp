#include "swiper.h"

int main(int argc, char *argv[]) {

  using std::cout;
  using std::endl;

#ifdef MMM_SIMPLE_SWIPER_DEBUG
  // the executable will be placed in the 'build' directory, so the
  // debug_parameters.json file will located one level down
  std::ifstream debug_file("../debug_parameters.json");

  std::string debug_args((std::istreambuf_iterator<char>(debug_file)),
                         (std::istreambuf_iterator<char>()));

  char *debug_JSON(const_cast<char *>(debug_args.c_str()));

#else
  if (argc < 2) {
    cout << "ERROR: No input arguments." << endl;
    exit(-1);
  }
#endif

  // snatch those gnarly keyboard interrupts
  signal(SIGINT, signal_catcher);

  Sensor sensor[2];

  // read and parse the config passed over from MMM-simple-swiper.js, or debug_parameters.json
  parse_JSON(sensor,
#ifdef MMM_SIMPLE_SWIPER_DEBUG
             debug_JSON
#else
             argv[1]
#endif
  );

#ifdef MMM_SIMPLE_SWIPER_DEBUG
  cout << "Sensor initialization details:" << endl;
  cout << "==============================" << endl;

  for (int i{0}; i < 2; i++) {
    cout << "Side = " << (sensor[i].side() == LEFT ? "LEFT" : "RIGHT") << endl;
    cout << "Echo Pin = " << sensor[i].echo_pin() << endl;
    cout << "Trigger Pin = " << sensor[i].trigger_pin() << endl;
    cout << "\n" << endl;
  }

#endif

  // setting up the pins and stuff
  wiringPiSetupGpio();

  pinMode(sensor[LEFT].trigger_pin(), OUTPUT);
  pinMode(sensor[RIGHT].trigger_pin(), OUTPUT);

  pinMode(sensor[LEFT].echo_pin(), INPUT);
  pinMode(sensor[RIGHT].echo_pin(), INPUT);

  digitalWrite(sensor[LEFT].trigger_pin(), LOW);
  digitalWrite(sensor[RIGHT].trigger_pin(), LOW);

  using boost::ref;
  using boost::thread;

  std::array<thread, 2> threads{thread(bind(&calculate_sensor_distance, ref(sensor[LEFT]))),
                                thread(bind(calculate_sensor_distance, ref(sensor[RIGHT])))};

  // will never get to this point anyway
  threads[LEFT].join();
  threads[RIGHT].join();

  return 0;
}
