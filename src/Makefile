LDFLAGS = -lpthread -lwiringPi -lm -lrt -lcrypt
OBJS = main.o swiper.o Sensor.o
CFLAGS = -o
CC = g++

all: build_depends main

debug: CFLAGS = -g -o
debug: LDFLAGS += -DEBUG
debug: build_depends main

build_depends:
	@if [ ! -f /usr/local/include/wiringPi.h ]; then\
		printf "wiringPi dependency not met. Collecting and building wiringPi...\n";\
		git clone git://git.drogon.net/wiringPi;\
		cd wiringPi && ./build;\
	fi
	@if [ ! -d ../node_modules ]; then\
		printf "NPM dependencies do not appear to be installed. Building NPM modules...\n";\
		npm install; \
	fi

main: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) main $(LDFLAGS)

Sensor.o: Sensor.cpp
	$(CC) -c Sensor.cpp $(LDFLAGS)

swiper.o: swiper.cpp
	$(CC) -c swiper.cpp $(LDFLAGS)

main.o: main.cpp
	$(CC) -c main.cpp $(LDFLAGS)

clean:
	rm -rf main $(OBJS)
