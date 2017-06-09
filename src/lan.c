// lan.c

#include "common.h"
#include "lan.h"
#include "arp.h"
#include "udp.h"
#include "sbl_iap.h"

u8 mac_addr1[6] =
{ 0x00, 0x13, 0x37, 0x01, 0x27, 0x45 };
u8 mac_addr2[6] =
{ 0x00, 0x13, 0x37, 0x01, 0x27, 0x46 };

u8 mac_addr0[6] =
{ 0x00, 0x13, 0x00, 0x00, 0x00, 0x00 };

NetInfoDef NetInfo;

//u8* mac_addr;
//u32 ip_addr = 0;
//u32 ip_mask = IP_MASK;
//u32 ip_gateway = IP_GATEWAY;

u32 server_ip = DEFAULT_SERVER_IP;

//u8 net_buf[ENC28J60_MAXFRAME];
u8* net_buf;

void eth_reply(eth_frame_t* eth, u16 len);
void ip_reply(eth_frame_t *eth, u16 len);

ip_packet_t* eth_get_ip(eth_frame_t* eth)
{
	return (ip_packet_t*) eth->data;
}

// ICMP

icmp_echo_packet_t* ip_get_icmp(ip_packet_t* ip)
{
	return (icmp_echo_packet_t*) ip->data;
}

u16 icmp_get_cksum(icmp_echo_packet_t* icmp)
{
	return swap2(icmp->_cksum);
}
void icmp_set_cksum(icmp_echo_packet_t* icmp, u16 value)
{
	icmp->_cksum = swap2(value);
}
u16 icmp_get_id(icmp_echo_packet_t* icmp)
{
	return swap2(icmp->_id);
}
u16 icmp_get_seq(icmp_echo_packet_t* icmp)
{
	return swap2(icmp->_seq);
}

void icmp_filter(eth_frame_t* eth, u16 len)
{
//	printf("icmp_filter len = %u\n\n", (u32) len);

	ip_packet_t* ip = eth_get_ip(eth);
	icmp_echo_packet_t* icmp = ip_get_icmp(ip);

//	printf("icmp->type: %X\tICMP_TYPE_ECHO_RQ: %X len = %u (%u)\n", (u32) icmp->type, (u32) ICMP_TYPE_ECHO_RQ,
//	        (u32) len, (u32) sizeof(icmp_echo_packet_t));

	if (len >= sizeof(icmp_echo_packet_t))
	{
		if (icmp->type == ICMP_TYPE_ECHO_RQ)
		{
//			msgn("PING MATCH!!!");
			icmp->type = ICMP_TYPE_ECHO_RPLY;

			icmp_set_cksum(icmp, 0);
			icmp_set_cksum(icmp, ip_cksum(0, (void*) icmp, len));

			ip_reply(eth, len);
		}
	}
}

// IP

u16 ip_get_total_len(ip_packet_t* ip_pak)
{
	return swap2(ip_pak->_total_len);
}
void ip_set_total_len(ip_packet_t* ip_pak, u16 value)
{
	ip_pak->_total_len = swap2(value);
}

u16 ip_get_fragment_id(ip_packet_t* ip_pak)
{
	return swap2(ip_pak->_fragment_id);
}
void ip_set_fragment_id(ip_packet_t* ip_pak, u16 value)
{
	ip_pak->_fragment_id = swap2(value);
}

u16 ip_get_flags_framgent_offset(ip_packet_t* ip_pak)
{
	return swap2(ip_pak->_flags_framgent_offset);
}
void ip_set_flags_framgent_offset(ip_packet_t* ip_pak, u16 value)
{
	ip_pak->_flags_framgent_offset = swap2(value);
}

u16 ip_get_cksum(ip_packet_t* ip_pak)
{
	return swap2(ip_pak->_cksum);
}
void ip_set_cksum(ip_packet_t* ip_pak, u16 value)
{
	ip_pak->_cksum = swap2(value);
}

