#ifndef MMM_SIMPLE_SWIPER_SWIPER_H
#define MMM_SIMPLE_SWIPER_SWIPER_H
#define _DEFAULT_SOURCE // usleep macro requirement

// converting to CPP headers
#include "Sensor.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <cmath>
#include <csignal>
#include <ctime>
#include <iostream>
#include <sys/wait.h>
#include <wiringPi.h>

constexpr int READ = 0;
constexpr int WRITE = 1;
constexpr int LEFT = 0;
constexpr int RIGHT = 1;
constexpr int NUM_SAMPLES = 10;
constexpr int HALF_NUM_SAMPLES = (NUM_SAMPLES / 2);

#if defined(MMM_SIMPLE_SWIPER_DEBUG)
#include <fstream>

#define DEBUG_PRINT(fmt, args...)                                                                  \
    fprintf(stderr, "DEBUG MODE: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

auto signal_catcher(int sig) -> void;

auto inline substringExists(std::size_t size) -> bool {
    return (size != std::string::npos);
}

auto inline errorMessage(const std::string msg) -> void {
    std::cout << msg << std::endl;
    exit(-1);
}

auto stdout_handler() -> void;

auto calculate_sensor_distance(Sensor &sensor) -> void;

auto average(float vals[NUM_SAMPLES]) -> float;

auto parse_JSON(Sensor sensor[2], char *JSON) -> void;

#endif
