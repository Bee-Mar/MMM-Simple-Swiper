#include "swiper.h"
#include "Sensor.h"

// used for synchronization between each of the threads
boost::asio::io_service service;
boost::asio::io_service::work work(service);
boost::barrier thread_barrier(2);

// inactive count/tracker for each sensor, and the recorded distances for each sensor
std::array<int, 2> INACT_CNT{0, 0};
std::array<float, 2> SENSOR_OUTPUT{-10'000.0, -10'000.0};

int THRTL_DELAY{0};
int SENSOR_DELAY{1'250};
int MAX_DELAY{4'000};

// inline functions in header file: substringExists & errorMessage

auto signal_catcher(int sig) -> void {
    // catch the signal and clean up
    std::cout << "Shutting down swiper MMM-simple-swiper." << std::endl;
    wait(0);
    exit(0);
}

auto average(std::array<float, NUM_SAMPLES> vals) -> float {
    float sum{0};

    // only use the first half of the values to trim extreme outliers
    for (int i{0}; i < HALF_NUM_SAMPLES; i++) { sum += vals[i]; }

    return (sum / HALF_NUM_SAMPLES);
}

auto stdout_handler() -> void {
    std::cout << SENSOR_OUTPUT[LEFT] << ":" << SENSOR_OUTPUT[RIGHT] << std::endl;
}

auto calculate_sensor_distance(Sensor &sensor) -> void {
    std::array<float, NUM_SAMPLES> distance;

    long int start{0}, end{0}, elapsed{0};

    float prev_dist{-1'000.0}, curr_dist{0};

    while (true) {
        start = end = static_cast<long int>(time(NULL));
        elapsed = 0;

        for (int i{0}; i < NUM_SAMPLES; i++) {
            digitalWrite(sensor.trigger_pin(), HIGH);

            delayMicroseconds(100);

            digitalWrite(sensor.trigger_pin(), LOW);

            // do nothing until the read changes
            while (digitalRead(sensor.echo_pin()) == LOW) {}

            start = micros();

            while (digitalRead(sensor.echo_pin()) == HIGH) {}

            end = micros();

            elapsed = (end - start);
            distance.at(i) = (elapsed / 58);
        }

        // sort the values, then write average to global array
        std::sort(distance.begin(), distance.end());
        curr_dist = average(distance);

        // write the output value to the global array for each thread
        SENSOR_OUTPUT[sensor.side()] = curr_dist;

        thread_barrier.wait();

        if (std::fabs(curr_dist - prev_dist) > 20.0) {
            if (sensor.side() == RIGHT) { // print the contents of the global array to stdout
                service.post(boost::bind(&stdout_handler));
                service.run_one();
            }

            INACT_CNT[sensor.side()] = 0;
            sensor.set_delay(0);

        } else {
            INACT_CNT[sensor.side()]++;

            if (sensor.side() == RIGHT) {
                // take the maximum of the recorded delays and assign it to both sensors
                INACT_CNT[LEFT] = INACT_CNT[RIGHT] = std::max(INACT_CNT[LEFT], INACT_CNT[RIGHT]);
            }
        }

        prev_dist = curr_dist;
        thread_barrier.wait();

        // short circuit the if statement if the sensor throttle is set to false
        if (sensor.throttle() && INACT_CNT[sensor.side()] > 0 &&
            (INACT_CNT[sensor.side()] % 10 == 0)) {
            // add an eighth of a second if we haven't hit MAX_DELAY
            sensor.set_delay((sensor.delay() < MAX_DELAY) ? (sensor.delay() + 125) : (MAX_DELAY));
        }

        DEBUG_PRINT("Thread %d throttleDelay = %d\n", sensor.side(), sensor.delay());
        DEBUG_PRINT("Thread %d INACT_CNT = %d\n", sensor.side(), INACT_CNT[sensor.side()]);

        // this may be removed entirely
        // if (THRTL_SENSOR && (INACT_CNT[sensor.side] > 0) && (INACT_CNT[sensor.side] % 125 == 0))
        // {
        //   // try resetting page to home page after a while in here, if they want
        //   DEBUG_PRINT("Resetting to home page\n");
        // }

        usleep((SENSOR_DELAY + sensor.delay()) * 1'000);
    }
}

auto parse_JSON(Sensor sensor[2], char *JSON) -> void {
    std::string config_type, config_value;

    const int len(strlen(JSON) + 1);
    int side{0};

    sensor[LEFT].set_side(LEFT);
    sensor[RIGHT].set_side(RIGHT);

    for (int i{0}; i < len; i++) {
        const char current_char(tolower(JSON[i]));

        if (isalpha(current_char)) {
            config_type.push_back(current_char);

        } else if (isdigit(current_char)) {
            config_value.push_back(current_char);

        } else if (current_char == ',' || current_char == '}') {
            side = (substringExists(config_type.find("right")) ? RIGHT : LEFT);

            if (substringExists(config_type.find("trigger"))) {
                sensor[side].set_trigger_pin(std::stoi(config_value));

            } else if (substringExists(config_type.find("echo"))) {
                sensor[side].set_echo_pin(std::stoi(config_value));

            } else if (substringExists(config_type.find("delay"))) {
                SENSOR_DELAY = std::stoi(config_value);

            } else if (substringExists(config_type.find("throttleSensor"))) {
                sensor[side].set_throttle_sensor(substringExists(config_type.find("true")));

            } else if (substringExists(config_type.find("maxDelay"))) {
                MAX_DELAY = std::stoi(config_value);
            }

            config_type.clear();
            config_value.clear();
        }
    }
}
