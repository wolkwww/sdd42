// ntp.c

#include <stdio.h>
#include "time.h"
#include "common.h"
#include "lan.h"
#include "arp.h"
#include "udp.h"
#include "ntp.h"
#include "utils.h"
#include "HAL.h"

#include "test_udp.h"
#include "tftp.h"

// ����� ������� � NTP - ����� � �������� � 1 ������ 1900 �.
//  � ������� fixed point 32:32
typedef struct ntp_timestamp
{
	u32 seconds __attribute__((packed)); // 4; // ����� �����
	u32 fraction __attribute__((packed)); // 4; // ������� �����
} ntp_timestamp_t;

u32 ntp_ts_get_seconds(ntp_timestamp_t* ntp_ts)
{
	return swap4(ntp_ts->seconds);
}

// ������ NTP-���������
typedef struct ntp_message
{
	// ���������� � ������
	u8 status;

	// ���������� �� ��������� ����� (���, ��������, etc.)
	u8 type;
	u16 _precision __attribute__((packed)); // 2
	u32 _est_error __attribute__((packed)); // 4
	u32 _est_drift_rate __attribute__((packed)); // 4

	u32 _ref_clock_id __attribute__((packed)); // 4

	// ���������� � �������
	ntp_timestamp_t ref_timestamp; // ��������� ��������� �����
	ntp_timestamp_t orig_timestamp; // �������� ������ ��������
	ntp_timestamp_t recv_timestamp; // ��������� ������ ��������
	ntp_timestamp_t xmit_timestamp; // �������� ������ ��������
} ntp_message_t;

ntp_message_t* udp_get_ntp(udp_packet_t* udp)
{
	return (ntp_message_t*) udp->data;
}

// �������� ������� �� NTP-������
u8 ntp_request(u32 srv_ip)
{
	eth_frame_t* eth = (void*) net_buf;
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);
	ntp_message_t* ntp = udp_get_ntp(udp);

	ip_set_to_addr(ip, srv_ip);
	udp_set_to_port(udp, NTP_SRV_PORT);

	udp_set_from_port(udp, NTP_LOCAL_PORT);

	// ntp.status = 8
	// ��������� ���� ��������� ������
	memset(ntp, 0, sizeof(ntp_message_t));
	ntp->status = 0x08;

	u8 res = udp_send(eth, sizeof(ntp_message_t));
	return res;
}


// ��������� ������ NTP-�������
u32 ntp_parse_reply(ntp_message_t* ntp, u16 len)
{
//	printf("ntp_parse_reply len = %u\n\n", (u32)len);

	// ��������� ����� ������
	if (len >= sizeof(ntp_message_t))
	{
//		msgn("��������� � ���������� timestamp � ����������!");
		// ��������� � ���������� timestamp � ����������
		u32 temp = ntp->xmit_timestamp.seconds;
		return (swap4(temp) - 2208988800UL);
	}
	return 0;
}

// ����� NTP-�������
//  !!! ������� �� �������� ����� �������� � �������, �� ������� ������ !!!
#define NTP_SERVER    inet_addr(62,117,76,142)

#define NTP_SERVER2    inet_addr(88,147,254,232)
#define NTP_SERVER3    inet_addr(91,226,136,136)
#define NTP_SERVER4    inet_addr(109,195,19,73)
#define NTP_SERVER1    inet_addr(88,147,254,234)
#define NTP_SERVER5    inet_addr(91,226,136,138)
#define NTP_SERVER6    inet_addr(91,226,136,139)
#define NTP_SERVER7    inet_addr(91,226,136,141)
#define NTP_SERVER8    inet_addr(88,147,254,229)

#define NTP_SERVER_COUNT	8

const u32 NtpServers[NTP_SERVER_COUNT] =
{ NTP_SERVER1, NTP_SERVER2, NTP_SERVER3, NTP_SERVER4, NTP_SERVER5, NTP_SERVER6, NTP_SERVER7, NTP_SERVER8 };

int NtpServersPos = 0;

// ����� ���������� NTP-�������
static volatile u32 ntp_next_update;

void ntp_process(void)
{
	static u32 display_next_update = 0;

	eth_frame_t *frame = (void*) net_buf;
	u16 len;

	// ����� ������
	if ((len = enc28j60_recv_packet(net_buf, ENC28J60_MAXFRAME))) //sizeof(net_buf))))
	{
//		printf("Packet arrived enc28j60_recv_packet %u\n", (u32) len);

		if (len == sizeof(net_buf))
		{
			printf("NET SYSTEM ERROR: reinited!!!\n");
			enc28j60_init(NetInfo.mac_addr);
			return;
		}

		eth_filter(frame, len);
	}

//	printf("len = %u ", (u32)len);

	// ���� ��������� NTP-������
	if (GetRTC() >= ntp_next_update)
	{
		msg("Trying to send a request to the NTP-server: (");
		p32(NtpServersPos);
		msg(") ");
		prnIP(NtpServers[NtpServersPos]);
		CR();

		// ������� ��������� ������ �� NTP-������
		if (!ntp_request(NtpServers[NtpServersPos]))
		{
			// ����� �� ��������� - MAC-����� ����� ��� �� ��������
			// ��������� ����� ����� 2 �������
			ntp_next_update = GetRTC() + 2;
		}
		else
		{
			// ����� ��������� - ���� ����� �� �������,
			// ��������� ����� ����� 60 ������
			ntp_next_update = GetRTC() + 60;

			NtpServersPos++;
			if (NtpServersPos >= NTP_SERVER_COUNT)
			{
				NtpServersPos = 0;
			}
		}

//		tftp_request(TFTP_SERVER, "test.txt");
	}

	// ���� �������� ������ �� ������ (������ ����� ��������)
	if (GetRTC() >= display_next_update)
	{
		tm tmr;
		GetCurrDateTime(&tmr);

//		printf("(%u)\t%02u.%02u.%02u %02u:%02u:%02u ", //
//		GetUptime(), //
//		tmr.tm_year - 100, //
//		tmr.tm_mon + 1, //
//		tmr.tm_mday, //
//		tmr.tm_hour, //
//		tmr.tm_min, //
//		tmr.tm_sec); //
//
//		showARP();
//		CR();

		// ��������� ���������� ����� 1 �������
		display_next_update = GetRTC() + 60;//10;

//		UdpSendTime(s, m, h);
	}
}

void NTPProc(eth_frame_t* eth, u16 len)
{
	printf("NTPProc len = %u\n\n", (u32) len);
	ip_packet_t* ip = eth_get_ip(eth);
	udp_packet_t* udp = ip_get_udp(ip);

	u32 timestamp;

	// �������� ����� �� NTP-�������?
	msgn("Received a response from the NTP-server?");

	if ((timestamp = ntp_parse_reply(udp_get_ntp(udp), len)))
	{
		msgn("NTP- OK! Store the time!");
		// ���������� �����
		time_offset = timestamp - GetRTC();
		IsTimeValid = true;

		// ��������� ���������� ����� 12 �����
		ntp_next_update = GetRTC() + 12UL * 60 * 60;

		// ��������������� "�����������" ����� ntp
		NtpServersPos--;
		if (NtpServersPos < 0)
		{
			NtpServersPos = NTP_SERVER_COUNT - 1;
		}

	}
}
