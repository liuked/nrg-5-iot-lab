//
// Created by Luca De Mori on 18/12/2018.
//

#ifndef NRG_5_M3_RIOT_IOTA8M3_H
#define NRG_5_M3_RIOT_IOTA8M3_H

#include <stdbool.h>
#include "a8m3_cmds.h"

#include "kernel_types.h"
#include "net/gnrc.h"
#include "thread.h"


#define START_SEQ_LEN (5)

#define DEBUG(...) putchar(M3_DEBUG); printf("debug: " __VA_ARGS__)
#define INFO(...) putchar(M3_INFO); printf("info: " __VA_ARGS__)
#define ERROR(...) putchar(M3_ERROR); printf("error: " __VA_ARGS__)
#define RECV(...) putchar(M3_RECV); printf("receive"__VA_ARGS__)


/**
 * @brief   Default stack size to use for the IOTA8M3 thread.
 */
#ifndef GNRC_IOTA8M3_STACK_SIZE
#define GNRC_IOTA8M3_STACK_SIZE       (THREAD_STACKSIZE_DEFAULT)
#endif

/**
 * @brief   Default priority for the IOTA8M3 thread.
 */
#ifndef GNRC_IOTA8M3_PRIO
#define GNRC_IOTA8M3_PRIO             (THREAD_PRIORITY_MAIN - 1)
#endif

/**
 * @brief   Default message queue size to use for the IOTA8M3 thread.
 */
#ifndef GNRC_IOTA8M3_MSG_QUEUE_SIZE
#define GNRC_IOTA8M3_MSG_QUEUE_SIZE   (8U)
#endif

#define HDR_LENGTH (9)  // 2 fcf, 1 seq, 2 pan, 2 src, 2 dst

#include "a8m3_cmds.h"

/**
 * @brief   Initialization of the IOTA8M3 thread.
 *
 * @details If IOTA8M3 was already initialized, it will just return the PID of
 *          the IOTA8M3 thread.
 *
 * @return  The PID to the IOTA8M3 thread, on success.
 * @return  -EINVAL, if @ref GNRC_IOTA8M3_PRIO was greater than or equal to
 *          @ref SCHED_PRIO_LEVELS
 * @return  -EOVERFLOW, if there are too many threads running already in general
 */

kernel_pid_t gnrc_iota8m3_init(void);
void sprint_bytes_str(char* str, const uint8_t *addr, size_t addr_len, const char *separator);
void print_bytes_str(const uint8_t *addr, size_t addr_len, const char *separator);
void putbytes(uint8_t *addr, size_t addr_len);

void debug(const char* msg);
void ack(const char* msg);
void error(const char* msg);

kernel_pid_t a8m3_rcv_init(void);


#endif //NRG_5_M3_RIOT_IOTA8M3_H
