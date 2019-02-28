#include <stdio.h>
#include <string.h>

#include "net/gnrc/netreg.h"
//#include "net/gnrc/pktdump.h"
#include "xtimer.h"
#include "shell.h"
#include "net/netdev/ieee802154.h"
#include <net/gnrc/netif.h>
#include <net/gnrc/netapi.h>
#include "at86rf2xx.h"
#include "od.h"
#include "net/ieee802154.h"
#include "net/gnrc/netif/hdr.h"
#include "random.h"

#define MAX_MSG_LEN (127U)
#define MAX_LINE 80
#define _STACKSIZE      (THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF)


int send(kernel_pid_t iface, char *addrstr, char* data);
static inline bool _is_iface(kernel_pid_t iface);




int receive(int argc, char **argv);
void print_addr(uint8_t *addr, size_t addr_len);


int go(int argc, char **argv);
static gnrc_netif_t *netif = NULL;
static kernel_pid_t iface = 0;

static char stack[_STACKSIZE];
static kernel_pid_t _recv_pid;

void *_recv_thread(void *arg);
int _load_iface(uint8_t* new_addr);
void recv(netdev_t *dev); static uint8_t buffer[IEEE802154_FRAME_LEN_MAX];


int main(void)
{

    uint8_t hw_addr[IEEE802154_SHORT_ADDRESS_LEN];

    /* Set random HW address*/
    random_init(0);
    random_bytes(hw_addr, IEEE802154_SHORT_ADDRESS_LEN);

    _load_iface(hw_addr);

    _recv_pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1,
                              THREAD_CREATE_STACKTEST, _recv_thread, NULL,
                              "recv_thread");

    /* Start recv thread */
    if (_recv_pid <= KERNEL_PID_UNDEF) {
        puts("Creation of receiver thread failed");
        return 1;
    }

    static const shell_command_t commands[] = {
        { "go", "send test msg", go },
        { "recv", "listen on ieee802154 iface", receive },
        { NULL, NULL, NULL }
    };

    /* Start sending data */

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}


void *_recv_thread(void *arg)
{
    (void)arg;
    while (1) {
        if (netif){
            recv(netif->dev);
        }

    }
}

int _load_iface(uint8_t* new_addr) {

    uint16_t devtyp;
    int res;

    while ((netif = gnrc_netif_iter(netif))) {
        iface = netif->pid;
        res = gnrc_netapi_get(iface, NETOPT_DEVICE_TYPE, 0, &devtyp, sizeof(devtyp));
        if (res >= 0 && devtyp == 3) break;
    }

    // Setting address
    if (gnrc_netapi_set(iface, NETOPT_ADDRESS, 0, (void *)new_addr, sizeof(new_addr)) != sizeof(new_addr)) {
        return -1;
    }

    uint16_t alen = 2;
    // Setting src addredd length
    if (gnrc_netapi_set(iface, NETOPT_SRC_LEN, 0, &alen, sizeof(uint16_t)) != sizeof(uint16_t)) {

        return  -1;
    }


    return 0;
}

int go(int argc, char **argv) {


    uint16_t devtyp;
    int res;

    printf("Selecting Interface - ");
    while ((netif = gnrc_netif_iter(netif))) {
        iface = netif->pid;
        res = gnrc_netapi_get(iface, NETOPT_DEVICE_TYPE, 0, &devtyp, sizeof(devtyp));
        if (res >= 0 && devtyp == 3) break;
    }

    printf("found %d. OK\n", iface);
    static const uint8_t new_addr[] = { 0x01, 0x02 };
    printf("Setting address - ");
    if (gnrc_netapi_set(iface, NETOPT_ADDRESS, 0, (void *)new_addr, sizeof(new_addr)) != sizeof(new_addr)) {
        puts("Error setting device address");
        return 0;
    }
    printf("OK.\n");

    uint16_t alen = 2;
    printf("Setting src addredd length - ");
    if (gnrc_netapi_set(iface, NETOPT_SRC_LEN, 0, &alen, sizeof(uint16_t)) != sizeof(uint16_t)) {
        puts("Error setting src address len");
        return 0;
    }
    printf("OK.\n");


    char data[] = "prova 1 2 3";

    char addr[] = "01:02";

    char *dst_addr = addr;

    if (argc >= 2) {
        dst_addr = argv[1];
    }



    printf("Sending message to %s through iface (%d): %s - ", dst_addr, iface, data);
    send(iface, dst_addr, data);
    printf("OK.\n");



    return 0;

}

int receive(int argc, char **argv){


    (void)argc;
    (void)argv;

    if (netif) {
        recv(netif->dev);
    } else {
        printf("interface not selected");
        return -1;
    }


    return 0;
}

void print_addr(uint8_t *addr, size_t addr_len)
{
    for (size_t i = 0; i < addr_len; i++) {
        if (i != 0) {
            printf(":");
        }
        printf("%02x", (unsigned)addr[i]);
    }
}


void hdr_print(gnrc_pktsnip_t* pkt)
{
    assert(pkt != NULL);

    pkt = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_NETIF);
    if (pkt && pkt->data) {
        gnrc_netif_hdr_t *netif_hdr = pkt->data;
        gnrc_netif_hdr_print(netif_hdr);
    }
    return;
}



void recv(netdev_t *dev)
{
    uint8_t src[IEEE802154_LONG_ADDRESS_LEN], dst[IEEE802154_LONG_ADDRESS_LEN];
    size_t mhr_len, data_len, src_len, dst_len;
    netdev_ieee802154_rx_info_t rx_info;
    le_uint16_t src_pan, dst_pan;

    data_len = dev->driver->recv(dev, buffer, sizeof(buffer), &rx_info);
    mhr_len = ieee802154_get_frame_hdr_len(buffer);
    if (mhr_len == 0) {
        return;
    }

    dst_len = ieee802154_get_dst(buffer, dst, &dst_pan);
    src_len = ieee802154_get_src(buffer, src, &src_pan);


    switch (buffer[0] & IEEE802154_FCF_TYPE_MASK) {
        case IEEE802154_FCF_TYPE_BEACON:
            puts("BEACON");
            break;
        case IEEE802154_FCF_TYPE_DATA:
            puts("DATA");
            break;
        case IEEE802154_FCF_TYPE_ACK:
            puts("ACK");
            break;
        case IEEE802154_FCF_TYPE_MACCMD:
            puts("MACCMD");
            break;
        default:
            puts("UNKNOWN");
            break;
    }

    printf(" S: "); print_addr(src, src_len);
    printf(" D: "); print_addr(dst, dst_len);

    printf(", ACKR: ");
    if (buffer[0] & IEEE802154_FCF_ACK_REQ) {
        printf("1, ");
    }
    else {
        printf("0, ");
    }
    printf("SEQ: %u, ", (unsigned)ieee802154_get_seq(buffer));
    printf("DLEN: %d, ", data_len-mhr_len);
    printf("RSSI: %u, LQI: %u\n", rx_info.rssi, rx_info.lqi);
}


static inline bool _is_iface(kernel_pid_t iface)
{
    return (gnrc_netif_get_by_pid(iface) != NULL);
}


int send(kernel_pid_t iface, char *addrstr, char* data)
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
    pkt = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
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