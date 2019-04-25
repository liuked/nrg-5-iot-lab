//
// Created by Luca De Mori on 19/12/2018.
//

#include "iota8m3_serial.h"

#include <stdio.h>                       /* Standard Includes     */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <termios.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "linux/serial.h"

serial_t serial_connect(void) {

    struct termios newtio;
    struct serial_struct ser_info;

    LOG_DEBUG("serial_connect: opening serial port...\n");

    serial_t serial;
    serial = open(PORT, O_RDWR );               /* Try user input depending on port */

    if (serial < 0) {                                   /* cannot open     */
        error("serial_connect: error opening serial...\n");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag |= CS8;


    newtio.c_cflag |= CREAD|HUPCL|CLOCAL|CSTOPB;
    newtio.c_iflag &= ~(IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
    newtio.c_iflag |= BRKINT;
    newtio.c_lflag &= ~(ICANON|ECHO|ISTRIP);
    newtio.c_oflag  = 0;
    newtio.c_lflag  = 0;

//    VMIN = 0 and VTIME = 0
//    This is a completely non-blocking read - the call is satisfied immediately directly from the driver's input queue. If data are available, it's transferred to the caller's buffer up to nbytes and returned. Otherwise zero is immediately returned to indicate "no data". We'll note that this is "polling" of the serial port, and it's almost always a bad idea. If done repeatedly, it can consume enormous amounts of processor time and is highly inefficient. Don't use this mode unless you really, really know what you're doing.
//    VMIN = 0 and VTIME > 0
//    This is a pure timed read. If data are available in the input queue, it's transferred to the caller's buffer up to a maximum of nbytes, and returned immediately to the caller. Otherwise the driver blocks until data arrives, or when VTIME tenths expire from the start of the call. If the timer expires without data, zero is returned. A single byte is sufficient to satisfy this read call, but if more is available in the input queue, it's returned to the caller. Note that this is an overall timer, not an intercharacter one.
//    VMIN > 0 and VTIME > 0
//    A read() is satisfied when either VMIN characters have been transferred to the caller's buffer, or when VTIME tenths expire between characters. Since this timer is not started until the first character arrives, this call can block indefinitely if the serial line is idle. This is the most common mode of operation, and we consider VTIME to be an intercharacter timeout, not an overall one. This call should never return zero bytes read.
//    VMIN > 0 and VTIME = 0
//    This is a counted read that is satisfied only when at least VMIN characters have been transferred to the caller's buffer - there is no timing component involved. This read can be satisfied from the driver's input queue (where the call could return immediately), or by waiting for new data to arrive: in this respect the call could block indefinitely. We believe that it's undefined behavior if nbytes is less then VMIN.

    newtio.c_cc[VMIN]  = 0; // 5 sec timeout non-blocking serial
    newtio.c_cc[VTIME] = 50;

    if (tcflush(serial, TCIFLUSH) < 0) {             /* Flush the serial port            */
        close(serial);
        return -1;
    }

    LOG_DEBUG("serial_connect: set speed %d...\n", BAUDRATE);
    cfsetospeed (&newtio, BAUDRATE);
    cfsetispeed (&newtio, BAUDRATE);

    LOG_DEBUG("serial_connect: write parameters...\n");
    if (tcsetattr(serial, TCSANOW, &newtio) < 0) {   /* Set the parameters                */
        close(serial);
        return -1;
    }


//    if (ioctl(serial, TIOCGSERIAL, &ser_info) < 0) {
//        perror ("TIOCGSERIAL");
//        return -1;
//    }
//    //ser_info.flags = ASYNC_SPD_CUST | ASYNC_LOW_LATENCY;
//    //ser_info.custom_divisor = ser_info.baud_base / BAUDRATE;
//    ser_info.flags = (ser_info.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
//    ser_info.custom_divisor = (ser_info.baud_base + (BAUDRATE / 2)) / BAUDRATE;
//    float closestSpeed = ser_info.baud_base / ser_info.custom_divisor;
//    if (closestSpeed < BAUDRATE * 98 / 100 || closestSpeed > BAUDRATE * 102 / 100) {
//        LOG_INFO("Cannot set serial port speed to %d. Closest possible is %d\n", BAUDRATE, closestSpeed);
//    }
//
//    if (ioctl(serial, TIOCSSERIAL, &ser_info) < 0) {
//        perror ("TIOCSSERIAL");
//        return -1;
//    }

    return serial;

}

int serial_close(serial_t sfd)
{
    if (sfd) {
        if (close(sfd)) {
            LOG_ERROR("error closing serial %d", sfd);
            return -1;
        }
        return 0;
    }
    LOG_DEBUG("null pointer");
    return -1;
}


int __m3_reads(int fd, char *data, unsigned int size)
{
    int n=0;
    int i=0;
    int end_c=0;
    char stop_seq[STOP_SEQ_LEN];
    strcpy(stop_seq, STOP_SEQ);

    memset(data, 0, size);

    do {
        n = read (fd, data+i, 1);
        if (!n) {
            return i;
        }
        if (data[i]==stop_seq[end_c]) end_c++; else end_c=0;
        i++;
    }
    while (end_c<STOP_SEQ_LEN && i<size);
    //remove end caracters from string
    memset(data+i-STOP_SEQ_LEN, 0, STOP_SEQ_LEN);

    return i-STOP_SEQ_LEN;
}

void print_bytes_str(const uint8_t *addr, size_t addr_len, const char *separator)
{
    for (size_t i = 0; i < addr_len; i++) {
        if (i != 0 && separator) {
            printf("%s", separator);
        }
        printf("%02X", (unsigned)addr[i]);
    }
}

void sprint_bytes_str(char* str, const uint8_t *addr, size_t addr_len, const char *separator)
{
    char *p;
    p=str;
    for (size_t i = 0; i < addr_len; i++) {
        if (i != 0 && separator) {
            sprintf(p, "%s", separator);
            p+=(strlen(separator));
        }
        sprintf(p, "%02X", (unsigned)addr[i]);
        p+=2;
    }
}


size_t serial_recv(serial_t sfd, serial_buf_t buf, size_t buflen, msg_type_t *rcv_msg_type)
{
    size_t n;
    n = (size_t)__m3_reads(sfd, buf, (buflen < MAX_SER_MSG_LEN) ? buflen : MAX_SER_MSG_LEN);
    if (!n) {
        LOG_DEBUG("serial_recv: serial timeout\n");
        return 0;
    }
    *rcv_msg_type = buf[0];
    switch (*rcv_msg_type) {
        case M3_ACK:
            LOG_INFO("---> [M3 ACK] [%d]: %s\n", n, buf);
            break;
        case M3_ERR:
            LOG_INFO("---> [M3 ERR][%d]: %s\n", n, buf);
            break;
        case M3_RECV:
            LOG_INFO("---> [M3 RECV][%d]: %s\n", n, buf);
            break;
        default:
            LOG_INFO("---> [M3 DEBUG][%d]: %s\n", n, buf);
            break;
    }

    return n;

}



int serial_send(serial_t sfd, serial_buf_t buf, size_t buflen, msg_type_t msg_type, const uint8_t *data, const size_t len)
{
    msg_type_t rcv_msg_type = M3_NULL;
    uint8_t msg[MAX_DATA_LEN + START_SEQ_LEN];
    char dummy[MAX_DATA_LEN];
    int attempts = 0;
    uint8_t skip;
    int i;
    int timeouts;

    // build init msg
    sprintf(msg, "%s%c", START_SEQ, msg_type);
    for (i=0; i<len; i++) {
        sprintf(msg+START_SEQ_LEN+1+i, "%c", data[i]);
    }

    while (rcv_msg_type != M3_ACK && attempts++ < MAX_ATTEMPTS) {
        LOG_DEBUG("sending attempt n.%d\n", attempts);
        buf[0] = M3_NULL;
        // SEND COMMAND
        if (write(sfd, msg, (START_SEQ_LEN + sizeof(msg_type_t) + len)) < 0) {     // send 5 character greetings + init command + HW addr
            LOG_DEBUG("serial_send: serial send error\n");
            continue;
        }
        // print screen info
        sprint_bytes_str(dummy, msg, START_SEQ_LEN + sizeof(msg_type_t) + len, " ");
        LOG_INFO("<--- A8: %s (%s)\n", msg, dummy);

        skip = 1; timeouts=0;
        // DON'T CONSIDER DEBUG MSG
        while (skip) {
            if (!serial_recv(sfd, buf, buflen, &rcv_msg_type)) timeouts++;
            if (timeouts>=MAX_TIMEOUTS) break;
            if (rcv_msg_type==M3_ACK||rcv_msg_type==M3_ERR||rcv_msg_type==M3_RECV) {
                skip=0;
            }
        }
    }

    LOG_DEBUG("finish sending, exiting due to msgtype: 0x%02X\n", rcv_msg_type);

    if (attempts>MAX_ATTEMPTS) {
        LOG_ERROR("cannot send (reached max attempts)\n");
        return -1;
    }
    return 0;

}
