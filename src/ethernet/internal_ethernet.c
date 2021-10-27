#include "internal_ethernet.h"

#include <MDR32F9Qx_config.h>
#include <MDR32F9Qx_eth.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_dma.h>

#include "../utils.h"

#define ETH_DATA_SHIFT_VALUE ETH_SEND_PREFIX_SIZE

static uint8_t local_mac[ETH_MAC_SIZE];

static void PrepareDataForSend(ethFrame_t *eth_frame);
static void ShiftDataToRight(ethFrame_t *eth_frame, uint32_t shift_value);
static void PutPrefix(ethFrame_t *eth_frame);
static uint32_t ReceiveFrameLinearMode(uint32_t *frame);

static void PutPrefix(ethFrame_t *eth_frame) {
	eth_frame->data[0] = eth_frame->data_size;
	eth_frame->data[1] = eth_frame->data_size >> 8;
	eth_frame->data[2] = 0x00;
	eth_frame->data[3] = 0x00;
}

static void ShiftDataToRight(ethFrame_t *eth_frame, uint32_t shift_value) {
	for(int32_t i = eth_frame->data_size - 1; i >= 0; i--) {
		eth_frame->data[i + shift_value] = eth_frame->data[i];
	}
}

static void PrepareDataForSend(ethFrame_t *eth_frame) {
	ShiftDataToRight(eth_frame, ETH_DATA_SHIFT_VALUE);
	PutPrefix(eth_frame);
}

static uint32_t ReceiveFrameLinearMode(uint32_t *frame) {
	ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;
	uint32_t status;
	uint32_t PacketLength, i, Rhead, EthBaseBufferAddr, * ptr_InputFrame, tmp;
	int32_t EthReceiverFreeBufferSize;
  	uint32_t RHead_res;

	Rhead = MDR_ETHERNET1->ETH_R_Head;
	/* Set pointer to the status word reading message */
	ptr_InputFrame = (uint32_t *)((EthBaseBufferAddr = ((uint32_t)MDR_ETHERNET1) + 0x08000000) + Rhead);
	/* Read the status of the receiving a packet */
	ETH_StatusPacketReceptionStruct.Status = (uint32_t)*ptr_InputFrame++;
	PacketLength = (ETH_StatusPacketReceptionStruct.Fields.Length + 3)/4;
	/* Read the input frame */
	EthReceiverFreeBufferSize = (uint32_t) (MDR_ETHERNET1->ETH_Dilimiter - Rhead) - PacketLength*4;
	if(ETH_StatusPacketReceptionStruct.Fields.Length <= ETH_FRAME_SIZE) {
		if(EthReceiverFreeBufferSize > 0) {
			/* Read the input frame */
			for(i = 0; i < PacketLength; i++) {
				frame[i] = *ptr_InputFrame++;
			}
		} else {
			/* Read the the first part of the packet */
			for(i = 0; i < (PacketLength*4 + EthReceiverFreeBufferSize)/4 - 1; i++) {
				frame[i] = *ptr_InputFrame++;
			}
			/* Set pointer to second part of the packet */
			ptr_InputFrame = (uint32_t *)EthBaseBufferAddr;
			EthReceiverFreeBufferSize = 0 - EthReceiverFreeBufferSize;
			tmp = i;
			/* Read the the second part of the packet */
			for( i = 0; i < (EthReceiverFreeBufferSize/4) + 1; i++) {
				frame[tmp + i] = *ptr_InputFrame++;
			}
		}
		/* Set the new value of the ETH_R_Head register */
		RHead_res = ((uint32_t)ptr_InputFrame)&0x1FFF;
		status = ETH_StatusPacketReceptionStruct.Status;
	} else { //Discard received frame if its size greater then ETH_FRAME_SIZE.
		if(EthReceiverFreeBufferSize > 0) {
			RHead_res = ((uint32_t)ptr_InputFrame + PacketLength * 4) & 0x1FFF;
		} else {
			EthReceiverFreeBufferSize = 0 - EthReceiverFreeBufferSize;
			RHead_res = (EthBaseBufferAddr + (((EthReceiverFreeBufferSize + 3) / 4) * 4) + 4) & 0x1FFF;
		}
		status = 0;
	}

	if (RHead_res < MDR_ETHERNET1->ETH_Dilimiter) {
		MDR_ETHERNET1->ETH_R_Head = RHead_res;
	} else {
		MDR_ETHERNET1->ETH_R_Head = 0;
	}

	return status;
}

void ethSetLocalMAC(const uint8_t *mac) {
	for(uint32_t i = 0; i < ETH_MAC_SIZE; i++) {
		local_mac[i] = mac[i];
	}
}

void ethGetLocalMAC(uint8_t *return_mac) {
	for(uint32_t i = 0; i < ETH_MAC_SIZE; i++) {
		return_mac[i] = local_mac[i];
	}
}

void ethCreateHeader(uint8_t *header, const uint8_t *dist_mac_addr, uint16_t ether_type) {
    for(uint32_t i = 0; i < ETH_MAC_SIZE; i++)
		header[i] = dist_mac_addr[i]; //Destination MAC

	//Source MAC
	header[6] = local_mac[0];
	header[7] = local_mac[1];
	header[8] = local_mac[2];
	header[9] = local_mac[3];
	header[10] = local_mac[4];
	header[11] = local_mac[5];

	//EtherType
	header[12] = (uint8_t)(ether_type >> 8);
	header[13] = (uint8_t)ether_type;
}

void ethPutPayload(uint8_t *frame, const uint8_t *payload, uint16_t pl_size) {
	for(uint16_t i = ETH_PAYLOAD_POS; i < ETH_PAYLOAD_POS + pl_size; i++)
		frame[i] = payload[i - ETH_PAYLOAD_POS];
}

void ethSend(ethFrame_t *eth_frame) {
	PrepareDataForSend(eth_frame);
	ETH_SendFrame(MDR_ETHERNET1, (uint32_t *)eth_frame->data, eth_frame->data_size);
}

ETH_StatusPacketReceptionTypeDef ethReceive(ethFrame_t *eth_frame) {
	utlClearArray(eth_frame->data, ETH_FRAME_SIZE);
	eth_frame->data_size = 0;

	ETH_StatusPacketReceptionTypeDef status_packet_reception;
	status_packet_reception.Status = ReceiveFrameLinearMode((uint32_t *)eth_frame->data);
	eth_frame->data_size = status_packet_reception.Fields.Length;
	return status_packet_reception;
}

