/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_rx.c
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
 * Poll for a received ETHERNET packet. PAYLOAD of packet is
 * copied to buffer and actual PAYLOAD length is returned.
 *     max_len - is the length of data
 *     from_mac - contains MAC of sender
 *     buff - buffer for data
 */
uint8_t avr8_com_enc28j60_poll_packet(
	uint8_t max_len, uint8_t *from_mac, uint8_t *buff
) {
	uint8_t len;
	uint16_t current_packet_ptr = global_next_packet_ptr;
	uint16_t address;
	uint16_t frame_len;
	/* Check if a packet has been received and buffered. */
	avr8_com_enc28j60_set_bank(1);
	if (avr8_com_enc28j60_read_reg(EPKTCNT) == 0) {
		return (0);
	}
	avr8_com_enc28j60_set_bank(0);
	/**
	 * Read pointer is NOT already at start of next packet
	 * even though we leave it in that state.
	 */
	avr8_com_enc28j60_write_reg(ERDPTL, (global_next_packet_ptr & 0xff));
	avr8_com_enc28j60_write_reg(ERDPTH, (global_next_packet_ptr) >> 8);
	/* Read next packet pointer. */
	global_next_packet_ptr = avr8_com_enc28j60_read_op(
		ENC28J60_READ_BUF_MEM, 0
	);
	global_next_packet_ptr |= avr8_com_enc28j60_read_op(
		ENC28J60_READ_BUF_MEM, 0
	) << 8;
	/* Read the frame length. */
	frame_len = avr8_com_enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	frame_len |= avr8_com_enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0) << 8;
	/* Subtract ETHERNET source, destination and length fields. */
	if (max_len > frame_len - 14) {
		max_len = frame_len - 14;
	}
	/* Note: +6 for receive vectors. */
	address = current_packet_ptr + AVR8_COM_ETH_SRC_MAC_P + 6;
	if (address > RXSTOP_INIT) {
		address -= (RXSTOP_INIT - RXSTART_INIT + 1);
	}
	avr8_com_enc28j60_write_reg(ERDPTL, address & 0xff);
	avr8_com_enc28j60_write_reg(ERDPTH, (address) >> 8);
	avr8_com_enc28j60_read_buffer(6, from_mac);
	/**
	 * Value of less than 0x05dc in EtherType has to be interpreted as
	 * length. Length is 16 bit. Upper 8 bits must be zero otherwise it
	 * is not our packet.
	 */
	len = avr8_com_enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	if (len != 0) {
		len = 0;
		goto NEXTPACKET;
	}
	/* Read lower byte of the length field. */
	len = avr8_com_enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	/* Limit retrieve length to max_len, ignoring anything else. */
	if (len > max_len) {
		len = max_len;
	}
	/* Copy PAYLOAD data from receive buffer. */
	avr8_com_enc28j60_read_buffer(len, buff);
	NEXTPACKET:
	/**
	 * Move the RX read pointer to start of next received packet,
	 * this frees memory we just read out. However, compensate
	 * for the ERRATA, revision B4: write an even address!
	 */
	if (
	    (global_next_packet_ptr - 1 < RXSTART_INIT) ||
		(global_next_packet_ptr - 1 > RXSTOP_INIT)
	) {
		avr8_com_enc28j60_write_reg(ERXRDPTL, (RXSTOP_INIT)&0xFF);
		avr8_com_enc28j60_write_reg(ERXRDPTH, (RXSTOP_INIT)>>8);
	} else {
		avr8_com_enc28j60_write_reg(
			ERXRDPTL, (global_next_packet_ptr - 1) & 0xFF
		);
		avr8_com_enc28j60_write_reg(
			ERXRDPTH, (global_next_packet_ptr - 1) >> 8
		);
	}
	/* Decrement packet counter indicate we are done with this packet. */
	avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return (len);
}
