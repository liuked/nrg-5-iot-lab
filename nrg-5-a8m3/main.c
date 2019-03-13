#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "iota8m3_serial.h"

#define START_SEQ_LEN (5)
#define MAX_LINE 100

int c;
uint8_t msg[128] = { 'm', 'e', 's' ,'s' ,'g', M3_INIT, 0xF1, 0x6A };
char buf [MAX_LINE];

int main(int argc, char* argv)
{

    serial_t serial;
    debug("connecting serial...\n");
    serial = serial_connect();
    if (serial < 0) {
        error("main: cannot open port\n");
        return -1;
    }
    info("main: serial connected with 5 sec timeout non-blocking \n");



    int n = 0;
    uint8_t msg_typ = 0;

//    printf("main: waiting for m3\n");

//    while(!n) {   // TODO: change to !n
//        n = read(serial, buf, sizeof buf);  // read up to 100 characters if ready to read
//        if (n>0) {
//
//            printf("m3: %s", buf);
//        } else {
//            debug("main: serial timeout\n");
//        }
//    }
//    info("m3 detected!\n");


//    sleep(5);


    int attempts = 0;


    while (msg_typ!=M3_ACK && attempts<3) {


        memset(buf, 0, 100);

        // SEND INIT COMMAND
        if (write (serial, msg, 8) < 0) {     // send 5 character greetings + init command + HW addr
            error("main: serial send error");
            continue;
        }
        printf("---> A8: (1) %s", msg);
        printf("  (");
        print_bytes_str(msg,8," ");
        printf(")\n");
        attempts++;

        n = reads (serial, buf, MAX_LINE);

        // SKIP DEBUG COMMENTS
        while (n!=0 && buf[0]!=M3_ERR && buf[0]!=M3_ACK) {
            printf("<--- M3[%d]: %s", n, buf);
#ifdef VERBOSE
            printf("  (");
        print_bytes_str(buf,n," ");
        printf(")\n");
#endif
            n = reads (serial, buf, MAX_LINE);  // read up to MAX_LINE characters if ready to read
            if (!n) debug("serial timeout");
        }

        msg_typ = buf[0];

        printf("<--- M3[%d]: %s", n, buf);
#ifdef VERBOSE
        printf("  (");
        print_bytes_str(buf,n," ");
        printf(")\n");
#endif

        switch (msg_typ) {
            case M3_ACK:
                info("[M3 ACK]!");
                break;
            case M3_ERR:
                info("[M3 ERR]");
                break;
            default:
                break;
        }


        sleep(3);


    }


    printf("PRESS KEY To CONTINE...");
    getchar();

    // SEND SEND COMMAND
    sprintf (msg, "messg%c", M3_SEND);
    if (write (serial, msg, 6) < 0) {     // send 5 character greetings + init command + HW addr
        error("main: serial send error");
    }
    printf("---> A8: (1) %s", msg);
    printf("  (");
    print_bytes_str(msg,8," ");
    printf(")\n");



    while(1) {
        n = reads (serial, buf, sizeof buf);  // read up to 100 characters if ready to read
        if (n>0) {
            printf("<--- M3[%d]: %s", n, buf);
#ifdef VERBOSE
            printf("  (");
        print_bytes_str(buf,n," ");
        printf(")\n");
#endif

        } else {
            debug ("serial timeout");
        }

    }




    return 0;
}
