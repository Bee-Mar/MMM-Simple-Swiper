LDFLAGS=-lpthread -lwiringPi
CFLAGS=-g -o
DEPENDS=main.c required_lib
CC=gcc

main: $(DEPENDS)
	$(CC) -o main main.c $(LDFLAGS)

debug: $(DEPENDS)
	$(CC) $(CFLAGS) main main.c $(LDFLAGS) -DEBUG

required_lib: main.c
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
	fi

clean:
	rm -rf main main.o
