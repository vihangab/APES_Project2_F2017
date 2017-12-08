CC= arm-linux-gnueabihf-gcc
CFLAGS= -g -O0 -pthread
LFLAGS= -lm -lrt

SSRCS= main.c gpio.c
#headers=i2c_lib.h apds9301.h

PROGRAMS=test

#Default make command builds executable file
all: $(PROGRAMS)

test: $(SSRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

.PHONY: clean

clean:
	rm -rf $(PROGRAMS)
