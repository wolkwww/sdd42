/*
 * tftp.h
 *
 *  Created on: 03.09.2013
 *      Author: mrhru
 */

#ifndef TFTP_H_
#define TFTP_H_

#define TFTP_SERVER    			inet_addr(192, 168, 0, 254)
//#define TFTP_SERVER    			inet_addr(192, 168, 0, 198)
// Порт TFTP-сервера
#define TFTP_SERVER_PORT		(69)

#define TFTP_CLIENT_PORT		(69)

void TFTPProc(eth_frame_t* eth, u16 len);
u8 tftp_request(u32 srv_ip, const char* filename);

#endif /* TFTP_H_ */
