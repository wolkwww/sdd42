// arp.h

#ifndef _ARP_H
#define _ARP_H

#include <stdio.h>
#include "common.h"

//#define ARP_HW_TYPE_ETH		htons(0x0001)
//#define ARP_PROTO_TYPE_IP	htons(0x0800)
#define ARP_HW_TYPE_ETH		(0x0001)
#define ARP_PROTO_TYPE_IP	(0x0800)

//#define ARP_TYPE_REQUEST	htons(1)
//#define ARP_TYPE_RESPONSE	htons(2)
#define ARP_TYPE_REQUEST	(1)
#define ARP_TYPE_RESPONSE	(2)

typedef struct arp_message {
	u16 	_hw_type			__attribute__((packed));	// 2
	u16 	_proto_type			__attribute__((packed));	// 2
	u8 		hw_addr_len;									// 1
	u8 		proto_addr_len;									// 1
	u16 	_type				__attribute__((packed));	// 2
	u8 		mac_addr_from[6];								// 6
	u32 	_ip_addr_from		__attribute__((packed));	// 4
	u8 		mac_addr_to[6];									// 6
	u32 	_ip_addr_to			__attribute__((packed));	// 4
} arp_message_t;

arp_message_t* eth_get_arp(eth_frame_t *frame);

void showARP(void);

u16 	arp_get_hw_type(arp_message_t* arp);
u16 	arp_get_proto_type(arp_message_t* arp);
u16 	arp_get_type(arp_message_t* arp);
u32 	arp_get_ip_addr_from(arp_message_t* arp);
u32 	arp_get_ip_addr_to(arp_message_t* arp);

void 	arp_filter(eth_frame_t *frame, u16 len);
u8 		ip_send(eth_frame_t* eth, u16 len);

#endif

