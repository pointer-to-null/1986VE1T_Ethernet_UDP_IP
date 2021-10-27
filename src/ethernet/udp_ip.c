#include "udp_ip.h"
#include "internal_ethernet.h"
#include "ethio.h"
#include "connection.h"

static uint8_t local_ip[IP_ADDR_SIZE];
static uint16_t local_port;

static uint16_t IPCheckSum(uint8_t *Dt);

static uint16_t IPCheckSum(uint8_t *Dt) {
	unsigned long temp, Check = 0;
	for(temp = 0; temp < 20; temp += 2)	{
		if(temp == 10) continue;
		else Check += ((Dt[temp + 14] << 8)&0xFF00)|Dt[temp + 15];
	}
	Check = (Check >> 16) + (Check & 0xFFFF);
	return (uint16_t)(~Check);
}

void udpipSetLocalIP(const uint8_t *ip) {
	for(uint32_t i = 0; i < IP_ADDR_SIZE; i++) {
		local_ip[i] = ip[i];
	}
}

void udpipGetLocalIP(uint8_t *return_ip) {
	for(uint32_t i = 0; i < IP_ADDR_SIZE; i++) {
		return_ip[i] = local_ip[i];
	}
}

void udpipSetLocalPort(uint16_t port) {
	local_port = port;
}

uint16_t udpipGetLocalPort(void) {
	return local_port;
}

void udpipCreateHeader(uint8_t *udp_packet, const connection_t *udp_ip_conn,/* const unsigned char *data,*/ uint16_t data_length) {

	//IP-header
	udp_packet[14] = 0x45; //version, IHL
	udp_packet[15] = 0x00; //DSCP, ECN

	//Total length
	uint16_t udp_ip_size = IP_HEADER_SIZE + UDP_HEADER_SIZE + data_length;
	udp_packet[16] = (uint8_t)(udp_ip_size >> 8);
	udp_packet[17] = (uint8_t)udp_ip_size;

	//[18...19] Identification
	//[20...21] Flags, Fragmet offset
	//[22] Time to live (128)
	//[23] Protocol (UDP = 0x11)
	udp_packet[18] = 0x00;
	udp_packet[19] = 0x01;
	udp_packet[20] = 0x00;
	udp_packet[21] = 0x00;
	udp_packet[22] = 0x80;
	udp_packet[23] = UDP_ID;

	//Source IP
	udp_packet[26] = local_ip[0];
	udp_packet[27] = local_ip[1];
	udp_packet[28] = local_ip[2];
	udp_packet[29] = local_ip[3];

	//Destination IP
	udp_packet[30] = udp_ip_conn->ip[0];
	udp_packet[31] = udp_ip_conn->ip[1];
	udp_packet[32] = udp_ip_conn->ip[2];
	udp_packet[33] = udp_ip_conn->ip[3];

	uint16_t ip_cs = IPCheckSum(udp_packet);

	//IP header checksum
	udp_packet[24] = (uint8_t)(ip_cs >> 8);
	udp_packet[25] = (uint8_t)ip_cs;

	//UDP-header
	//Source port
	udp_packet[34] = (uint8_t)(local_port >> 8);
	udp_packet[35] = (uint8_t)local_port;

	//Destination port
	udp_packet[36] = (uint8_t)(udp_ip_conn->port >> 8);
	udp_packet[37] = (uint8_t)udp_ip_conn->port;

	//Length
	uint16_t udp_length = data_length + UDP_HEADER_SIZE;
	udp_packet[38] = (uint8_t)(udp_length >> 8);
	udp_packet[39] = (uint8_t)udp_length;

	udp_packet[40] = 0x00;	  // UDP Checksum
	udp_packet[41] = 0x00;
}

void udpipWriteData(uint8_t *udp_packet, const uint8_t *data, const uint16_t data_length) {
	for(uint16_t i = ETH_IP_UDP_HEADER_SIZE; i < data_length + ETH_IP_UDP_HEADER_SIZE; i++) {
		udp_packet[i] = data[i - ETH_IP_UDP_HEADER_SIZE];
	}
}

uint16_t udpipGetDataSize(const uint8_t *eth_frame) {
	uint16_t data_size = (((uint16_t)eth_frame[38] << 8) | eth_frame[39]) - UDP_HEADER_SIZE;
	return data_size;
}

uint16_t udpipGetData(uint8_t *output_data, const uint8_t *eth_frame) {
	uint16_t data_size = udpipGetDataSize(eth_frame);
	for(uint16_t i = 0; i < data_size; i++)
		output_data[i] = eth_frame[UDP_DATA_Pos + i];
	return data_size;
}

