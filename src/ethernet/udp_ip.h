#ifndef UDP_IP_H
#define UDP_IP_H

#include <stdint.h>

#include "connection.h"
#include "internal_ethernet.h"
#include "eth_ip_udp_config.h"

#define UDP_ID 0x11
#define IPv4_ID 0x0800

#define IP_ADDR_SIZE 4

#define NETMASK_SIZE IP_ADDR_SIZE

#define UDP_DATA_SIZE_Pos 39
#define UDP_DATA_Pos 42

void udpipSetLocalIP(const uint8_t *ip);
void udpipGetLocalIP(uint8_t *return_ip);
void udpipSetLocalPort(uint16_t port);
uint16_t udpipGetLocalPort(void);

void udpipCreateHeader(uint8_t *udp_packet, const connection_t *udp_ip_conn, /*const unsigned char *data,*/ uint16_t data_length);

void udpipWriteData(uint8_t *udp_packet, const uint8_t *data, uint16_t data_length);

uint16_t udpipGetDataSize(const uint8_t *eth_frame);
uint16_t udpipGetData(uint8_t *output_data, const uint8_t *eth_frame);

#endif //UDP_IP_H
