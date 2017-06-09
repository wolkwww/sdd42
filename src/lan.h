//lan.h

#ifndef _LAN_H_
#define _LAN_H_

#include <string.h>
#include <stdio.h>

#include "enc28j60.h"
#include "common.h"

// Config

#define MAC_ADDR1			{0x00,0x13,0x37,0x01,0x27,0x45}
#define MAC_ADDR2			{0x00,0x13,0x37,0x01,0x27,0x46}

#define IP_ADDR0			inet_addr(192,168,0,42)
#define IP_ADDR1			inet_addr(192,168,0,221)
#define IP_ADDR2			inet_addr(192,168,0,222)

#define IP_MASK 			inet_addr(255,255,255,0)
#define IP_GATEWAY 			inet_addr(192,168,0,254)

#define	S42_LOCAL_PORT		42424

//#define	DEFAULT_SERVER_IP 	inet_addr(192,168,0,198)
#define	DEFAULT_SERVER_IP 	inet_addr(192,168,0,254)

extern u32 server_ip;

typedef struct
{
	u32 ip_addr;
	u32 ip_mask;
	u32 ip_gateway;
	u8 mac_addr[6];
} NetInfoDef;

extern NetInfoDef NetInfo;

//extern u8* mac_addr;
//extern u32 ip_addr;
//extern u32 ip_mask;
//extern u32 ip_gateway;

//extern u8 net_buf[ENC28J60_MAXFRAME];
extern u8* net_buf;

#define IP_PACKET_TTL		64

// Ethernet
 
//#define ETH_TYPE_ARP		htons(0x0806)
//#define ETH_TYPE_IP			htons(0x0800)
#define ETH_TYPE_ARP		(0x0806)
#define ETH_TYPE_IP			(0x0800)

typedef struct eth_frame 
{
	u8 	to_addr[6];
	u8 	from_addr[6];
	u16 _type			__attribute__((packed));	// 2
	u8 data[];
} eth_frame_t;

u16		eth_get_type(eth_frame_t* eth);
void 	eth_set_type(eth_frame_t* eth, u16 type);
void	eth_set_from_addr(eth_frame_t* eth, u8* mac);
void	eth_set_to_addr(eth_frame_t* eth, u8* mac);
void	eth_fill_to_addr(eth_frame_t* eth, u8 fill_pattern);

// IP

#define IP_PROTOCOL_ICMP	1
#define IP_PROTOCOL_TCP		6
#define IP_PROTOCOL_UDP		17

typedef struct ip_packet
{
	u8 		ver_head_len;
	u8 		tos;
	u16 	_total_len				__attribute__((packed));	//
	u16 	_fragment_id			__attribute__((packed));	//
	u16 	_flags_framgent_offset	__attribute__((packed));	//
	u8 		ttl;
	u8 		protocol;
	u16 	_cksum					__attribute__((packed));	//
	u32 	_from_addr				__attribute__((packed));	//
	u32 	_to_addr				__attribute__((packed));	//
	u8 		data[];
} ip_packet_t;


u16 	ip_get_total_len(ip_packet_t* ip_pak);
void	ip_set_total_len(ip_packet_t* ip_pak, u16 value);
u16 	ip_get_fragment_id(ip_packet_t* ip_pak);
void 	ip_set_fragment_id(ip_packet_t* ip_pak, u16 value);
u16 	ip_get_flags_framgent_offset(ip_packet_t* ip_pak);
void	ip_set_flags_framgent_offset(ip_packet_t* ip_pak, u16 value);
u16 	ip_get_cksum(ip_packet_t* ip_pak);
void	ip_set_cksum(ip_packet_t* ip_pak, u16 value);
u32 	ip_get_from_addr(ip_packet_t* ip_pak);
void	ip_set_from_addr(ip_packet_t* ip_pak, u32 value);
u32 	ip_get_to_addr(ip_packet_t* ip_pak);
void 	ip_set_to_addr(ip_packet_t* ip_pak, u32 value);

// UDP
typedef struct _udp_packet
{
	u16 	_from_port	__attribute__((packed));	//
	u16 	_to_port	__attribute__((packed));	//
	u16 	_len		__attribute__((packed));	//
	u16 	_cksum		__attribute__((packed));	//
	u8 		data[];
} udp_packet_t;

// ICMP

#define ICMP_TYPE_ECHO_RQ	8
#define ICMP_TYPE_ECHO_RPLY	0

typedef struct icmp_echo_packet
{
	u8 		type;
	u8 		code;
	u16 	_cksum		__attribute__((packed));	//
	u16 	_id			__attribute__((packed));	//
	u16 	_seq		__attribute__((packed));	//
	u8 		data[];
} icmp_echo_packet_t;

u16 	icmp_get_cksum(icmp_echo_packet_t* icmp);
void	icmp_set_cksum(icmp_echo_packet_t* icmp, u16 value);

u16 	icmp_get_id(icmp_echo_packet_t* icmp);
u16 	icmp_get_seq(icmp_echo_packet_t* icmp);

// LAN

void 	lan_init(void);
void 	lan_poll(void);

void 	eth_filter(eth_frame_t* eth, u16 len);

void 	eth_send(eth_frame_t* eth, u16 len);
void 	eth_reply(eth_frame_t* eth, u16 len);
void 	ip_reply(eth_frame_t* eth, u16 len);
u16 	ip_cksum(u32 sum, u8 *buf, u32 len);


udp_packet_t* 		ip_get_udp(ip_packet_t* ip_pak);
u8*					udp_get_data(udp_packet_t* udp);
icmp_echo_packet_t* ip_get_icmp(ip_packet_t* ip_pak);
ip_packet_t* 		eth_get_ip(eth_frame_t* eth);


// UDP PORT HANDLERS

typedef void (*PUdpPortProc)(eth_frame_t* eth, u16 len);

#endif
