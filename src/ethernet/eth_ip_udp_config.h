#ifndef ETH_IP_UDP_CONFIG_H
#define ETH_IP_UDP_CONFIG_H

#define ETH_HEADER_SIZE	14
#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE	8

#define ETH_IP_UDP_HEADER_SIZE 	(ETH_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE)//42 // UDP/IP standard header size. Ethernet (14 bytes) + IP (20 bytes) + UDP (8 bytes)
#define UDP_DATA_SIZE_MAX 42 //UDP data size in bytes.
#if UDP_DATA_SIZE_MAX >= 500
	#error "UDP data size must be less then 500 bytes in case of preventing of packet fragmenting."
#endif

#define ETH_SEND_PREFIX_SIZE 4 //Ethernet frame prefix. This is 1986VE1T feature.
#define ETH_FRAME_SIZE (ETH_SEND_PREFIX_SIZE + ETH_IP_UDP_HEADER_SIZE + UDP_DATA_SIZE_MAX)
#if (ETH_FRAME_SIZE % 4) != 0
	#error "Full Ethernet frame size must be a multiple of 4."
#endif

//MAC addres of a MCU.
#define ETH_LOCAL_MAC_ADDR_1 0x01
#define ETH_LOCAL_MAC_ADDR_2 0x23
#define ETH_LOCAL_MAC_ADDR_3 0x45
#define ETH_LOCAL_MAC_ADDR_4 0x67
#define ETH_LOCAL_MAC_ADDR_5 0x89
#define ETH_LOCAL_MAC_ADDR_6 0xAB

//IP address of a MCU.
#define IP_LOCAL_ADDR_1 192
#define IP_LOCAL_ADDR_2 168
#define IP_LOCAL_ADDR_3 0
#define IP_LOCAL_ADDR_4 10

//UDP port of a MCU.
#define UDP_LOCAL_PORT 3000

//Broadcast MAC address.
#define ETH_BROADCAST_MAC_ADDR_1 0xFF
#define ETH_BROADCAST_MAC_ADDR_2 0xFF
#define ETH_BROADCAST_MAC_ADDR_3 0xFF
#define ETH_BROADCAST_MAC_ADDR_4 0xFF
#define ETH_BROADCAST_MAC_ADDR_5 0xFF
#define ETH_BROADCAST_MAC_ADDR_6 0xFF

//Network mask.
#define CONNECT_LOCAL_NETMASK_1 ((uint8_t)255)
#define CONNECT_LOCAL_NETMASK_2 ((uint8_t)255)
#define CONNECT_LOCAL_NETMASK_3 ((uint8_t)255)
#define CONNECT_LOCAL_NETMASK_4 ((uint8_t)0)

#endif //ETH_IP_UDP_CONFIG_H
