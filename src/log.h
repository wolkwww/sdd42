#ifndef _LOG_H_
#define _LOG_H_

typedef enum TLOGType
{
	LOG_NONE = 0,
	LOG_GPS = 1,
	LOG_FS = 2,
	LOG_ADC = 3,
	LOG_DNS = 4,
	LOG_ATM = 5,
	LOG_PPP = 6,
	LOG_TCP = 7,
	LOG_ICMP = 8,
	LOG_IP = 9,
	LOG_INFO = 10,
	LOG_UDP = 11,
	LOG_BUS = 12,
	LOG_FTP = 13,
	LOG_MEM = 14,
	LOG_OTHER = 254
} T_LOG_Type;

#endif
