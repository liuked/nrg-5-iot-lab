#include <stdio.h>
#include <string.h>
#include "iota8m3_serial.h"

int main(int argc, char* argv)
{

    serial_t serial;
    printf("connecting serial...");
    serial = serial_connect();
    if (serial < 0) {
        printf("main: ERROR connecting port...");
        return -1;
    }
    printf("main: serial connected");

    write (serial, "start", 5);           // send 7 character greeting

    usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
    // receive 25:  approx 100 uS per char transmit
    char buf [100];
    int n = read (serial, buf, sizeof buf);  // read up to 100 characters if ready to read

    if (n>0) {
        printf("received: %s", buf);
    }



    return 0;
}
