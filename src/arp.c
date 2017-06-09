// arp.c

#include <string.h>
#include "common.h"
#include "lan.h"
#include "arp.h"

// Размер ARP-кэша
#define ARP_CACHE_SIZE      8

// ARP-кэш
typedef struct arp_cache_entry
{
	u32 ip_addr;
	u8 mac_addr[6];
} arp_cache_entry_t;

u32 arp_cache_wr;
arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];

void showARP(void)
{
	msg("\tARP:\t");
	for (u32 i = 0; i < ARP_CACHE_SIZE; i++)
	{
		if (arp_cache[i].ip_addr != 0)
		{
			msg("\t(");
			prnIP(arp_cache[i].ip_addr);
			msg(" ");
			prnMAC(arp_cache[i].mac_addr);
			msg(")");
		}
	}
}

// Поиск в ARP-кэше
u8* arp_search_cache(u32 ip_addr)
{
	for (u32 i = 0; i < ARP_CACHE_SIZE; ++i)
	{
		if (arp_cache[i].ip_addr == ip_addr)
		{
			return arp_cache[i].mac_addr;
		}
	}
	return null;
}

void arp_cache_add(u32 ip, u8* mac)
{
	msgn("arp_cache_add");
	arp_cache[arp_cache_wr].ip_addr = ip;
	memcpy(arp_cache[arp_cache_wr].mac_addr, mac, 6);
	arp_cache_wr++;

	if (arp_cache_wr == ARP_CACHE_SIZE)
	{
		arp_cache_wr = 0;
	}
}

arp_message_t* eth_get_arp(eth_frame_t* eth)
{
	return (arp_message_t*) (eth->data);
}

u16 arp_get_hw_type(arp_message_t* arp)
{
	return swap2(arp->_hw_type);
}
void arp_set_hw_type(arp_message_t* arp, u16 value)
{
	arp->_hw_type = swap2(value);
}

u16 arp_get_proto_type(arp_message_t* arp)
{
	return swap2(arp->_proto_type);
}
void arp_set_proto_type(arp_message_t* arp, u16 value)
{
	arp->_proto_type = swap2(value);
}

u16 arp_get_type(arp_message_t* arp)
{
	return swap2(arp->_type);
}
void arp_set_type(arp_message_t* arp, u16 type)
{
	arp->_type = swap2(type);
}

void arp_set_mac_addr_from(arp_message_t* arp, u8* mac_addr)
{
	memcpy(arp->mac_addr_from, mac_addr, 6);
}

u32 arp_get_ip_addr_from(arp_message_t* arp)
{
	return swap4(arp->_ip_addr_from);
}
void arp_set_ip_addr_from(arp_message_t* arp, u32 addr)
{
	arp->_ip_addr_from = swap4(addr);
}

void arp_set_mac_addr_to(arp_message_t* arp, u8* mac_addr)
{
	memcpy(arp->mac_addr_to, mac_addr, 6);
}
void arp_fill_mac_addr_to(arp_message_t* arp, u8 fill_pattern)
{
	memset(arp->mac_addr_to, fill_pattern, 6);
}

u32 arp_get_ip_addr_to(arp_message_t* arp)
{
	return swap4(arp->_ip_addr_to);
}
void arp_set_ip_addr_to(arp_message_t* arp, u32 addr)
{
	arp->_ip_addr_to = swap4(addr);
}

