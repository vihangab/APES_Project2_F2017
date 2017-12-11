CC=gcc
#arm-linux-gnueabihf-gcc
CFLAGS= -g -O0 -pthread
LFLAGS= -lm -lrt

SSRCS= main.c gpio.c socketThread.c loggerThread.c logger.c initialise_queue.c
#headers=i2c_lib.h apds9301.h

PROGRAMS=p2

#Default make command builds executable file
all: $(PROGRAMS)

p2: $(SSRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

.PHONY: clean

clean:
	rm -rf $(PROGRAMS)
