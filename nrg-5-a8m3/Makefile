CC=gcc
CFLAGS=-I.
DEPS = iota8m3_serial.h
OBJ = iota8m3_serial.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

nrg-5-a8m3: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

all: clean nrg-5-a8m3

.PHONY: clean

clean:
	rm -f ./*.o nrg-5-a8m3 
