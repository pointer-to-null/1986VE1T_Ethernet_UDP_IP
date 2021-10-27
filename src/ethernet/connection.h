#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>

typedef struct {
	uint8_t mac[6];
	uint8_t ip[4];
	uint16_t port;
} connection_t;

void connDeinit(connection_t *connection);
void connSetMAC(connection_t *connection, const uint8_t *mac);
void connGetMAC(uint8_t *mac, const connection_t *connection);
void connSetIP(connection_t *connection, const uint8_t *ip);
void connGetIP(uint8_t *ip, const connection_t *connection);
void connBroadcastInit(void);

#endif //CONNECTION_H