u32 ip_get_from_addr(ip_packet_t* ip_pak)
{
	return swap4(ip_pak->_from_addr);
}
void ip_set_from_addr(ip_packet_t* ip_pak, u32 value)
{
	ip_pak->_from_addr = swap4(value);
}

u32 ip_get_to_addr(ip_packet_t* ip_pak)
{
	return swap4(ip_pak->_to_addr);
}
void ip_set_to_addr(ip_packet_t* ip_pak, u32 value)
{
	ip_pak->_to_addr = swap4(value);
}

u16 ip_cksum(u32 sum, u8 *buf, u32 len)
{
//	printf("ip_cksum len = %u\n\n", (u32) len);
	while (len >= 2)
	{
		sum += ((u16) *buf << 8) | *(buf + 1);
		buf += 2;
		len -= 2;
	}

	if (len)
	{
		sum += (u16) *buf << 8;
	}

	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

//	return ~swap2((u16)sum);
	return ~((u16) sum);
}

void ip_reply(eth_frame_t* eth, u16 len)
{
//	printf("ip_reply len = %u\n\n", (u32) len);
	ip_packet_t* ip = eth_get_ip(eth);

	len += sizeof(ip_packet_t);

	ip_set_total_len(ip, len);
	ip_set_fragment_id(ip, 0);
	ip_set_flags_framgent_offset(ip, 0);
	ip->ttl = IP_PACKET_TTL;
	ip_set_cksum(ip, 0);
	ip_set_to_addr(ip, ip_get_from_addr(ip));
	ip_set_from_addr(ip, NetInfo.ip_addr);
	ip_set_cksum(ip, ip_cksum(0, (void*) ip, sizeof(ip_packet_t)));

	eth_reply(eth, len);
}

void ip_filter(eth_frame_t* eth, u16 len)
{
//	printf("ip_filter len = %u\n\n", (u32) len);

	ip_packet_t* ip = eth_get_ip(eth);

//	msg("IP:\tFROM\t");
//	prnIP(ip_get_from_addr(ip));
//	msg("\tTO\t");
//	prnIP(ip_get_to_addr(ip));
//	CR();

	u32 addr = ip_get_to_addr(ip);

//	if ((ip->ver_head_len == 0x45) &&
//			(ip_cksum(0, (void*) ip, sizeof(ip_packet_t)) == hcs) &&
//			((addr == ip_addr) || ((addr & 0x000000ff) == 0x000000ff)))
//	{

	if (ip->ver_head_len != 0x45)
	{
		printf("ip->ver_head_len != 0x45 (%u)\n", (u32) ip->ver_head_len);
		return;
	}

	u16 calc_hcs, hcs = ip_get_cksum(ip);
	ip_set_cksum(ip, 0);
	if ((calc_hcs = ip_cksum(0, (void*) ip, sizeof(ip_packet_t))) != hcs)
	{
		if (((hcs == 0xFFFF) && (calc_hcs == 0)) || ((hcs == 0) && (calc_hcs == 0xFFFF)))
		{
			// may be it's correct
		}
		else
		{
			printf("ip->cksum don't match\t(%u)\t(%u)\n", (u32) hcs, (u32) calc_hcs);
			return;
		}
	}

//	printf("ip->cksum OK\n");

	if ((addr != NetInfo.ip_addr) && ((addr & 0x000000ff) == 0x000000ff))
	{
//		printf("ip->to_addr don't match\n");
		return;
	}

	len = ip_get_total_len(ip) - sizeof(ip_packet_t);

	switch (ip->protocol)
	{
		case IP_PROTOCOL_ICMP:
//			msgn("ICMP");
			icmp_filter(eth, len);
			break;
		case IP_PROTOCOL_UDP:
//			msgn("UDP");
			udp_filter(eth, len);
			break;
	}
//	}
}

// Ethernet

