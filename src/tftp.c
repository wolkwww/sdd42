/*
 * tftp.c
 *
 *  Created on: 03.09.2013
 *      Author: mrhru
 */

#include <stdio.h>

#include "common.h"
#include "lan.h"
#include "arp.h"
#include "udp.h"
#include "ntp.h"

#include "tftp.h"

// RFC1350

u16 clientTID = 0;
u16 serverTID = 0;

typedef enum
{
	TFTP_OPCODES_RRQ = 1,	// Read request
	TFTP_OPCODES_WRQ = 2,	// Write request
	TFTP_OPCODES_DATA = 3,	// Data
	TFTP_OPCODES_ACK = 4,	// Acknowledgment
	TFTP_OPCODES_ERROR = 5,	// Error

	TFTP_OPCODES_RESET = 101,	// RESET REQUEST, not in TFTP standart
} TFTP_OPCODES;

// tftp packet mode
const char* TFTP_MODE_NETASCII = "netascii";	// size = 8 + 1
const char* TFTP_MODE_OCTET = "octet";			// size = 5 + 1
const char* TFTP_MODE_MAIL = "mail";			// size = 4 + 1

#define FILE_NAME_AND_MODE_LENGTH		32

// TFTP-сообщение c opcode
typedef struct tftp_message
{
	// тип пакета
	u16 Opcode __attribute__((packed));
} tftp_message_t;


// TFTP-сообщение запроса
typedef struct tftp_rrq_wrq_message
{
	// тип пакета
	u16 Opcode __attribute__((packed));

	char FileNameAndMode[FILE_NAME_AND_MODE_LENGTH];
} tftp_rrq_wrq_message_t;

// TFTP-сообщение с данными
typedef struct tftp_data_message
{
	// тип пакета
	u16 Opcode __attribute__((packed));

	// номер пакета
	u16 Block __attribute__((packed));

	u8 Data[512];
} tftp_data_message_t;

// TFTP-сообщение подтверждение
typedef struct tftp_ack_message
{
	// тип пакета
	u16 Opcode __attribute__((packed));

	// номер пакета
	u16 Block __attribute__((packed));
} tftp_ack_message_t;



tftp_message_t* udp_get_tftp(udp_packet_t* udp)
{
	return (tftp_message_t*) udp->data;
}

tftp_rrq_wrq_message_t* udp_get_tftp_rrq(udp_packet_t* udp)
{
	return (tftp_rrq_wrq_message_t*) udp->data;
}

tftp_data_message_t* udp_get_tftp_data(udp_packet_t* udp)
{
	return (tftp_data_message_t*) udp->data;
}

tftp_ack_message_t* udp_get_tftp_ack(udp_packet_t* udp)
{
	return (tftp_ack_message_t*) udp->data;
}


// Отправка запроса на TFTP-сервер
u8 tftp_request(u32 srv_ip, const char* filename)
{
	eth_frame_t* eth = (void*) net_buf;
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	ip_set_to_addr(ip, srv_ip);					//
	udp_set_to_port(udp, TFTP_SERVER_PORT);		//
	udp_set_from_port(udp, TFTP_CLIENT_PORT);	//

	tftp_rrq_wrq_message_t* tftp = udp_get_tftp_rrq(udp);
	tftp->Opcode = swap2(TFTP_OPCODES_RRQ);

	int len1 = strlen((char*) filename);
	strcpy(tftp->FileNameAndMode, filename);

	int len2 = strlen(TFTP_MODE_OCTET);
	strcpy(tftp->FileNameAndMode + len1 + 1, TFTP_MODE_OCTET);

	u8 res = udp_send(eth, sizeof(u16) + len1 + 1 + len2 + 1);
	return res;
}

// Отправка запроса на TFTP-сервер
u8 tftp_send_ack(u32 srv_ip, u16 block)
{
	eth_frame_t* eth = (void*) net_buf;
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	ip_set_to_addr(ip, srv_ip);					//
	udp_set_to_port(udp, TFTP_SERVER_PORT);		//
	udp_set_from_port(udp, TFTP_CLIENT_PORT);	//

	tftp_ack_message_t* tftp = udp_get_tftp_ack(udp);
	tftp->Opcode = swap2(TFTP_OPCODES_ACK);
	tftp->Block = swap2(block);

	u8 res = udp_send(eth, sizeof(tftp_ack_message_t));
	return res;
}


void TFTPProc(eth_frame_t* eth, u16 len)
{
	msgn("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
	printf("TFTPProc len = %u\n\n", (u32) len);
	msgn("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	// анализируем только RESET пакет
	tftp_message_t* tftp = udp_get_tftp(udp);

	int opcode = (u32) swap2(tftp->Opcode);
	u32 sender_ip = ip_get_from_addr(ip);

	printf("TFTP SRV TID = %u\tOPCODE = %u\n", (u32) udp_get_from_port(udp), opcode);

	if (opcode == TFTP_OPCODES_RESET)
	{
		msgn("RESET REQUEST from ");
		prnIP(sender_ip);
		CR();
		SaveBootLoaderIP(sender_ip);
		CR();
		Reset();
	}
	else if (opcode == TFTP_OPCODES_DATA)
	{
		msgn("DATA from ");
		prnIP(sender_ip);
		CR();

		tftp_data_message_t* tftp_data = udp_get_tftp_data(udp);

		u16 block = swap2(tftp_data->Block);

		printf("TFTP DATA BLOCK# = %u\n", (u32)block);

		u8* data = tftp_data->Data;
		msgn((char*)data);
		CR();

		tftp_send_ack(sender_ip, block);
	}
	else
	{

	}
	msgn("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
	msgn("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");

}
