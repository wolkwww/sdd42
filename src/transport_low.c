// transport_low.c

#include <stdio.h>

#include "common.h"
#include "Transport.h"
#include "transport_low.h"

#include "lan.h"
#include "udp.h"
#include "test_udp.h"
#include "utils.h"
#include "UART_DEBUG.h"

volatile bool packetReceived = FALSE;

void TransportLowInit(void)
{
}

// отправка сообщения "наружу"
// формат MsgBuf:
// ip_addr:addr|msg
void TransportSendMsg(char* MsgBuf)
{
	//TODO!!

	u32 ip = GetMsgIP(MsgBuf);
	UdpSendMsg(ip, MsgBuf);
}

void RFSendMsg(char* addr, char* msg)
{
	//TODO!!
}

/*******************************************************************************
 * Function Name  : processReceivedPackets
 * Description    : It processes packed received in RX
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
bool processReceivedPackets(void)
{
	//TODO!!
	bool res = false;

	if (packetReceived == TRUE)
	{
		// собираем конечный адрес

//		printf("MESSAGE: |%s|\n", MsgBuf);

		ReceiveFullMsg(MsgBuf);

		res = true;

		/* The packet has been processed, so free the single entry queue up */
	}
	packetReceived = FALSE;

	return (res);
}

void S42Proc(eth_frame_t* eth, u16 len)
{
	u8 i;
	u8* data;

//	char* s = "S42Proc\n";

	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	data = udp_get_data(udp);

	// копирование в рабочий буфер
	for (i = 0; i < min(len, MSG_BUF_SIZE); ++i)
	{
		MsgBuf[i] = data[i];
	}
	MsgBuf[len] = '\0';
	packetReceived = TRUE;

	// тест
//	msgn("############  S42Proc  ###############");

//	for (i = 0; i < len; ++i)
//	{
//		putchar(data[i]);
//	}

//	slen = strlen(s);
//	for (i = 0; i < slen; ++i)
//	{
//		data[i] = s[i];
//	}
//	udp_reply(eth, slen);
}
