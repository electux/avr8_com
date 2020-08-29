/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_arp.c
 * Copyright (C) 2020 Vladimir Roncevic <elektron.ronca@gmail.com>
 *
 * avr8_com is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * avr8_com is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/pgmspace.h>
#include "avr8_com_enc28j60.h"
#include "avr8_com_eth.h"

const char avr8_com_ip[4] PROGMEM = { 10, 10, 42, 42 };
const char arp_req_header[10] PROGMEM = { 8, 6, 0, 1, 8, 0, 6, 4, 0, 1 };

/**
 * AVR8_COM API
 * Send gratuitous ARP request to teach any switches what avr8_com MAC is.
 *     mac_addr - avr8_com MAC
 */
void avr8_com_enc28j60_gratuitous_arp_request(uint8_t *mac_addr) {
	uint8_t i = 0;
	uint16_t address;
	avr8_com_enc28j60_set_bank(0);
#ifdef AVR8_COM_ENC28J60_HAS_PENDING_TRANSMIT_ON_TRANSMIT
	while (
		avr8_com_enc28j60_read_op(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS
	) {
		/* Reset transmit logic problem. Check revision B4 Silicon ERRATA. */
		if ((avr8_com_enc28j60_read_reg(EIR) & EIR_TXERIF)) {
			avr8_com_enc28j60_write_op(
				ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST
			);
			avr8_com_enc28j60_write_op(
				ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST
			);
		}
	}
#else
    /* Reset transmit logic problem. Check revision B4 Silicon ERRATA. */
    if ((avr8_com_enc28j60_read_reg(EIR) & EIR_TXERIF)) {
        avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
        avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
#endif
	/**
	 * Set write pointer to start of transmit buffer area +1
	 * to skip per packet control byte and write directly MAC,
	 * control byte was set to zero during initialization
	 * and remains like that.
	 */
	avr8_com_enc28j60_write_reg(EWRPTL, (TXSTART_INIT + 1) & 0xFF);
	avr8_com_enc28j60_write_reg(EWRPTH, (TXSTART_INIT + 1) >> 8);
	/* Write a broadcast destination MAC (all FF). */
	while (i < 6) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0xFF);
		i++;
	}
	/**
	 * The MAC in ETHERNET field does not need to be changed.
	 * Set write pointer to first byte of EtherType field.
	 */
	address = TXSTART_INIT + 1 + AVR8_COM_ETH_TYPE_H_P;
	avr8_com_enc28j60_write_reg(EWRPTL, address & 0xFF);
	avr8_com_enc28j60_write_reg(EWRPTH, address >> 8);
	/* 10 fixed bytes in the ARP request. */
	i = 0;
	while (i < 10) {
		avr8_com_enc28j60_write_op(
			ENC28J60_WRITE_BUF_MEM, 0, pgm_read_byte(arp_req_header + i)
		);
		i++;
	}
	i = 0;
	while (i < 6) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, mac_addr[i]);
		i++;
	}
	i = 0;
	while (i < 4) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, avr8_com_ip[i]);
		i++;
	}
	/* Target data. */
	i = 0;
	while (i < 6) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0xff);
		i++;
	}
	/* To self, for gratuitous ARP. */
	i = 0;
	while (i < 4) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, avr8_com_ip[i]);
		i++;
	}
	/* Set TXND pointer to correspond to PAYLOAD size given. */
	address = (TXSTART_INIT + 42);
	avr8_com_enc28j60_write_reg(ETXNDL, address & 0xFF);
	avr8_com_enc28j60_write_reg(ETXNDH, address >> 8);
	/* Send the contents of the transmit buffer on to network. */
	avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}
