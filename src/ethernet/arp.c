#include "arp.h"
#include "internal_ethernet.h"
#include "udp_ip.h"
#include "connection.h"

uint16_t arpGetPacketOpcode(uint8_t *frame_data) {
	return (uint16_t)((frame_data[20] << 8) | frame_data[21]);
}

void arpCreateBroadcastRequest(uint8_t *request_frame, const uint8_t *target_ip) {
	uint8_t local_ip[IP_ADDR_SIZE];
	udpipGetLocalIP(local_ip);

	uint8_t local_mac[ETH_MAC_SIZE];
	ethGetLocalMAC(local_mac);

	//Destination MAC
	request_frame[0] = ETH_BROADCAST_MAC_ADDR_1;
	request_frame[1] = ETH_BROADCAST_MAC_ADDR_2;
	request_frame[2] = ETH_BROADCAST_MAC_ADDR_3;
	request_frame[3] = ETH_BROADCAST_MAC_ADDR_4;
	request_frame[4] = ETH_BROADCAST_MAC_ADDR_5;
	request_frame[5] = ETH_BROADCAST_MAC_ADDR_6;

	//Source MAC
	request_frame[6] = local_mac[0];
	request_frame[7] = local_mac[1];
	request_frame[8] = local_mac[2];
	request_frame[9] = local_mac[3];
	request_frame[10] = local_mac[4];
	request_frame[11] = local_mac[5];

	//eth type (ARP)
	request_frame[12] = (ARP_ID >> 8);
	request_frame[13] = (uint8_t)ARP_ID;

	//hardware type (Ethernet)
	request_frame[14] = (ETH_ID >> 8);
	request_frame[15] = (uint8_t)ETH_ID;

	//network protocol type (IPv4)
	request_frame[16] = (IPv4_ID >> 8);
	request_frame[17] = (uint8_t)IPv4_ID;

	//MAC size (6)
	request_frame[18] = ETH_MAC_SIZE;

	//IP size (4)
	request_frame[19] = IP_ADDR_SIZE;

	//opcode
	request_frame[20] = (ARP_REQUEST_OPCODE >> 8);
	request_frame[21] = (uint8_t)ARP_REQUEST_OPCODE;

	//Source MAC
	request_frame[22] = local_mac[0];
	request_frame[23] = local_mac[1];
	request_frame[24] = local_mac[2];
	request_frame[25] = local_mac[3];
	request_frame[26] = local_mac[4];
	request_frame[27] = local_mac[5];

	//Source IP
	request_frame[28] = local_ip[0];
	request_frame[29] = local_ip[1];
	request_frame[30] = local_ip[2];
	request_frame[31] = local_ip[3];

	//Destination MAC
	request_frame[32] = 0x00;
	request_frame[33] = 0x00;
	request_frame[34] = 0x00;
	request_frame[35] = 0x00;
	request_frame[36] = 0x00;
	request_frame[37] = 0x00;

	//Destination IP
	request_frame[38] = target_ip[0];
	request_frame[39] = target_ip[1];
	request_frame[40] = target_ip[2];
	request_frame[41] = target_ip[3];
}

void arpCreateReply(uint8_t *reply_frame, const uint8_t *received_frame_data) {
	uint8_t local_ip[IP_ADDR_SIZE];
	udpipGetLocalIP(local_ip);

	uint8_t local_mac[ETH_MAC_SIZE];
	ethGetLocalMAC(local_mac);

	//Destination MAC
	reply_frame[0] = received_frame_data[6];
	reply_frame[1] = received_frame_data[7];
	reply_frame[2] = received_frame_data[8];
	reply_frame[3] = received_frame_data[9];
	reply_frame[4] = received_frame_data[10];
	reply_frame[5] = received_frame_data[11];

	//Source MAC
	reply_frame[6] = local_mac[0];
	reply_frame[7] = local_mac[1];
	reply_frame[8] = local_mac[2];
	reply_frame[9] = local_mac[3];
	reply_frame[10] = local_mac[4];
	reply_frame[11] = local_mac[5];

	//Service data from received frame.
	for(uint16_t i = 12; i < 20; i++)
		reply_frame[i] = received_frame_data[i];

	//Opcode. 0x0002 for answer
	reply_frame[20] = (ARP_REPLY_OPCODE >> 8); //0x00
	reply_frame[21] = (uint8_t)ARP_REPLY_OPCODE;//0x02;

	//Source MAC
	reply_frame[22] = local_mac[0];
	reply_frame[23] = local_mac[1];
	reply_frame[24] = local_mac[2];
	reply_frame[25] = local_mac[3];
	reply_frame[26] = local_mac[4];
	reply_frame[27] = local_mac[5];

	//Source IP
	reply_frame[28] = local_ip[0];
	reply_frame[29] = local_ip[1];
	reply_frame[30] = local_ip[2];
	reply_frame[31] = local_ip[3];

	//Distenation MAC
	reply_frame[32] = received_frame_data[6];
	reply_frame[33] = received_frame_data[7];
	reply_frame[34] = received_frame_data[8];
	reply_frame[35] = received_frame_data[9];
	reply_frame[36] = received_frame_data[10];
	reply_frame[37] = received_frame_data[11];

	//Dustenation IP
	reply_frame[38] = received_frame_data[28];
	reply_frame[39] = received_frame_data[29];
	reply_frame[40] = received_frame_data[30];
	reply_frame[41] = received_frame_data[31];
}