u16 eth_get_type(eth_frame_t* eth)
{
	return swap2(eth->_type);
}
void eth_set_type(eth_frame_t* eth, u16 type)
{
	eth->_type = swap2(type);
}
void eth_set_from_addr(eth_frame_t* eth, u8* mac)
{
	memcpy(eth->from_addr, mac, 6);
}
void eth_set_to_addr(eth_frame_t* eth, u8* mac)
{
	memcpy(eth->to_addr, mac, 6);
}
void eth_fill_to_addr(eth_frame_t* eth, u8 fill_pattern)
{
	memset(eth->to_addr, fill_pattern, 6);
}

// Отправка Ethernet-фрейма
// Должны быть установлены следующие поля:
//    - frame.to_addr - MAC-адрес получателя
//    - frame.type - протокол
// len - длина поля данных фрейма
void eth_send(eth_frame_t* eth, u16 len)
{
//	printf("eth_send len = %u\n\n", (u32) len);
//  memcpy(frame->from_addr, mac_addr, 6);
	eth_set_from_addr(eth, NetInfo.mac_addr);

	enc28j60_send_packet((void*) eth, len + sizeof(eth_frame_t)); //SIZE_OF_ETH_FRAME);
}

void eth_reply(eth_frame_t* eth, u16 len)
{
//	printf("eth_reply len = %u\n\n", (u32) len);
	//memcpy(eth->to_addr, eth->from_addr, 6);
	eth_set_to_addr(eth, eth->from_addr);

	//memcpy(eth->from_addr, mac_addr, 6);
	eth_set_from_addr(eth, NetInfo.mac_addr);

	enc28j60_send_packet((void*) eth, len + sizeof(eth_frame_t)); //SIZE_OF_ETH_FRAME);
}

void eth_filter(eth_frame_t* eth, u16 len)
{
//	printf("eth_filter len = %u\n\n", (u32) len);
	if (len >= sizeof(eth_frame_t)) //SIZE_OF_ETH_FRAME)
	{
//		printf("ETHER:\tTYPE: %X", (u32) eth_get_type(eth));
//		msg("\tFROM: ");
//		prnMAC(eth->from_addr);
//		msg("\t\tTO: ");
//		prnMAC(eth->to_addr);
//		CR();
		switch (eth_get_type(eth))
		{
			case ETH_TYPE_ARP:
//				msgn("arp_filter");
				arp_filter(eth, len - sizeof(eth_frame_t));
				break;
			case ETH_TYPE_IP:
//				msgn("ip_filter");
				ip_filter(eth, len - sizeof(eth_frame_t));
				break;
		}
	}
}

// LAN

void InitMACAddress(void)
{
	// создаем МАС-адрес из уникального номера процессора (самое первое 32-битное слово из 4-х)
	u32 udin[4];
	read_device_serial_number(udin);

	mac_addr0[2] = (u8) (udin[0] >> 24);
	mac_addr0[3] = (u8) (udin[0] >> 16);
	mac_addr0[4] = (u8) (udin[0] >> 8);
	mac_addr0[5] = (u8) (udin[0] >> 0);
}

void lan_init()
{
	InitMACAddress();

	memcpy(NetInfo.mac_addr, mac_addr0, 6);
	NetInfo.ip_addr = IP_ADDR0;
	NetInfo.ip_mask = IP_MASK;
	NetInfo.ip_gateway = IP_GATEWAY;

	msg("MAC:\t");
	prnMAC(NetInfo.mac_addr);
	CR();
	msg("IP:\t");
	prnIP(NetInfo.ip_addr);
	CR();

	enc28j60_init(NetInfo.mac_addr);
}

void lan_poll()
{
	u32 len;
	eth_frame_t* eth = (void*) net_buf;

	while ((len = enc28j60_recv_packet(net_buf, sizeof(net_buf))))
	{
		//printf("len: %u (%u (%u))\n", len, (u32)sizeof(eth_frame_t), (u32)SIZE_OF_ETH_FRAME);
		eth_filter(eth, len);
	}
}

