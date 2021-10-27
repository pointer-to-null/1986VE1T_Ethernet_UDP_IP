#ifndef ETHIO_H
#define ETHIO_H

#include <stdint.h>

#include "connection.h"

void eprintf(const connection_t *eth_conn, const char *string, ...);
void SendEthARP(const uint8_t *arp_frame);
void DirectSendDataEthUDP(const connection_t *eth_conn, const uint8_t *data, uint16_t data_size);

#endif //ETHIO_H
