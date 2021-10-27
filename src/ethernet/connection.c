#include "connection.h"

#include <stdint.h>

#include "udp_ip.h"
#include "internal_ethernet.h"

static connection_t local_broadcast_con; //Broadcast connection.

static void CalculateBroadcastIP(uint8_t *broadcast_ip, const uint8_t *ip, const uint8_t *netmask);

static void CalculateBroadcastIP(uint8_t *broadcast_ip, const uint8_t *ip, const uint8_t *netmask) {
	for(uint32_t i = 0; i < IP_ADDR_SIZE; i++) {
		broadcast_ip[i] = ip[i] | (~netmask[i]);
	}
}

void connBroadcastInit(void) {
	connDeinit(&local_broadcast_con);

	uint8_t broadcast_mac[ETH_MAC_SIZE];
	broadcast_mac[0] = ETH_BROADCAST_MAC_ADDR_1;
	broadcast_mac[1] = ETH_BROADCAST_MAC_ADDR_2;
	broadcast_mac[2] = ETH_BROADCAST_MAC_ADDR_3;
	broadcast_mac[3] = ETH_BROADCAST_MAC_ADDR_4;
	broadcast_mac[4] = ETH_BROADCAST_MAC_ADDR_5;
	broadcast_mac[5] = ETH_BROADCAST_MAC_ADDR_6;
	connSetMAC(&local_broadcast_con, broadcast_mac);

	uint8_t local_ip[IP_ADDR_SIZE];
	udpipGetLocalIP(local_ip);
	uint8_t local_netmask[NETMASK_SIZE];
	local_netmask[0] = CONNECT_LOCAL_NETMASK_1;
	local_netmask[1] = CONNECT_LOCAL_NETMASK_2;
	local_netmask[2] = CONNECT_LOCAL_NETMASK_3;
	local_netmask[3] = CONNECT_LOCAL_NETMASK_4;
	uint8_t local_broadcast_ip[IP_ADDR_SIZE];
	CalculateBroadcastIP(local_broadcast_ip, local_ip, local_netmask);
	connSetIP(&local_broadcast_con, local_broadcast_ip);
	local_broadcast_con.port = udpipGetLocalPort();
}

void connDeinit(connection_t *connection) {
	for(uint8_t i = 0; i < ETH_MAC_SIZE; i++)
		connection->mac[i] = 0x00;

	for(uint8_t i = 0; i < IP_ADDR_SIZE; i++)
		connection->ip[i] = 0;

	connection->port = 0;
}

void connSetMAC(connection_t *connection, const uint8_t *mac) {
	for(uint8_t i = 0; i < ETH_MAC_SIZE; i++)
		connection->mac[i] = mac[i];
}

void connGetMAC(uint8_t *mac, const connection_t *connection) {
	for(uint8_t i = 0; i < ETH_MAC_SIZE; i++)
		mac[i] = connection->mac[i];
}

void connSetIP(connection_t *connection, const uint8_t *ip) {
	for(uint8_t i = 0; i < IP_ADDR_SIZE; i++)
		connection->ip[i] = ip[i];
}

void connGetIP(uint8_t *ip, const connection_t *connection) {
	for(uint8_t i = 0; i < IP_ADDR_SIZE; i++)
		ip[i] = connection->ip[i];
}