// ARP-ресолвер
// Если MAC-адрес узла известен, возвращает его
// Неизвестен - посылает запрос и возвращает 0
u8 *arp_resolve(u32 node_ip_addr)
{
	eth_frame_t *eth = (void*) net_buf;
	arp_message_t* arp = eth_get_arp(eth);

	u8 *mac;

	// Ищем узел в кэше
	if ((mac = arp_search_cache(node_ip_addr)))
	{
		return mac;
	}

	// Отправляем запрос	
	//memset(eth->to_addr, 0xff, 6);
	eth_fill_to_addr(eth, 0xff);

	//eth->type = ETH_TYPE_ARP;
	eth_set_type(eth, ETH_TYPE_ARP);

	//arp->hw_type = ARP_HW_TYPE_ETH;
	arp_set_hw_type(arp, ARP_HW_TYPE_ETH);

	//arp->proto_type = ARP_PROTO_TYPE_IP;
	arp_set_proto_type(arp, ARP_PROTO_TYPE_IP);

	arp->hw_addr_len = 6;
	arp->proto_addr_len = 4;

	//arp->type = ARP_TYPE_REQUEST;
	arp_set_type(arp, ARP_TYPE_REQUEST);

	//memcpy(arp->mac_addr_from, mac_addr, 6);
	arp_set_mac_addr_from(arp, NetInfo.mac_addr);

	//arp->ip_addr_from = ip_addr;
	arp_set_ip_addr_from(arp, NetInfo.ip_addr);

	//memset(arp->mac_addr_to, 0x00, 6);
	arp_fill_mac_addr_to(arp, 0x00);

	//arp->ip_addr_to = node_ip_addr;
	arp_set_ip_addr_to(arp, node_ip_addr);

	msgn("\t\t\tSEND arp_resolve");
//	printf("\thw_type\t%u\n", (u32) arp_get_hw_type(arp));
//	printf("\tproto_type\t%u\n", (u32) arp_get_proto_type(arp));
//	printf("\thw_type\t%u\n", (u32) arp_get_type(arp));
//	msg("\tip_from\t"); prnIP(arp_get_ip_addr_from(arp)); CR();
//	msg("\tip_to\t"); prnIP(arp_get_ip_addr_to(arp)); CR();

	eth_send(eth, sizeof(arp_message_t));
	return null;
}

void arp_filter(eth_frame_t* eth, u16 len)
{
	arp_message_t* arp = eth_get_arp(eth);

//	printf("-----ARP-----, len = %u (%u)\n", (u32)len, (u32)sizeof(arp_message_t));
//	printf("arp->type         = %u\t(%u/%u)\n", (u32)swap2(arp->type), 		(u32)swap2(ARP_TYPE_REQUEST), (u32)swap2(ARP_TYPE_RESPONSE));
//	printf("arp->hw_type      = %u\t(%u)\n", (u32)swap2(arp->hw_type), 		(u32)swap2(ARP_HW_TYPE_ETH));
//	printf("arp->proto_type   = %u\t(%u)\n", (u32)swap2(arp->proto_type),	(u32)swap2(ARP_PROTO_TYPE_IP));
//	msg("arp->ip_addr_from = "); prnIP(arp->ip_addr_from); msgn("");
//	msg("arp->ip_addr_to   = "); prnIP(arp->ip_addr_to); msg(" ("); prnIP(ip_addr); msgn(")");

	if (len >= sizeof(arp_message_t))
	{
//		printf("arp_filter len = %u\n\n", (u32) len);
//
//		printf("hw_type\t%u\t(%u)\n", (u32) arp_get_hw_type(arp), (u32) ARP_HW_TYPE_ETH);
//		printf("proto_type\t%u\t(%u)\n", (u32) arp_get_proto_type(arp), (u32) ARP_PROTO_TYPE_IP);
//		msg("ip_to\t"); prnIP(arp_get_ip_addr_to(arp)); msg("\t"); prnIP(NetInfo.ip_addr); CR();
//		printf("arp->type\t%u\t(%u/%u)\n", (u32)arp_get_type(arp), 		(u32)ARP_TYPE_REQUEST, (u32)ARP_TYPE_RESPONSE);

		if ((arp_get_hw_type(arp) == ARP_HW_TYPE_ETH) && (arp_get_proto_type(arp) == ARP_PROTO_TYPE_IP)
		        && (arp_get_ip_addr_to(arp) == NetInfo.ip_addr))
		{
//			msg("ARP:\t");
//			printf("arp->type         = %u\t(%u/%u)\n", (u32)arp_get_type(arp), 		(u32)swap2(ARP_TYPE_REQUEST), (u32)swap2(ARP_TYPE_RESPONSE));
			switch (arp_get_type(arp))
			{
				// ARP-запрос, посылаем ответ
				case ARP_TYPE_REQUEST:
//					msg("ARP_TYPE_REQUEST FROM: ");
//					prnIP(arp->ip_addr_from);
//					msg(" (");
//					prnMAC(arp->mac_addr_from);
//					msg(")");
//					msg("\tTO: ");
//					prnIP(arp->ip_addr_to);
//					msg(" (");
//					prnMAC(arp->mac_addr_to);
//					msg(")");
//					msg("\n\rANSWER:\t");

					arp_set_type(arp, ARP_TYPE_RESPONSE);
					arp_set_mac_addr_to(arp, arp->mac_addr_from);
					arp_set_mac_addr_from(arp, NetInfo.mac_addr);
					arp_set_ip_addr_to(arp, arp_get_ip_addr_from(arp));
					arp_set_ip_addr_from(arp, NetInfo.ip_addr); //!!

//					msg("FROM: ");
//					prnIP(arp->ip_addr_from);
//					msg(" (");
//					prnMAC(arp->mac_addr_from);
//					msg(")");
//					msg("\tTO: ");
//					prnIP(arp->ip_addr_to);
//					msg(" (");
//					prnMAC(arp->mac_addr_to);
//					msgn(")");

					eth_reply(eth, sizeof(arp_message_t));

					break;

					// ARP-ответ, добавляем узел в кэш
				case ARP_TYPE_RESPONSE:
					msgn("\t\t\t\t\t\t\t\tARP_TYPE_RESPONSE");
					prnIP(arp_get_ip_addr_from(arp));
					msg(" (");
					prnMAC(arp->mac_addr_from);
					msg(")");
					msg("\tTO: ");
					prnIP(arp_get_ip_addr_to(arp));
					msg(" (");
					prnMAC(arp->mac_addr_to);
					msgn(")");

					if (!arp_search_cache(arp_get_ip_addr_from(arp)))
					{
						msgn("ARP not found!!!");
						arp_cache_add(arp_get_ip_addr_from(arp), arp->mac_addr_from);
						showARP();
						CR();
					}
					break;
			}
		}
	}
}

