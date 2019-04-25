#include <stdio.h>
#include <string.h>

#include "net/gnrc/netreg.h"
//#include "net/gnrc/pktdump.h"
#include "xtimer.h"
#include "shell.h"
#include <net/gnrc/netif.h>
#include "net/gnrc/netif/hdr.h"

#include "random.h"
#include "iota8m3.h"


#define MAX_LINE 128

char start_seq[START_SEQ_LEN];
char addrstr[IEEE802154_SHORT_ADDRESS_LEN+1];
static uint8_t hw_addr[IEEE802154_SHORT_ADDRESS_LEN];

uint8_t cmd;


int main(void)
{
//    uint8_t data[IEEE802154_FRAME_LEN_MAX-HDR_LENGTH];
//    uint8_t datalen;


    //

    //DEBUG("starting recv_thread");
    //a8m3_rcv_init();

//    while (1) {
//        DEBUG("M3 is alive!");
//        xtimer_sleep(10);
//    }


    uint8_t i;
    uint8_t datalen;
    uint8_t data[MAX_DATA_LEN];
    char dummy[MAX_LINE];

    DEBUG("I'm alive!");
    
    while (1) {
        // check against start seq
        i=0;
        strcpy(start_seq, START_SEQ);

        while (i<START_SEQ_LEN) {
            if (getchar() == start_seq[i]) i++; else i=0;
        }

        DEBUG("recognized message!");

        // receice 1byte for cmd (INIT, SEND, STOP, RECEIVED)
        cmd = (uint8_t)getchar();

        // switch handler
        switch (cmd){
            case M3_INIT:
                DEBUG("received M3_INIT");
                // expect 2 Bytes of data for this command (HW_ADDR)
                for (i=0; i<IEEE802154_SHORT_ADDRESS_LEN; i++){
                    hw_addr[i] = (uint8_t)getchar();
                }
                m3_init(hw_addr);
                break;
            case M3_SEND:
                DEBUG("received M3_SEND");
                // format: 2B addr, 1B size
                for (i=0; i<IEEE802154_SHORT_ADDRESS_LEN; i++){
                    hw_addr[i] = (uint8_t)getchar();
                }
                datalen = getchar();
                for (i=0; i<datalen; i++){
                    data[i] = (uint8_t)getchar();
                }
                if (hw_addr[0]==0xFF && hw_addr[1]==0xFF) {
                    sprintf(addrstr, "bcast");
                } else {
                    sprintf(addrstr, "%02X:%02X", hw_addr[0], hw_addr[1]);
                }
                sprint_bytes_str(dummy, data, datalen, " ");
                DEBUG("main: SENDING: %s", dummy);
                send_over_air(addrstr, data , datalen);
                break;
            default:
                ERROR("unrecognized command");
                continue;
        }
    }


    return 0;
}



