//
// Created by Luca De Mori on 19/12/2018.
//

#ifndef NRG_5_SORBONNE_IOTA8M3_SERIAL_H
#define NRG_5_SORBONNE_IOTA8M3_SERIAL_H

#include "a8m3_cmds.h"
#include <stdint.h>
#include <stddef.h>

#define PORT "/dev/ttyUSB1"
#define BAUDRATE 500000

#define DEBUG(...) printf("debug: " __VA_ARGS__); putchar('\n')
#define INFO(...) printf("info: " __VA_ARGS__); putchar('\n')
#define RECV(...) putchar(M3_RECV); printf("receive"__VA_ARGS__); putchar('\n')

#define INVALID_FD (-1)

typedef int serial_t;

serial_t serial_connect(void);

void print_bytes_str(const uint8_t *addr, size_t addr_len, const char *separator);
int reads(int fd, char* data, unsigned int size);

void debug(const char* msg);
void info(const char* msg);
void error(const char* msg);


#endif //NRG_5_SORBONNE_IOTA8M3_SERIAL_H
