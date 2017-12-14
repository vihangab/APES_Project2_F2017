CC=gcc
#arm-linux-gnueabihf-gcc
CFLAGS= -g -O0 -pthread
LFLAGS= -lm -lrt

SSRCS= main.c gpio.c socketThread.c loggerThread.c logger.c initialise_queue.c decisionThread.c timer.c
#headers=i2c_lib.h apds9301.h

PROGRAMS=p2 test0 test1
TEST0=mockServer.c
TEST1=mockClient.c
LOG_FILE=dataLog
#Default make command builds executable file
all: $(PROGRAMS)

p2: $(SSRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

test0: $(TEST0)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

test1: $(TEST1)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

.PHONY: clean

clean:
	rm -rf $(PROGRAMS) $(LOG_FILE)
