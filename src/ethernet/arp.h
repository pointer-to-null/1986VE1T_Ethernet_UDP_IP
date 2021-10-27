#ifndef ARP_H
#define ARP_H

#include <stdint.h>

#include "internal_ethernet.h"
#include "udp_ip.h"

#define ARP_ID 0x0806

#define ARP_REQUEST_OPCODE 0x0001
#define ARP_REPLY_OPCODE 0x0002

#define ARP_PACKET_SIZE 42 //Size of standart ARP answer.

void arpCreateReply(uint8_t *reply_frame, const uint8_t *received_frame_data);
uint16_t arpGetPacketOpcode(uint8_t *frame_data);
void arpCreateBroadcastRequest(uint8_t *request_frame, const uint8_t *target_ip);

#endif //ARP_H
