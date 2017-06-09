// udp.c

#include <stdio.h>

#include "Utils.h"
#include "lan.h"
#include "udp.h"
#include "arp.h"

// UDP

udp_packet_t* ip_get_udp(ip_packet_t* ip_pak)
{
	return (udp_packet_t*) ip_pak->data;
}

u8* udp_get_data(udp_packet_t* udp)
{
	return udp->data;
}

u16 udp_get_from_port(udp_packet_t* udp)
{
	return swap2(udp->_from_port);
}
void udp_set_from_port(udp_packet_t* udp, u16 value)
{
	udp->_from_port = swap2(value);
}

u16 udp_get_to_port(udp_packet_t* udp)
{
	return swap2(udp->_to_port);
}
void udp_set_to_port(udp_packet_t* udp, u16 value)
{
	udp->_to_port = swap2(value);
}

u16 udp_get_len(udp_packet_t* udp)
{
	return swap2(udp->_len);
}
void udp_set_len(udp_packet_t* udp, u16 value)
{
	udp->_len = swap2(value);
}

u16 udp_get_cksum(udp_packet_t* udp)
{
	return swap2(udp->_cksum);
}
void udp_set_cksum(udp_packet_t* udp, u16 value)
{
	udp->_cksum = swap2(value);
}

void udp_filter(eth_frame_t* eth, u16 len)
{
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	if (len >= sizeof(udp_packet_t))
	{
		udp_packet(eth, udp_get_len(udp) - sizeof(udp_packet_t));
	}
}

void udp_reply(eth_frame_t* eth, u16 len)
{
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

//	printf("from: %u\tto: %u\n", (u32) udp_get_from_port(udp), (u32) udp_get_to_port(udp));
	u16 temp = udp_get_from_port(udp);
	udp_set_from_port(udp, udp_get_to_port(udp));
	udp_set_to_port(udp, temp);
//	printf("from: %u\tto: %u\n", (u32) udp_get_from_port(udp), (u32) udp_get_to_port(udp));

	ip_set_to_addr(ip, ip_get_from_addr(ip));

	udp_send(eth, len);
	return;
}

u8 udp_send(eth_frame_t* eth, u16 len)
{
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	len += sizeof(udp_packet_t);

	ip->protocol = IP_PROTOCOL_UDP;
	ip_set_from_addr(ip, NetInfo.ip_addr);

	udp_set_len(udp, len);
	udp_set_cksum(udp, 0);

	udp_set_cksum(udp, ip_cksum(len + IP_PROTOCOL_UDP, (u8*) udp - 8, len + 8));

	u8 res = ip_send(eth, len);
	return res;
}

// UDP PORT HANDLERS ///////////////////////////////////////////////////////////
typedef struct UdpPortHandler TUdpPortHandler;
typedef TUdpPortHandler* PUdpPortHandler;
struct UdpPortHandler
{
	u16 port;
	PUdpPortProc proc;
	PUdpPortHandler next;
};

PUdpPortHandler UdpHandler = null;

void AddUDPHandler(u16 port, PUdpPortProc proc)
{
	printf("AddUDPHandler, port = %u\t", (u32) port);
	PUdpPortHandler udph, newh;

	newh = getmem(sizeof(struct UdpPortHandler), "UdpPortHandler");
	newh->port = port;
	newh->proc = proc;
	newh->next = null;

	if (UdpHandler == null )
	{
		UdpHandler = newh;
	}
	else
	{
		udph = UdpHandler;
		while (udph->next != null )
		{
			udph = udph->next;
		}
		udph->next = newh;
	}
}

PUdpPortProc FindUdpHandler(u16 port)
{
	PUdpPortHandler udph = UdpHandler;

	while (udph != null )
	{
		if (udph->port == port)
		{
			return udph->proc;
		}
		udph = udph->next;
	}
	return null ;
}

// Обработчик получаемых UDP-пакетов
void udp_packet(eth_frame_t* eth, u16 len)
{
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);
	u16 port = udp_get_to_port(udp);
	PUdpPortProc proc;

//	printf("UDP port %u ...\n", (u32) port);

	proc = FindUdpHandler(port);

	if (proc != null )
	{
		proc(eth, len);
	}
}

/*	
 switch(port)
 {
 case NTP_LOCAL_PORT:	// Получили ответ от NTP-сервера?
 msgn("Получили ответ от NTP-сервера?");

 if((timestamp = ntp_parse_reply(udp_get_ntp(udp), len)))
 {
 msgn("NTP- OK! Запоминаем время!");
 // Запоминаем время
 time_offset = timestamp - second_count;

 // Следующее обновление через 12 часов
 ntp_next_update = second_count + 12UL*60*60;
 }
 break;
 case 12345:
 data = udp_get_data(udp);

 msgn("############  udp_packet  ###############");

 for(i = 0; i < len; ++i)
 {
 putchar(data[i]);
 }

 for(i = 0; i < 10; ++i)
 {
 data[i] = i + '0';
 }
 data[10] = '\n';
 data[11] = '\r';
 udp_reply(eth, 12);
 break;
 }
 */

