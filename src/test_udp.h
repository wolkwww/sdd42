// test_udp.h

#ifndef _TEST_UDP_H_
#define _TEST_UDP_H_

void TestProc(eth_frame_t* eth, u16 len);
u8 UdpSendMsg(u32 ipaddr, char* s);
void UdpSendTime(u32 s, u32 m, u32 h);

#endif 
