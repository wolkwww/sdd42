// udp.h

#ifndef _UDP_H_
#define _UDP_H_

#include "lan.h"

// UDP

u16 	udp_get_from_port(udp_packet_t* udp);
void	udp_set_from_port(udp_packet_t* udp, u16 value);

u16 	udp_get_to_port(udp_packet_t* udp);
void	udp_set_to_port(udp_packet_t* udp, u16 value);

u16 	udp_get_len(udp_packet_t* udp);
void	udp_set_len(udp_packet_t* udp, u16 value);

u16 	udp_get_cksum(udp_packet_t* udp);
void	udp_set_cksum(udp_packet_t* udp, u16 value);

void 	udp_filter(eth_frame_t* eth, u16 len);
void 	udp_packet(eth_frame_t* eth, u16 len);
void 	udp_reply(eth_frame_t* eth, u16 len);
u8 		udp_send(eth_frame_t* eth, u16 len);

void AddUDPHandler(u16 port, PUdpPortProc proc);

#endif
