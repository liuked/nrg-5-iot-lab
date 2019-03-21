//
// Created by Luca De Mori on 19/12/2018.
//

#ifndef NRG_5_SORBONNE_IOTA8M3_SERIAL_H
#define NRG_5_SORBONNE_IOTA8M3_SERIAL_H

#include "a8m3_cmds.h"
#include <stdint.h>
#include <stddef.h>

#define PORT "/dev/ttyA8_M3"
#define BAUDRATE 500000
#define MAX_ATTEMPTS 5
#define MAX_LINE 100
#define HOSTNAME_FILE "/etc/hostname"
#define MAX_SER_MSG_LEN sizeof(msg_type_t)+START_SEQ_LEN+MAX_DATA_LEN
#define MAX_TIMEOUTS 5


#define DEBUG(...) printf("debug: " __VA_ARGS__); putchar('\n')
#define INFO(...) printf("info: " __VA_ARGS__); putchar('\n')
#define ERROR(...) printf("error: " __VA_ARGS__); putchar('\n')
#define RECV(...) putchar(M3_RECV); printf("receive"__VA_ARGS__); putchar('\n')

#define INVALID_FD (-1)

typedef int serial_t;
typedef uint8_t* serial_buf_t;

serial_t serial_connect(void);

void print_bytes_str(const uint8_t *addr, size_t addr_len, const char *separator);
void sprint_bytes_str(char* str, const uint8_t *addr, size_t addr_len, const char *separator);

int serial_send(serial_t sfd, serial_buf_t buf, size_t buflen, msg_type_t msg_type, const uint8_t *data, const size_t len);
size_t serial_recv(serial_t sfd, serial_buf_t buf, size_t buflen, msg_type_t *rcv_msg_type);
int __m3_reads(int fd, char *data, unsigned int size);

void debug(const char* msg);
void info(const char* msg);
void error(const char* msg);


#endif //NRG_5_SORBONNE_IOTA8M3_SERIAL_H
