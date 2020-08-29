/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_tx.c
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

#include "avr8_com_enc28j60.h"
#include "avr8_com_eth.h"

/**
 * AVR8_COM API
 * Transmitting packet, waiting for any previous transmission to end.
 *     to_mac - contains MAC of receiver
 *     len - is the length of data
 *     buff - buffer for data
 */
void avr8_com_enc28j60_transmit_packet(
	uint8_t len, uint8_t *to_mac, uint8_t *buff
) {
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
    if((avr8_com_enc28j60_read_reg(EIR) & EIR_TXERIF)) {
        avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
        avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
#endif
	/**
	 * Set write pointer to start of transmit buffer area
	 * +1 to skip per packet control byte and write directly
	 * MAC, control byte was set to zero during initialization
	 * and remains like that.
	 */
	avr8_com_enc28j60_write_reg(EWRPTL, (TXSTART_INIT + 1) & 0xFF);
	avr8_com_enc28j60_write_reg(EWRPTH, (TXSTART_INIT + 1) >> 8);
	avr8_com_enc28j60_write_buffer(6, to_mac);
	/**
	 * Set the write pointer to first byte of EtherType field
	 * (field after MAC address), this is IEEE802.3 length field.
	 */
	address = TXSTART_INIT + 1 + AVR8_COM_ETH_TYPE_H_P;
	avr8_com_enc28j60_write_reg(EWRPTL, address & 0xFF);
	avr8_com_enc28j60_write_reg(EWRPTH, address >> 8);
	/**
	 * Write length of data in the ETHERNET type field.
	 * Type field to be interpreted by receiver as
	 * IEEE802.3 length if value is less than 0x05dc.
	 */
	avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0);
	avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, len);
	/**
	 * Copy the PAYLOAD into transmit buffer,
	 * remove destination MAC and write rest.
	 */
	avr8_com_enc28j60_write_buffer(len, buff);
	/* Set TXND pointer to correspond to PAYLOAD size given. */
	address = (TXSTART_INIT + AVR8_COM_ETH_HEADER_LEN + len);
	avr8_com_enc28j60_write_reg(ETXNDL, address & 0xFF);
	avr8_com_enc28j60_write_reg(ETXNDH, address >> 8);
	/* Send contents of transmit buffer on to network. */
	avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}