// Отправка IP-пакета
// Следующие поля пакета должны быть установлены:
//    ip.to_addr - адрес получателя
//    ip.protocol - код протокола
// len - длина поля данных пакета
// Если MAC-адрес узла/гейта ещё не определён, функция возвращает 0 
u8 ip_send(eth_frame_t* eth, u16 len)
{
//	printf("ip_send len = %u\n\n", (u32) len);
	ip_packet_t* ip = eth_get_ip(eth);

	u32 route_ip;
	u8* mac_addr_to;

	// Если узел в локалке, отправляем пакет ему,
	// если нет, то гейту
	if (((ip_get_to_addr(ip) ^ NetInfo.ip_addr) & NetInfo.ip_mask) == 0)
	{
		route_ip = ip_get_to_addr(ip);
	}
	else
	{
		route_ip = NetInfo.ip_gateway;
	}

	// Резолвим MAC-адрес
	if (!(mac_addr_to = arp_resolve(route_ip)))
	{
		return 0;
	}

	// Отправляем пакет
	len += sizeof(ip_packet_t); // SIZE_OF_IP_PACKET;

	//memcpy(frame->to_addr, mac_addr_to, 6);
	eth_set_to_addr(eth, mac_addr_to);

	eth_set_type(eth, ETH_TYPE_IP);
	//eth_set_type(eth, ETH_TYPE_IP);

	ip->ver_head_len = 0x45;
	ip->tos = 0;

	//ip->total_len = htons(len);
	ip_set_total_len(ip, len);

	//ip->fragment_id = 0;
	ip_set_fragment_id(ip, 0);

	//ip->flags_framgent_offset = 0;
	ip_set_flags_framgent_offset(ip, 0);

	ip->ttl = IP_PACKET_TTL;

	//ip->cksum = 0;
	ip_set_cksum(ip, 0);

	ip_set_from_addr(ip, NetInfo.ip_addr);

	ip_set_cksum(ip, ip_cksum(0, (void*) ip, sizeof(ip_packet_t)));
//	ip_set_cksum(ip, ip_cksum(0, (void*) ip, SIZE_OF_IP_PACKET)); //sizeof(ip_packet_t));

	eth_send(eth, len);

	return 1;
}

