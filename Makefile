LDFLAGS=-lpthread -lwiringPi
CFLAGS=-g -o
CC=gcc

swiper: swiper.c
		$(CC) -o swiper swiper.c $(LDFLAGS);\

debug: swiper.c
		$(CC) $(CFLAGS) swiper swiper.c $(LDFLAGS) -DEBUG;

clean:
	rm -rf swiper swiper.o
