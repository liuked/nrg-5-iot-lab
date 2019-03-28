#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "iota8m3_serial.h"
#include <pthread.h>
//#include "iotm3_ops.h"


#define IEEE802154_SHORT_ADDRESS_LEN 2

int c;
uint8_t msg[128];
uint8_t hw_addr[2];
uint8_t buf [MAX_SER_MSG_LEN];

char *fileline;
size_t linelen;
char *nodeidstr;
uint8_t skip;
serial_t sfd;
pthread_t tid;


void *serial_listen(void *vargp);


int main(int argc, char *argv[]) {


    DEBUG("main: connecting sfd...");
    sfd = serial_connect();
    if (sfd < 0) {
        ERROR("main: cannot open port");
        return -1;
    }

    INFO("main: serial connected with 5 sec timeout non-blocking");
    INFO("main: starting listener thread");


    pthread_create(&tid, NULL, serial_listen, (void *)&sfd);

    int n = 0;
    msg_type_t msg_typ;

//    printf("main: waiting for m3\n");

//    while(!n) {   // TODO: change to !n
//        n = read(sfd, buf, sizeof buf);  // read up to 100 characters if ready to read
//        if (n>0) {
//
//            printf("m3: %s", buf);
//        } else {
//            debug("main: sfd timeout\n");
//        }
//    }
//    info("m3 detected!\n");


//    sleep(5);


    int attempts = 0;

    //printf ("argc: %d, argv: %s| %s %s", argc, argv[0], argv[1], argv[2]);


    FILE *fi;
    fi = fopen(HOSTNAME_FILE, "r");
    getline(&fileline, &linelen, fi);
    //printf("%s", fileline);

    char *tok;
    if (linelen) tok = strtok(fileline, "-");
    while (tok) {
        //printf("%s", tok);
        nodeidstr = tok;
        tok = strtok(NULL, "-");
    }
    INFO("detected node id: %s", nodeidstr);
    if (nodeidstr) {
        hw_addr[0] = (uint8_t) atoi(nodeidstr) % 255;
        hw_addr[1] = (uint8_t) atoi(nodeidstr) / 255;
    } else if (argc > 2) {
        hw_addr[0] = (uint8_t) atoi(argv[1]);
        hw_addr[1] = (uint8_t) atoi(argv[2]);
    } // uses the args as address

    uint8_t state = 0;

    while (state<3) {

        INFO("main: STARTING M3 RADIO with hw_addr: %d %d", hw_addr[0], hw_addr[1]);
        if (serial_send(sfd, buf, MAX_SER_MSG_LEN, M3_INIT, hw_addr, IEEE802154_SHORT_ADDRESS_LEN) < 0) {
            ERROR("main: cannot send M3_INIT");
            exit(-1);
        }


        INFO("main: STARTING SENDER...");
        sprintf(msg, "Hello I am node %s [wave #%d]", nodeidstr, j);
        if (serial_send(sfd, buf, MAX_SER_MSG_LEN, M3_SEND, msg, 0) < 0) {
            ERROR("main: cannot send M3_SEND");
            exit(-1);
        }

        j++;
    }

    INFO("main: CLOSING LISTENER THREAD");
    pthread_exit(NULL);


    return 0;
}


void *serial_listen(void *vargp)
{
    serial_t  sfd = *(serial_t *)vargp;
    int n;
    msg_type_t msg_typ;

    DEBUG("serial_listen: thread is started");
    static int s = 0;



}
