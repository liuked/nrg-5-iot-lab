#ifndef __IOTM3_OPS_H__
#define __IOTM3_OPS_H__

#include "packet.h"
#include "nic_ops.h"
#include "iota8m3_serial.h"

#define IEEE802154_SHORT_ADDRESS_LEN 2

struct iotm3_handle_s {
    serial_t sfd;
    uint8_t buf [MAX_SER_MSG_LEN];
    int if_index;
    char if_mac[IEEE802154_SHORT_ADDRESS_LEN];
    int if_mtu;
};

/*
struct iotm3_metadata_s {
    int rssi;
    int lqi;
};

typedef struct iotm3_metadata_s iotm3_metadata_t;
 */

typedef struct iotm3_handle_s iotm3_handle_t;

int iotm3_open(char *name, nic_handle_t **handle);
int iotm3_close(nic_handle_t *handle);
int iotm3_send(nic_handle_t *handle, packet_t *pkt, l2addr_t *dst);
int iotm3_broadcast(nic_handle_t *handle, packet_t *pkt);
int iotm3_receive(nic_handle_t *handle, packet_t *pkt, l2addr_t **src, l2addr_t **dst/*, void *metadata*/);
int iotm3_get_info(nic_handle_t *handle, nic_info_t *info);

#endif
