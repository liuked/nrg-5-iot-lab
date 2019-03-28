//
// Created by Luca De Mori on 28/03/2019.
//

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "iotm3_ops.h"


int main(int argc, void *argv) {

    nic_handle_t *handle;
    uint8_t *msg;
    size_t msglen;
    int addr;
    packet_t *pkt;
    uint8_t *hw_addr[IEEE802154_SHORT_ADDRESS_LEN];
    hw_addr[0] = 0;

    pkt = alloc_packet(MAX_DATA_LEN);

    LOG_DEBUG("opening M3 communications\n");
    if (iotm3_open(NULL, &handle)) {
        // second attempt
        LOG_DEBUG("error, retry...\n");
        iotm3_close(handle);
        if (iotm3_open(NULL, &handle)) {
            LOG_ERROR("unable to open m3 interface\n");
            return -1;
        }
    }
    LOG_DEBUG("M3 initialized!\n");


    msg = malloc(MAX_DATA_LEN*sizeof(char));
    printf("acquire msg (press enter at the end):\n > ");
    fflush(stdin);
    getline(msg, &msglen, stdin);
    //if (!msglen) break;
    printf("acquire last addr byte (256 is broadcast):\n > ");
    fflush(stdin);
    do scanf("%02hhX", hw_addr+1); while (!hw_addr[1]);
    LOG_DEBUG("building packet\n");
    packet_append_data(pkt, msg, msglen);
    if (hw_addr[1]==256) {
        iotm3_broadcast(handle, pkt);
    }
    LOG_INFO("sending to %02X:%02X \n", hw_addr[1], hw_addr[0]);
    iotm3_send(handle, pkt, hw_addr);
    free(msg);
    return 0;
}