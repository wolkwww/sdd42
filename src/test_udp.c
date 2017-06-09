// test_udp.c

#include "lan.h"
#include "udp.h"
#include "test_udp.h"
#include "UART_DEBUG.h"

// UDP port 12345
void TestProc(eth_frame_t* eth, u16 len)
{
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	u8* data = udp_get_data(udp);

	msgn("############  udp packet  ###############");

	for (u32 i = 0; i < len; ++i)
	{
		putchar(data[i]);
	}

	for (u32 i = 0; i < len - 2; ++i)
	{
		data[i] = (u8) (i + '0');
	}
	data[len - 2] = '\n';
	data[len - 1] = '\r';
	udp_reply(eth, len);
}

u8 UdpSendMsg(u32 ipaddr, char* s)
{
	eth_frame_t* eth = (void*) net_buf;
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);
	u8* data = udp_get_data(udp);

	ip_set_to_addr(ip, ipaddr);

	udp_set_to_port(udp, S42_LOCAL_PORT);

	udp_set_from_port(udp, S42_LOCAL_PORT);

	strcpy((char*) data, s);

	return udp_send(eth, strlen((char*) data) + 1);
}

void UdpSendTime(u32 s, u32 m, u32 h)
{
	char buf[12];

	sprintf(buf, "%2d.%2d.%2d\n", h, m, s);

	UdpSendMsg(server_ip, buf);
}

