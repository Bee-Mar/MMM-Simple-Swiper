LDFLAGS = -lwiringPi -lm -lrt -lcrypt -lboost_system -lboost_thread -lpthread
OBJS = main.o swiper.o Sensor.o
CFLAGS = -o
SUB_CFLAGS = -c
CC = g++
VERSION = -std=c++11
CC += $(VERSION)

all: build_depends mmm_simple_swiper

# modify the compile flags then, build if using debug
debug: CFLAGS = -g -o
debug: SUB_CFLAGS = -g -c
debug: LDFLAGS += -DDEBUG

debug: build_depends mmm_simple_swiper

build_depends:
	@if [ ! -f /usr/local/include/wiringPi.h ]; then\
		printf "wiringPi dependency not met. Collecting and building wiringPi...\n";\
		git clone git://git.drogon.net/wiringPi;\
		cd wiringPi && ./build;\
	fi
	@if [ ! -d node_modules ]; then\
		printf "NPM dependencies do not appear to be installed. Building NPM modules...\n";\
		npm install; \
	fi

mmm_simple_swiper: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) mmm_simple_swiper $(LDFLAGS)

Sensor.o: Sensor.cpp
	$(CC) $(SUB_CFLAGS) Sensor.cpp $(LDFLAGS)

swiper.o: swiper.cpp
	$(CC) $(SUB_CFLAGS) swiper.cpp $(LDFLAGS)

main.o: main.cpp
	$(CC) $(SUB_CFLAGS) main.cpp $(LDFLAGS)

clean:
	rm -rf mmm_simple_swiper $(OBJS)
