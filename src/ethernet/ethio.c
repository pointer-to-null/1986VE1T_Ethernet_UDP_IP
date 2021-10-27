#include "ethio.h"
#include "connection.h"
#include "internal_ethernet.h"
#include "udp_ip.h"
#include "arp.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define USE_FLOATING_POINT_FORMAT

//NOTE: This function consume a lot of memory.
void eprintf(const connection_t *eth_conn, const char *string, ...) {
	char formatted_string[UDP_DATA_SIZE_MAX];

	va_list argptr;
	va_start(argptr, string);
#if defined (USE_FLOATING_POINT_FORMAT)
	uint16_t fs_len = vsnprintf(formatted_string, UDP_DATA_SIZE_MAX, string, argptr);
#else
	uint16_t fs_len = _vsnprintf(formatted_string, UDP_DATA_SIZE_MAX, string, argptr);
#endif //USE_FLOATING_POINT_FORMAT
	va_end(argptr);

	uint8_t packet[ETH_FRAME_SIZE];
	ethCreateHeader(packet, eth_conn->mac, IPv4_ID);
	udpipCreateHeader(packet, eth_conn, fs_len);
	udpipWriteData(packet, (uint8_t *)formatted_string, fs_len);

	ethFrame_t frame;
	for(uint16_t i = 0; i < fs_len + ETH_IP_UDP_HEADER_SIZE; i++) {
		frame.data[i] = packet[i];
	}
	frame.data_size = fs_len + ETH_IP_UDP_HEADER_SIZE;
	ethSend(&frame);
}

void SendEthARP(const uint8_t *arp_frame) {
	ethFrame_t tx_frame;
	for(uint8_t i = 0; i < ARP_PACKET_SIZE; i++) {
		tx_frame.data[i] = arp_frame[i];
	}
	tx_frame.data_size = ARP_PACKET_SIZE;
	ethSend(&tx_frame);
}

void DirectSendDataEthUDP(const connection_t *eth_conn, const uint8_t *data, uint16_t data_size) {
	uint8_t tx_packet[ETH_FRAME_SIZE];
	//uint16_t string_len = strlen((char *)string);
	ethCreateHeader(tx_packet, eth_conn->mac, IPv4_ID);
	udpipCreateHeader(tx_packet, eth_conn, data_size);
	udpipWriteData(tx_packet, data, data_size);

	ethFrame_t tx_frame;
	for(uint16_t i = 0; i < ETH_IP_UDP_HEADER_SIZE + data_size; i++) {
		tx_frame.data[i] = tx_packet[i];
	}
	tx_frame.data_size = ETH_IP_UDP_HEADER_SIZE + data_size;
	ethSend(&tx_frame);
}

