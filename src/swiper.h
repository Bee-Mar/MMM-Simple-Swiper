#ifndef SWIPER_H
#define SWIPER_H
#define _DEFAULT_SOURCE // that damn usleep macro requirement

// converting to CPP headers
#include "Sensor.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <ctime>
#include <iostream>
#include <sys/wait.h>
#include <thread>
#include <wiringPi.h>

#define READ 0
#define WRITE 1
#define LEFT 0
#define RIGHT 1
#define NUM_SAMPLES 10

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, args...)                                              \
  fprintf(stderr, "DEBUG_MODE: %s:%d:%s(): " fmt, __FILE__, __LINE__,          \
          __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

void signalCatcher(int sig);

// defined within header since I made this inline
inline void errorMsg(const std::string msg) {
  std::cout << msg << std::endl;
  exit(-1);
}

void busyWaitForStdoutThread(void);

void stdoutHandler(void);

void sensorDistance(Sensor &sensor, boost::barrier &barrier);

float average(float vals[NUM_SAMPLES]);

void parseJSON(Sensor sensor[2], char *JSON);

#endif
