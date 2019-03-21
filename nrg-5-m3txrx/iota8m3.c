//
// Created by Luca De Mori on 18/12/2018.
//

#include "iota8m3.h"
#include "a8m3_cmds.h"
#include "stdio.h"

static kernel_pid_t _pid = KERNEL_PID_UNDEF;

/* handles GNRC_NETAPI_MSG_TYPE_RCV commands */
static void _receive(gnrc_pktsnip_t *pkt);
/* handles GNRC_NETAPI_MSG_TYPE_SND commands */
//static void _send(gnrc_pktsnip_t *pkt);
/* Main event loop for 6LoWPAN */
static void *_event_loop(void *args);

//#if ENABLE_DEBUG
//static char _stack[GNRC_IOTA8M3_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];
//#else
//static char _stack[GNRC_IOTA8M3_STACK_SIZE];
//#endif

static char _stack[GNRC_IOTA8M3_STACK_SIZE + THREAD_EXTRA_STACKSIZE_PRINTF];


kernel_pid_t gnrc_iota8m3_init(void)
{

    _pid = thread_create(_stack, sizeof(_stack), GNRC_IOTA8M3_PRIO,
                         THREAD_CREATE_STACKTEST, _event_loop, NULL, "iota8m3");

    return _pid;

}


//static void *_a8m3_recv(void *args)
//{
//    (void)args;
//    char *line;
//    size_t size;
//
//
//    DEBUG("shell receive thread started!");
//
//    while(1) {
//
//        if (__getline(&line, &size, stdin) == -1) {
//        } else {
//            // printf("echo: %s", line);
//        }
//    }
//
//
//    return NULL;
//}

//kernel_pid_t a8m3_rcv_init(void)
//{
//
//    _pid = thread_create(_stack, sizeof(_stack), GNRC_IOTA8M3_PRIO,
//                         THREAD_CREATE_STACKTEST, _a8m3_recv, NULL, "a8m3_recv");
//
//    return _pid;
//
//}


/**
 Receive messages from 802.15.4 interface
 **/
static void _receive(gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *payload;
    gnrc_netif_hdr_t *hdr;
//    uint8_t *src_addr[IEEE802154_LONG_ADDRESS_LEN], *dst_addr[IEEE802154_LONG_ADDRESS_LEN];

    /* seize payload as a temporary variable */
    payload = gnrc_pktbuf_start_write(pkt); /* need to duplicate since pkt->next
                                             * might get replaced */

    if (payload == NULL) {
        DEBUG("iota8m3: can not get write access on received packet\n");

        gnrc_pktbuf_release(pkt);
        return;
    }

    payload = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_NETIF);

    /* to be added if problem with memory: remove first header and put in pkt just payload (netif massage)*/
//    if ((payload == NULL) || (payload->size < 1)) {
//        DEBUG("iota8m3: Received packet has no NETIF payload\n");
//        gnrc_pktbuf_release(pkt);
//        return;
//    }

    hdr = (gnrc_netif_hdr_t *)payload->data;
    gnrc_netif_hdr_print((gnrc_netif_hdr_t *)payload->data);


    //print in human readable format
    putchar(M3_RECV);
    putchar('$'); print_bytes_str(gnrc_netif_hdr_get_src_addr(hdr), hdr->src_l2addr_len, ":");
    putchar('@'); print_bytes_str(gnrc_netif_hdr_get_dst_addr(hdr), hdr->dst_l2addr_len, ":");
    putchar('#'); printf("%02X", pkt->size); putchar(':');
    putchar('&'); print_bytes_str(pkt->data, pkt->size, ":");

    //print bytes for struct read
    putchar('#');
    putbytes(gnrc_netif_hdr_get_src_addr(hdr), hdr->src_l2addr_len);
    putbytes(gnrc_netif_hdr_get_dst_addr(hdr), hdr->dst_l2addr_len);
    putbytes((uint8_t *)&(pkt->size), sizeof(pkt->size));
    putbytes(pkt->data, pkt->size);


    putchar('\n');

    gnrc_pktbuf_release(pkt);
    return;
}

void sprint_bytes_str(char* str, const uint8_t *addr, size_t addr_len, const char *separator)
{
    for (size_t i = 0; i < addr_len; i++) {
        if (i != 0 && separator) {
            sprintf(str, "%s", separator);
        }
        sprintf(str, "%02X", (unsigned)addr[i]);
    }
}

void print_bytes_str(const uint8_t *addr, size_t addr_len, const char *separator)
{
    for (size_t i = 0; i < addr_len; i++) {
        if (i != 0 && separator) {
            printf("%s", separator);
        }
        printf("%02X", (unsigned)addr[i]);
    }
}

void putbytes(uint8_t *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        putchar(bytes[i]);
    }
}


static void *_event_loop(void *args)
{
    msg_t msg, reply, msg_q[GNRC_IOTA8M3_MSG_QUEUE_SIZE];
    gnrc_netreg_entry_t me_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                            sched_active_pid);

    (void)args;
    msg_init_queue(msg_q, GNRC_IOTA8M3_MSG_QUEUE_SIZE);

    /* register interest in all packets */
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &me_reg);

    /* preinitialize ACK */
    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;

    /* start event loop */
    while (1) {
        DEBUG("iota8m3: waiting for incoming message.");
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                DEBUG("iota8m3: GNRC_NETDEV_MSG_TYPE_RCV received");
                _receive(msg.content.ptr);
                break;

            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("iota8m3: GNRC_NETDEV_MSG_TYPE_SND received");
                // _send(msg.content.ptr);
                break;

            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("iota8m3: reply to unsupported get/set");
                reply.content.value = -ENOTSUP;
                msg_reply(&msg, &reply);
                break;

            default:
                DEBUG("iota8m3: operation not supported");
                break;
        }
    }

    return NULL;
}
