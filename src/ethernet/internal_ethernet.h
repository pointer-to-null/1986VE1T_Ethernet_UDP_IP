#ifndef INTERNAL_ETHERNET_H
#define INTERNAL_ETHERNET_H

#include <stdint.h>

#include <MDR32F9Qx_config.h>
#include <MDR32F9Qx_eth.h>

#include "eth_ip_udp_config.h"

#define ETH_ID 0x0001

typedef struct {
	uint8_t data[ETH_FRAME_SIZE];
	uint16_t data_size; //data size in bytes
} ethFrame_t;

#define ETH_MAC_SIZE 6

#define ETH_PAYLOAD_POS 14 //Data payload position in Ethernet frame.

void ethSetLocalMAC(const uint8_t *mac);
void ethGetLocalMAC(uint8_t *return_mac);

void ethCreateHeader(uint8_t *header, const uint8_t *dist_mac_addr, uint16_t ether_type);
void ethPutPayload(uint8_t *frame, const uint8_t *payload, uint16_t pl_size);
void ethSend(ethFrame_t *eth_frame);
ETH_StatusPacketReceptionTypeDef ethReceive(ethFrame_t *eth_frame);

#endif //INTERNAL_ETHERNET_H
