//
// Created by Luca De Mori on 07/03/2019.
//
#include "stdint.h"

#ifndef NRG_5_IOT_LAB_A8M3_CMDS_H
#define NRG_5_IOT_LAB_A8M3_CMDS_H

// definition of commands
#define M3_NULL (msg_type_t)0x00
#define M3_INIT (msg_type_t)0x01
#define M3_SEND (msg_type_t)0x02
#define M3_STOP (msg_type_t)0x03
#define M3_RECV (msg_type_t)0x04
#define M3_ERR  (msg_type_t)0xFF
#define M3_ACK  (msg_type_t)0xF0
#define M3_DEBUG (msg_type_t)0xFE

#define MAX_DATA_LEN 127 //802154MTU

#define START_SEQ_LEN 5
#define START_SEQ     "messg"
#define STOP_SEQ_LEN 1
#define STOP_SEQ "\n"

typedef uint8_t msg_type_t;

#endif //NRG_5_IOT_LAB_A8M3_CMDS_H
