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
    size_t buflen = MAX_DATA_LEN*sizeof(char);
    size_t msglen = 0;
    int addr;
    packet_t *pkt;
    uint8_t *hw_addr[IEEE802154_SHORT_ADDRESS_LEN];
    hw_addr[1] = 0; hw_addr[0] = 0;
    char dummy[MAX_SER_MSG_LEN];

    pkt = alloc_packet(MAX_DATA_LEN);

    LOG_DEBUG("opening M3 communications\n");
    if (iotm3_open(NULL, &handle)) {
        // second attempt
        LOG_DEBUG("error, retry...\n");
        if (handle) iotm3_close(handle);
        if (iotm3_open(NULL, &handle)) {
            LOG_ERROR("unable to open m3 interface\n");
            return -1;
        }
    }
    LOG_DEBUG("M3 initialized!\n");


    msg = malloc(buflen);
    printf("acquire msg (press enter at the end):\n > ");
    fflush(stdin);
    do {
        scanf("%c", msg+msglen);
    } while (msg[msglen++]!='\n' && msglen<buflen);
    sprint_bytes_str(dummy, msg, msglen, " ");
    LOG_INFO("msg[%d]: %s\n", msglen, dummy);
    //if (!msglen) break;
    printf("acquire last addr byte (255 is broadcast):\n > ");
    fflush(stdin);
    do scanf("%hhd", hw_addr); while (!hw_addr[0]);
    LOG_DEBUG("building packet\n");
    packet_append_data(pkt, msg, msglen);
    sprint_bytes_str(dummy, (uint8_t *)pkt->data, (size_t)pkt->byte_len, " ");
    LOG_INFO("packet: [%d] %s\n", (int)pkt->byte_len, dummy);
    if (hw_addr[0]==255) {
        LOG_INFO("broadcasting\n");
        iotm3_broadcast(handle, pkt);
    } else {
        LOG_INFO("sending to %02X:%02X \n", hw_addr[1], hw_addr[0]);
        iotm3_send(handle, pkt, hw_addr);
    }

    LOG_DEBUG("freeing msg\n");
    free(msg);
    LOG_DEBUG("freeing pkt\n");
    free_packet(pkt);


    LOG_INFO("listening\n");
    int iotm3_receive(nic_handle_t *handle, packet_t *pkt, l2addr_t **src, l2addr_t **dst/*, void *metadata*/);



    return 0;
}