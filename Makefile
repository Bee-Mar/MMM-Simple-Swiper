LDFLAGS=-lpthread -lwiringPi
CFLAGS=-g -o
CC=gcc

swiper: swiper.c
	@if [ ! -f /usr/local/include/wiringPi.h ]; then\
		printf "wiringPi dependency not met. Please run 'make install'\n";\
	else\
		$(CC) -o swiper swiper.c $(LDFLAGS);\
	fi

debug: swiper.c
	@if [ ! -f /usr/local/include/wiringPi.h ]; then\
		printf "\nwiringPi dependency not met. Please run 'make install'\n";\
	else\
		$(CC) $(CFLAGS) swiper swiper.c $(LDFLAGS) -DEBUG;\
	fi

install: swiper.c
	@if [ ! -f /usr/local/include/wiringPi.h ]; then\
		printf "\n==========================\n";\
		printf "Getting WiringPi Dependency\n";\
		printf "===========================\n\n";\
		git clone git://git.drogon.net/wiringPi;\
		cd wiringPi && ./build;\
		printf "\n===============================\n";\
		printf "Installing NodeJS Dependencies\n";\
		printf "==============================\n\n";\
		npm install; \
		printf "\n\nFinished installation\n\n";\
	else\
		printf "\nwiringPi library already exists in /usr/local/include/\n";\
		printf "\nRunning npm install...\n";\
		npm install;\
	fi

clean:
	rm -rf swiper swiper.o
