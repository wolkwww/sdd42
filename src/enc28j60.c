// enc28j60.c

#include <stdio.h>

#include "enc28j60.h"
#include "spi.h"
#include "HAL.h"
#include "common.h"

u16 enc28j60_free_space(void);

u16 enc28j60_rxrdpt = 0;

#define enc28j60_select() 	CS_Low()
#define enc28j60_release() 	CS_High()

u8 enc28j60_rxtx(u8 data)
{
	return SPI_ReadWriteData(data);
}

u8 enc28j60_rx(void)
{
	return enc28j60_rxtx(0xff);
}

u8 enc28j60_tx(u8 data)
{
	return enc28j60_rxtx(data);
}

void enc28j60_2tx(u8 b1, u8 b2)
{
	enc28j60_tx(b1);
	enc28j60_tx(b2);
}

void enc28j60_rxbuf(u8* buf, u32 len)
{
	SPI_ReadBuffer(buf, len);
}

void enc28j60_txbuf(u8* buf, u32 len)
{
	SPI_WriteBuffer(buf, len);
}

///////////////////////////////////////////////////////////////////////////
// Generic SPI read command
u8 enc28j60_read_op(u8 cmd, u8 adr)
{
	u8 data;

	enc28j60_select();
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	if (adr & 0x80) // throw out dummy byte
	{
		enc28j60_rx(); // when reading MII/MAC register
	}
	data = enc28j60_rx();
	enc28j60_release();
	return data;
}

// Generic SPI write command
void enc28j60_write_op(u8 cmd, u8 adr, u8 data)
{
	enc28j60_select();
//	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
//	enc28j60_tx(data);
	enc28j60_2tx(cmd | (adr & ENC28J60_ADDR_MASK), data);
	enc28j60_release();
}

static u8 enc28j60_current_bank = -1;
// Initiate software reset
void enc28j60_soft_reset()
{
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_SC);
	enc28j60_release();

	enc28j60_current_bank = -1;
	delay(100); // Wait until device initializes
}

// Memory access

// Set register bank
void enc28j60_set_bank(u8 adr)
{
	u8 bank;

	if ((adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR)
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if (bank != enc28j60_current_bank)
		{
			enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);

			enc28j60_current_bank = bank;
		}
	}
}

// Read register
u8 enc28j60_rcr(u8 adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
u16 enc28j60_rcr16(u8 adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr) | (enc28j60_read_op(ENC28J60_SPI_RCR, adr + 1) << 8);
}

