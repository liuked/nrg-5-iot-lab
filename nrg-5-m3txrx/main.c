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


static gnrc_netif_t *netif = NULL;
static kernel_pid_t iface = 0;


/* utility functions */
static inline bool _is_iface(kernel_pid_t iface);
int send(kernel_pid_t iface, const char *addrstr, uint8_t* data, uint8_t len);
int _send_on_802154(const char *addrstr, const void *data, size_t size);

int _load_iface(const uint8_t* new_addr, uint8_t addr_len);
static uint8_t hw_addr[IEEE802154_SHORT_ADDRESS_LEN];
int _m3_init(const uint8_t* hw_addr);

char start_seq[START_SEQ_LEN];
char addrstr[IEEE802154_SHORT_ADDRESS_LEN+1];



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
                _m3_init(hw_addr);
                break;
            case M3_SEND:
                DEBUG("received M3_SEND");
                sprintf(addrstr, "%02X:%02X", hw_addr[0], hw_addr[1]);
                _send_on_802154("bcast", &i, sizeof(i));
                break;
            default:
                ERROR("unrecognized command");
                continue;
        }
    }


    return 0;
}

int _send_on_802154(const char *addrstr, const void *data, size_t size)
{
    /* start sending data */
    (void) data;
    (void) size;
    if (!pid_is_valid(iface)) ERROR("radio interface not found or not initialized");

    DEBUG("starting data generator");
    uint16_t d = 0;
    ACK("Send operation ongoing");
    while (1) {
        xtimer_sleep(1);
//        datalen = (uint8_t)random_uint32_range(1, 10);
//        random_bytes(data, datalen);
        DEBUG("sending data");
        d++;
        send(iface, addrstr, (uint8_t *)&d, sizeof(uint16_t));
    }
}

int _m3_init(const uint8_t* hw_addr)
{

    DEBUG("changing hw address to: %02X:%02X", hw_addr[0], hw_addr[1]);

    // then try to load the interface with the new address

    /*Loading Interface with new address*/
    if (_load_iface(hw_addr, IEEE802154_SHORT_ADDRESS_LEN) < 0) {
        ERROR("failed loading interface");
        return -1;
    }

    /* init receiver */
    DEBUG("starting receiver...");
    if (gnrc_iota8m3_init() > KERNEL_PID_UNDEF) {
        DEBUG("OK!");
    } else {
        ERROR("receiver cannot start");
    }

    ACK(" iface: %d Addr: %02X:%02X", iface, hw_addr[0], hw_addr[1]);
//    sprint_bytes_str((char *)(a8msg+14+(iface/10+1)), hw_addr, IEEE802154_SHORT_ADDRESS_LEN, ":");
//    sprintf((char *)(a8msg+14+(iface/10+1)+IEEE802154_SHORT_ADDRESS_LEN), "\n");


    return 0;

}

int _load_iface(const uint8_t* new_addr, uint8_t addr_len) {

    uint16_t devtyp;

    int res;

    if (iface)  // if already loaded use that interface
        return iface;

    DEBUG("_load_iface: scan interfaces...");
    while ((netif = gnrc_netif_iter(netif))) {
        iface = netif->pid;
        res = gnrc_netapi_get(iface, NETOPT_DEVICE_TYPE, 0, &devtyp, sizeof(devtyp));
        if (res >= 0 && devtyp == 3) break;
    }
    if (!netif){
        ERROR("_load_iface: no available interfaces found");
        return -1;
    }

    DEBUG("_load_iface: selected %d", iface);

    // Setting address
    if (gnrc_netapi_set(iface, NETOPT_ADDRESS, 0, (void *)new_addr, addr_len) != addr_len) {
        return -1;
    }

    uint16_t alen = (uint16_t)addr_len;
    // Setting src addredd length
    if (gnrc_netapi_set(iface, NETOPT_SRC_LEN, 0, &alen, sizeof(uint16_t)) != sizeof(uint16_t)) {
        return  -1;
    }

    DEBUG("_load_iface: interface loaded successfully!");

    return 0;
}


int send(kernel_pid_t iface, const char *addrstr, uint8_t* data, uint8_t len)
{

    uint8_t addr[GNRC_NETIF_L2ADDR_MAXLEN];
    uint8_t addr_len;
    gnrc_pktsnip_t *pkt, *hdr;
    gnrc_netif_hdr_t *nethdr;
    uint8_t flags = 0x00;

    if (!_is_iface(iface)) {
        ERROR("send: invalid interface given");
        return 1;
    }


    /* parse address */
    addr_len = gnrc_netif_addr_from_str(addrstr, addr);

    if (addr_len == 0) {
        if (strcmp(addrstr, "bcast") == 0) {
            flags |= GNRC_NETIF_HDR_FLAGS_BROADCAST;
        }
        else {
            ERROR("send: invalid address given");
            return 1;
        }
    }

    /* put packet together */
    pkt = gnrc_pktbuf_add(NULL, data, len, GNRC_NETTYPE_UNDEF);
    if (pkt == NULL) {
        ERROR("send: packet buffer full");
        return 1;
    }
    hdr = gnrc_netif_hdr_build(NULL, 0, addr, addr_len);        // defined in hdr.h
    if (hdr == NULL) {
        ERROR("send: packet buffer full");
        gnrc_pktbuf_release(pkt);
        return 1;
    }
    LL_PREPEND(pkt, hdr);
    nethdr = (gnrc_netif_hdr_t *)hdr->data;
    nethdr->flags = flags;
    /* and send it */
    if (gnrc_netapi_send(iface, pkt) < 1) {
        ERROR("send: unable to send");
        gnrc_pktbuf_release(pkt);
        return 1;
    }

    return 0;
}

static inline bool _is_iface(kernel_pid_t iface)
{
    return (gnrc_netif_get_by_pid(iface) != NULL);
}

