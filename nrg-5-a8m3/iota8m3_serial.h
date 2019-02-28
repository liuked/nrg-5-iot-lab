//
// Created by Luca De Mori on 19/12/2018.
//

#ifndef NRG_5_SORBONNE_IOTA8M3_SERIAL_H
#define NRG_5_SORBONNE_IOTA8M3_SERIAL_H

#define PORT "/dev/ttyUSB1"
#define BAUDRATE 500000

#define INVALID_FD (-1)

typedef int serial_t;

serial_t serial_connect(void);


#endif //NRG_5_SORBONNE_IOTA8M3_SERIAL_H
