#include "iotm3_ops.h"
#include "log.h"
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>

/* interface handle will be returned by handle argument*/
int iotm3_open(char *name, nic_handle_t **ret_handle)
{

    iotm3_handle_t *handler = malloc(sizeof(iotm3_handle_t));

    //initialize serial a8_m3
    LOG_DEBUG("connecting sfd...\n");
    handler->sfd = serial_connect();
    if (handler->sfd < 0) {
        LOG_ERROR("cannot open serial\n");
        goto err_ret_noclose;
    }
    LOG_DEBUG("serial connected with 5 sec timeout non-blocking\n");
    
    //retrieve node id
    FILE *fi;
    char *fileline;  //string to read the file
    size_t linelen;
    char *nodeidstr;  // pointers to the fileline string, cursor to the already allocated memory
    char *tok;        //
//    LOG_DEBUG("open file\n");
    fi = fopen(HOSTNAME_FILE, "r");
    if (fi==NULL) {
        LOG_ERROR("unable to open file "HOSTNAME_FILE);
        goto err_ret;
    }

    fileline = malloc(MAX_LINE*sizeof(char));
//    LOG_DEBUG("getline\n");
    getline(&fileline, &linelen, fi);
//    LOG_DEBUG("got!\n");
    if (linelen) {
//        LOG_DEBUG("tok: %s\n", fileline);
        tok = strtok(fileline, "-");
    } else {
        LOG_ERROR("cannot read node id");
        goto err_ret;
    }
    while (tok) {
//        LOG_DEBUG("tok: %s\n", tok);
        nodeidstr = tok;
        tok = strtok(NULL, "-");
    }

    if (nodeidstr) {
        LOG_INFO("detected node id: %s\n", nodeidstr);
        handler->if_mac[1] = (uint8_t) atoi(nodeidstr) % 255;
        handler->if_mac[0] = (uint8_t) atoi(nodeidstr) / 255;
    } else {
        LOG_ERROR("unable to detect node id, fallback to 40:04\n")
    }

    free(fileline);

    //send  init command to m3 with proper hw_addr
    LOG_INFO("starting M3 radio with hw_addr: %d %d\n", handler->if_mac[0], handler->if_mac[1]);
    if (serial_send(handler->sfd, handler->buf, MAX_SER_MSG_LEN, M3_INIT, (uint8_t*)handler->if_mac, IEEE802154_SHORT_ADDRESS_LEN) < 0) {
        LOG_ERROR("cannot send M3_INIT\n");
        goto err_ret;
    }
    LOG_DEBUG("getting mtu\n");
    //get mtu
    //FIXME: get real mtu instead of fixe done
    handler->if_mtu = 127;
    LOG_DEBUG("returning handler\n");
    *ret_handle = (nic_handle_t*)handler;
    LOG_DEBUG("ret 0\n");
    return 0;

err_ret:
    close(handler->sfd);
err_ret_noclose:
    free(handler);
    return -1;
}

int iotm3_close(nic_handle_t *handle) {
    iotm3_handle_t *iotm3_handle = (iotm3_handle_t*)handle;
    //TODO: add M3 STOP, close serial
    serial_close(iotm3_handle->sfd);
    free(iotm3_handle);
    return 0;
}

int iotm3_send(nic_handle_t *handle, packet_t *pkt, l2addr_t *dst) {
    char dummy[MAX_SER_MSG_LEN];
    iotm3_handle_t *iotm3_hdl = (iotm3_handle_t *)handle;
    // prepare addr and datalen for sending
    uint8_t prepdata[IEEE802154_SHORT_ADDRESS_LEN+1];  // 2B for addr, 1B for datalen
    LOG_DEBUG("copy addr\n");
    memcpy(prepdata, dst, IEEE802154_SHORT_ADDRESS_LEN);
    LOG_DEBUG("copy datalen\n");
    prepdata[IEEE802154_SHORT_ADDRESS_LEN]=(uint8_t)pkt->byte_len;
    sprint_bytes_str(dummy, prepdata, IEEE802154_SHORT_ADDRESS_LEN+1, " ");
    LOG_DEBUG("prepdata: %s\n", dummy),
    LOG_DEBUG("prepend data\n");
    packet_prepend_data(pkt, prepdata, IEEE802154_SHORT_ADDRESS_LEN+1);
    sprint_bytes_str(dummy, pkt->data, pkt->byte_len, " ");
    LOG_INFO("packet AFTER prepend: [%d] %s\n", pkt->byte_len, dummy);
    LOG_DEBUG("sening data\n");
    if (serial_send(iotm3_hdl->sfd, iotm3_hdl->buf, MAX_SER_MSG_LEN, M3_SEND, pkt->data, pkt->byte_len)){
        LOG_ERROR("cannot send data");
        return -1;
    }
    return 0;
}

int iotm3_broadcast(nic_handle_t *handle, packet_t *pkt) {
    uint8_t bcastaddr[2] = { 0xFF, 0xFF };
    iotm3_send(handle, pkt, (l2addr_t*)bcastaddr);
    return 0;
}

int iotm3_receive(nic_handle_t *handle, packet_t *pkt, l2addr_t **src, l2addr_t **dst) {
//    iotm3_handle_t *iotm3_handle = (iotm3_handle_t*)handle;
//
//    //send receive command
//    int numbytes = ;
//    if (numbytes <= 0) {
//        LOG_DEBUG("unable to receive a valid packet\n");
//        goto err_ret;
//    }
//
//
//    *src = alloc_l2addr(IEEE802154_SHORT_ADDRESS_LEN, );
//    *dst = alloc_l2addr(IEEE802154_SHORT_ADDRESS_LEN,  );
//
//    pkt->byte_len = numbytes - sizeof(struct ethhdr);
//    pkt->tail = pkt->data + pkt->byte_len;
    return 0;

err_ret:
    pkt->data += sizeof(struct ethhdr);
    return -1;
}

int iotm3_get_info(nic_handle_t *handle, nic_info_t *info) {
//    iotm3_handle_t *iotm3_hdl = (iotm3_handle_t*)handle;
//    info->mtu = iotm3_hdl->if_mtu;
    return 0;
}
