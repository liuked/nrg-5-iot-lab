CC=gcc
CFLAGS=-I.
DEPS = iota8m3_serial.h iotm3_ops.h
OBJ = iotm3_ops.o iota8m3_serial.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

nrg-5-a8m3: $(OBJ)
	$(CC) -pthread -o $@ $^ $(CFLAGS)

all: clean nrg-5-a8m3

.PHONY: clean

clean:
	rm -f ./*.o nrg-5-a8m3 
