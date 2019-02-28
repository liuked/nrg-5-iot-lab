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

#define MAX_MSG_LEN (127U)
#define MAX_LINE 80


static gnrc_netif_t *netif = NULL;
static kernel_pid_t iface = 0;


/* utility functions */
static inline bool _is_iface(kernel_pid_t iface);
int send(kernel_pid_t iface, char *addrstr, uint8_t* data, uint8_t len);

int _load_iface(const uint8_t* new_addr, int addr_len);
static uint8_t hw_addr[IEEE802154_SHORT_ADDRESS_LEN];

const char start_seq[START_SEQ_LEN] = { 's', 't', 'a' ,'r' ,'t'};

int main(void)
{

    random_init(0);

    uint8_t i;
    uint8_t data[IEEE802154_FRAME_LEN_MAX-HDR_LENGTH];
    uint8_t datalen;



    // check against start seq
    i=0;
    while (i<START_SEQ_LEN) {
        if (getchar() == start_seq[i]) i++; else i=0;
    }

    puts("ENTERED!");

    for (i=0; i<IEEE802154_SHORT_ADDRESS_LEN; i++){
        hw_addr[i] = (uint8_t)getchar();
    }

    /*Loading Interface with new address*/
    if (_load_iface(hw_addr, IEEE802154_SHORT_ADDRESS_LEN) < 0) {
        puts("error loading interface\n");
        return -1;
    }

    printf("iface: %d\n", iface);
    puts("Addr:"); print_bytes_str((uint8_t *)&iface, IEEE802154_SHORT_ADDRESS_LEN, ":");
    putchar('\n');

    /* init receiver */
    puts("starting receiver\n");
    gnrc_iota8m3_init();

    /* start sending data */
    puts("starting random generator\n");
    while (1) {
        xtimer_sleep(1);
        datalen = (uint8_t)random_uint32_range(1, 10);
        random_bytes(data, datalen);
        send(iface, "bcast", data, datalen);
    }


    return 0;
}

int _load_iface(const uint8_t* new_addr, int addr_len) {

    uint16_t devtyp;

    int res;

    puts("scan interfaces...");
    while ((netif = gnrc_netif_iter(netif))) {
        iface = netif->pid;
        res = gnrc_netapi_get(iface, NETOPT_DEVICE_TYPE, 0, &devtyp, sizeof(devtyp));
        if (res >= 0 && devtyp == 3) break;
    }
    if (!netif){
        puts("ERROR loading iface");
        return -1;
    }

    printf("Selected %d\n", iface);

    // Setting address
    puts("set address...");
    if (gnrc_netapi_set(iface, NETOPT_ADDRESS, 0, (void *)new_addr, addr_len) != addr_len) {
        return -1;
    }

    uint16_t alen = 2;
    // Setting src addredd length
    puts("set src addr len...");
    if (gnrc_netapi_set(iface, NETOPT_SRC_LEN, 0, &alen, sizeof(uint16_t)) != sizeof(uint16_t)) {
        return  -1;
    }

    puts("interface loaded successfully!");

    return 0;
}


int send(kernel_pid_t iface, char *addrstr, uint8_t* data, uint8_t len)
{

    uint8_t addr[GNRC_NETIF_L2ADDR_MAXLEN];
    size_t addr_len;
    gnrc_pktsnip_t *pkt, *hdr;
    gnrc_netif_hdr_t *nethdr;
    uint8_t flags = 0x00;

    if (!_is_iface(iface)) {
        puts("error: invalid interface given");
        return 1;
    }

    /* parse address */
    addr_len = gnrc_netif_addr_from_str(addrstr, addr);

    if (addr_len == 0) {
        if (strcmp(addrstr, "bcast") == 0) {
            flags |= GNRC_NETIF_HDR_FLAGS_BROADCAST;
        }
        else {
            puts("error: invalid address given");
            return 1;
        }
    }

    /* put packet together */
    pkt = gnrc_pktbuf_add(NULL, data, len, GNRC_NETTYPE_UNDEF);
    if (pkt == NULL) {
        puts("error: packet buffer full");
        return 1;
    }
    hdr = gnrc_netif_hdr_build(NULL, 0, addr, addr_len);        // defined in hdr.h
    if (hdr == NULL) {
        puts("error: packet buffer full");
        gnrc_pktbuf_release(pkt);
        return 1;
    }
    LL_PREPEND(pkt, hdr);
    nethdr = (gnrc_netif_hdr_t *)hdr->data;
    nethdr->flags = flags;
    /* and send it */
    if (gnrc_netapi_send(iface, pkt) < 1) {
        puts("error: unable to send");
        gnrc_pktbuf_release(pkt);
        return 1;
    }

    return 0;
}

static inline bool _is_iface(kernel_pid_t iface)
{
    return (gnrc_netif_get_by_pid(iface) != NULL);
}
