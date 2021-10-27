#include <MDR32F9Qx_config.h>
#include <MDR32F9Qx_eth.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include <MDR32F9Qx_port.h>

#include "./ethernet/internal_ethernet.h"
#include "./ethernet/arp.h"
#include "./ethernet/udp_ip.h"
#include "./ethernet/eth_ip_udp_config.h"
#include "./ethernet/ethio.h"
#include "utils.h"

//Core frequency: 104 MHz (8 * 13)
static void ClkInit(void) {
	RST_CLK_DeInit();
	RST_CLK_PCLKcmd(RST_CLK_PCLK_RST_CLK, ENABLE);
	RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	while(RST_CLK_HSEstatus() != SUCCESS);
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul13);
	RST_CLK_CPU_PLLcmd(ENABLE);
	while(RST_CLK_CPU_PLLstatus() != SUCCESS);

	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
	EEPROM_SetLatency(EEPROM_Latency_5);

	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);
	RST_CLK_CPU_PLLuse(ENABLE);
  	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);
	SystemCoreClockUpdate();
}

static void EthInit(const uint8_t *mac) {
    ETH_ClockDeInit();
    RST_CLK_HSE2config(RST_CLK_HSE2_ON);
	if(RST_CLK_HSE2status() == ERROR)
    	while(1);
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);

	ETH_BRGInit(ETH_HCLKdiv1);
	ETH_ClockCMD(ETH_CLK1, ENABLE);

	ETH_InitTypeDef ETH_InitStruct;
	ETH_DeInit(MDR_ETHERNET1);
	ETH_StructInit((ETH_InitTypeDef *) &ETH_InitStruct);
	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_LINEAR; //NOTE: Only linear mode supported.
	ETH_InitStruct.ETH_Source_Addr_HASH_Filter = DISABLE;
	ETH_InitStruct.ETH_MAC_Address[2] = (mac[5] << 8) | mac[4];
	ETH_InitStruct.ETH_MAC_Address[1] = (mac[3] << 8) | mac[2];
	ETH_InitStruct.ETH_MAC_Address[0] = (mac[1] << 8) | mac[0];
	ETH_InitStruct.ETH_Dilimiter = 0x1000;
	ETH_Init(MDR_ETHERNET1, (ETH_InitTypeDef *) &ETH_InitStruct);
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);
	ETH_Start(MDR_ETHERNET1);
}

static void Eth_UDP_IP_init(void) {
	uint8_t mac[ETH_MAC_SIZE] = {ETH_LOCAL_MAC_ADDR_1, ETH_LOCAL_MAC_ADDR_2, ETH_LOCAL_MAC_ADDR_3,
								ETH_LOCAL_MAC_ADDR_4, ETH_LOCAL_MAC_ADDR_5, ETH_LOCAL_MAC_ADDR_6};
	ethSetLocalMAC(mac);
	EthInit(mac);

	uint8_t ip[IP_ADDR_SIZE] = {IP_LOCAL_ADDR_1, IP_LOCAL_ADDR_2, IP_LOCAL_ADDR_3, IP_LOCAL_ADDR_4};
	udpipSetLocalIP(ip);
	udpipSetLocalPort(UDP_LOCAL_PORT);
}

static uint32_t SelfBroadcast(ethFrame_t *eth_frame, uint8_t *local_ip) {
	return ((eth_frame->data[26] == local_ip[0]) &&
			(eth_frame->data[27] == local_ip[1]) &&
			(eth_frame->data[28] == local_ip[2]) &&
			(eth_frame->data[29] == local_ip[3]));
}

static void HandleReceivedFrame(ethFrame_t *eth_rx_frame) {
	uint16_t network_packet_type = ((uint16_t)eth_rx_frame->data[12] << 8) | eth_rx_frame->data[13];
	uint8_t local_ip[IP_ADDR_SIZE] = {[0] = 0};
	udpipGetLocalIP(local_ip);
	switch(network_packet_type) {
		case ARP_ID: {
			if((eth_rx_frame->data[38] == local_ip[0]) &&
			(eth_rx_frame->data[39] == local_ip[1]) &&
			(eth_rx_frame->data[40] == local_ip[2]) &&
			(eth_rx_frame->data[41] == local_ip[3])) { //if target IP in incoming packet equal to local IP
				uint16_t arp_opcode = arpGetPacketOpcode(eth_rx_frame->data);
				switch(arp_opcode) {
					case ARP_REQUEST_OPCODE: {
						uint8_t arp_reply[ARP_PACKET_SIZE] = {[0] = 0};
						arpCreateReply(arp_reply, eth_rx_frame->data);
						SendEthARP(arp_reply);
					} break;
					case ARP_REPLY_OPCODE: {
						//TODO: Send ARP request to remote host and process incoming ARP reply to resolve MAC address.
					} break;
					default: {
					} break;
				}
			}
		} break;
		case IPv4_ID: {
			if(SelfBroadcast(eth_rx_frame, local_ip) == TRUE) { //discard self broadcast frame
				break;
			}
			uint8_t transport_proto_type = eth_rx_frame->data[23];
			switch(transport_proto_type) {
				case UDP_ID: {
					uint16_t udp_port = ((uint16_t)eth_rx_frame->data[36] << 8) | eth_rx_frame->data[37];
					uint16_t udp_data_size = udpipGetDataSize(eth_rx_frame->data);
					if(udp_data_size > UDP_DATA_SIZE_MAX) {
						break;
					}
					uint8_t udp_data[UDP_DATA_SIZE_MAX] = {[0] = 0};
					udpipGetData(udp_data, eth_rx_frame->data);
					switch(udp_port) {
						case UDP_LOCAL_PORT: {
							connection_t remote_connection = { //Srtuct which contains addresses and port of the remote host.
								.mac = {0xAB, 0x45, 0x38, 0xCC, 0xEF, 0x12},
								.ip = {192, 168, 0, 50},
								.port = 4000,
							};
							static uint32_t packet_count = 0;

							//Sending formated string to remote host via UDP/IP.
							eprintf(&remote_connection, "rx: %s pc: %u\n", udp_data, packet_count++);

							//Alternative way to send raw data to remote host via UDP/IP.
							/*uint8_t arr[4] = {0x11, 0x22, 0x33, 0x44};
							DirectSendDataEthUDP(&remote_connection, arr, 4);*/
						} break;
						default: {
						} break;
					}
				} break;
				default: {
				} break;
			}
		} break;
		default: {
		} break;
	}
}

int main(void) {
	ClkInit();

	Eth_UDP_IP_init();

	ethFrame_t rx_frame;
	ETH_StatusPacketReceptionTypeDef rx_frame_status;

	while(1) {
		if(MDR_ETHERNET1->ETH_R_Head != MDR_ETHERNET1->ETH_R_Tail) { //If receiver buffer contains incoming packet.
			rx_frame_status = ethReceive(&rx_frame); //Read incoming packet.
			if((rx_frame_status.Fields.BCA || rx_frame_status.Fields.UCA) && (rx_frame_status.Fields.Length != 0)) { //If received broadcast frame or frame with local MAC address.
				HandleReceivedFrame(&rx_frame);
			}
		}
	}
}

