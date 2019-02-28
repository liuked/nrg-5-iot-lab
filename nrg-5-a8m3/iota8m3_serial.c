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

    printf("Opening serial port...");

    serial_t serial;
    serial = open(PORT, O_RDWR );               /* Try user input depending on port */

    if (serial < 0) {                                   /* cannot open     */
        printf("ser_conn: error opening serial...");
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

    newtio.c_cc[VMIN]  = 0;
    newtio.c_cc[VTIME] = 30;

    if (tcflush(serial, TCIFLUSH) < 0) {             /* Flush the serial port            */
        close(serial);
        return -1;
    }

    if (tcsetattr(serial, TCSANOW, &newtio) < 0) {   /* Set the parameters                */
        close(serial);
        return -1;
    }


    if (ioctl(serial, TIOCGSERIAL, &ser_info) < 0) {
        perror ("TIOCGSERIAL");
        return -1;
    }
    ser_info.flags = ASYNC_SPD_CUST | ASYNC_LOW_LATENCY;
    ser_info.custom_divisor = ser_info.baud_base / BAUDRATE;
    if (ioctl(serial, TIOCSSERIAL, &ser_info) < 0) {
        perror ("TIOCSSERIAL");
        return -1;
    }


    return serial;

}