// Write register
void enc28j60_wcr(u8 adr, u8 arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16(u8 adr, u16 arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr + 1, arg >> 8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfc(u8 adr, u8 mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfs(u8 adr, u8 mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(u8* buf, u16 len)
{
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_RBM);

//	while (len--)
//	{
//		*(buf++) = enc28j60_rx();
//	}

	enc28j60_rxbuf(buf, len);

	enc28j60_release();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(u8* buf, u16 len)
{
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_WBM);

//	while (len--)
//	{
//		enc28j60_tx(*(buf++));
//	}

	enc28j60_txbuf(buf, len);

	enc28j60_release();
}

// Read PHY register
u16 enc28j60_read_phy(u8 adr)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_bfs(MICMD, MICMD_MIIRD);

	while (enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;

	enc28j60_bfc(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16(MIRD);
}

// Write PHY register
void enc28j60_write_phy(u8 adr, u16 data)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_wcr16(MIWR, data);
	while (enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;
}

// Init & packet Rx/Tx

void enc28j60_init(u8* macadr)
{
	// Initialize SPI
	InitSPI();

	enc28j60_release();
	// Reset ENC28J60
	enc28j60_soft_reset();

	// Setup Rx/Tx buffer
	enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
	enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_wcr16(ERXND, ENC28J60_RXEND);
	enc28j60_rxrdpt = ENC28J60_RXSTART;

	// Setup MAC
	enc28j60_wcr(MACON1, MACON1_TXPAUS | // Enable flow control
	MACON1_RXPAUS | MACON1_MARXEN); // Enable MAC Rx
	enc28j60_wcr(MACON2, 0); // Clear reset
	enc28j60_wcr(MACON3, MACON3_PADCFG0 | // Enable padding,
	MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX); // Enable crc & frame len chk
	enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);
	enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
	enc28j60_wcr(MAIPGL, 0x12);
	enc28j60_wcr(MAIPGH, 0x0c);
	enc28j60_wcr(MAADR5, macadr[0]); // Set MAC address
	enc28j60_wcr(MAADR4, macadr[1]);
	enc28j60_wcr(MAADR3, macadr[2]);
	enc28j60_wcr(MAADR2, macadr[3]);
	enc28j60_wcr(MAADR1, macadr[4]);
	enc28j60_wcr(MAADR0, macadr[5]);

	// Setup PHY
	enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback
	enc28j60_write_phy(PHLCON, PHLCON_LACFG2 | // Configure LED ctrl
	PHLCON_LBCFG2 | PHLCON_LBCFG1 | PHLCON_LBCFG0 | PHLCON_LFRQ0 | PHLCON_STRCH);

	// Enable Rx packets
	enc28j60_bfs(ECON1, ECON1_RXEN);

	enc28j60_bfs(EIE, EIE_INTIE | EIE_TXIE | EIE_TXERIE);
}

void enc28j60_send_packet(u8* data, u16 len)
{
//	print_dump(data, len);

	while (enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
		if (enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
			enc28j60_bfc(ECON1, ECON1_TXRST);
			printf("X\n");
		}
	}

	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((u8*) "\x00", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);

	enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send

	push_cmd_cnt++;

	// проверка окончания передачи
	u8 eir;
	u32 cnt = 0;
	while (((eir = enc28j60_rcr(EIR)) & (EIR_TXIF | EIR_TXERIF)) == 0)
	{
		cnt++;
	}

//	if (cnt != 0)
//	{
//		printf("TX:%u\n", cnt);
//	}

	if (eir & EIR_TXIF)
	{
//		printf("TXIF\n");
		enc28j60_bfc(EIR, EIR_TXIF);
	}

	if (eir & EIR_TXERIF)
	{
		printf("TXERIF\n");
		enc28j60_bfc(EIR, EIR_TXERIF);
	}

//	printf("%u\n", cnt);
}

static u16 len, rxlen, status, temp, lasttemp;
//static u32 testcnt = 0;

u16 enc28j60_recv_packet(u8* buf, u16 buflen)
{
	len = 0;

	if (enc28j60_rcr(EPKTCNT))
	{
		enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

		enc28j60_read_buffer((u8*) &enc28j60_rxrdpt, 2); //sizeof(enc28j60_rxrdpt));
		enc28j60_read_buffer((u8*) &rxlen, 2); //sizeof(rxlen));
		enc28j60_read_buffer((u8*) &status, 2); //sizeof(status));

		if (status & 0x0080) //success
		{
			len = rxlen - 4; //throw out crc
			if (len > buflen)
				len = buflen;
			enc28j60_read_buffer(buf, len);
		}

//		printf("\t\t\t\t\t\t\t\t\t\t enc28j60_rxrdpt = %u", (u32)enc28j60_rxrdpt);
//		printf("\t############### rxlen = %u\n", (u32)rxlen);
		// Set Rx read pointer to next packet
//		temp = (enc28j60_rxrdpt - 1) & ENC28J60_BUFEND;

		temp = enc28j60_rxrdpt - 1;
		while (temp >= ENC28J60_RXSIZE)
		{
			temp -= ENC28J60_RXSIZE;
		}

		enc28j60_wcr16(ERXRDPT, temp);

		// Decrement packet counter
		enc28j60_bfs(ECON2, ECON2_PKTDEC);

//		u32 free = enc28j60_free_space();
//		printf("[%u]FS: %u (%u)\t%u (%u)\n", testcnt++, (u32) temp, (u32) (temp - lasttemp), free, (u32)(0x1A00 - free));
//		printf("(%u)\n", (u32)(0x1A00 - free));

		lasttemp = temp;
	}


	return len;
}

u16 enc28j60_free_space(void)
{
	u8 epktcnt1;
	u8 epktcnt2;
	u16 erxwrpt;

	epktcnt1 = 0;
	epktcnt2 = 1;

	while (epktcnt1 != epktcnt2)
	{
		epktcnt1 = enc28j60_rcr(EPKTCNT);	// 1

		erxwrpt = enc28j60_rcr16(ERXWRPT);	// 2

		epktcnt2 = enc28j60_rcr(EPKTCNT);	// 3
	}

	u16 erxrdpt = enc28j60_rcr16(ERXRDPT);
	u16 erxnd = enc28j60_rcr16(ERXND);
	u16 erxst = enc28j60_rcr16(ERXST);

	u16 FreeSpace;

	if (erxwrpt > erxrdpt)
	{
		FreeSpace = (erxnd - erxst) - (erxwrpt - erxrdpt);// + 10000;
	}
	else
	{
		if (erxwrpt == erxrdpt)
		{
			FreeSpace = (erxnd - erxst);// + 20000;
		}
		else
		{
			FreeSpace = erxrdpt - erxwrpt - 1;// + 30000;
		}
	}

	return FreeSpace;
}
