/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60.c
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

#include <util/delay.h>
#include "avr8_com_enc28j60.h"
#include "avr8_com_eth.h"

uint8_t global_bank;
uint16_t global_next_packet_ptr;

/**
 * AVR8_COM API bank - Set the control register bank to use.
 *     bank - target bank
 */
void avr8_com_enc28j60_set_bank(uint8_t bank) {
	if (bank != global_bank) {
		avr8_com_enc28j60_write_op(
		    ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0)
		);
		avr8_com_enc28j60_write_op(
			ENC28J60_BIT_FIELD_SET, ECON1, (bank & 0x03)
		);
		global_bank = bank;
	}
}

/**
 * AVR8_COM API - Initialize the controller.
 *     mac_addr - avr8_com MAC
 */
void avr8_com_enc28j60_init(uint8_t *mac_addr) {
	global_bank = 0xFF; /* Non-existent. */
	/* Initialize ATMEL I/O. */
	ENC28J60_DDR &= ~_BV(ENC28J60_PIN_MISO);
	ENC28J60_DDR |= (
	    _BV(ENC28J60_PIN_CS) | _BV(ENC28J60_PIN_MOSI) | _BV(ENC28J60_PIN_SCK)
	);
	CSPASSIVE;
	ENC28J60_PORT &= ~_BV(ENC28J60_PIN_MOSI);
	ENC28J60_PORT &= ~_BV(ENC28J60_PIN_SCK);
	/* Initialize ATMEL SPI interface master mode and FOSC/2 clock. */
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR |= (1 << SPI2X);
	/* Reset controller. */
	avr8_com_enc28j60_write_op(ENC28J60_SOFT_RESET, 0x1F, ENC28J60_SOFT_RESET);
	/* Note: polling CLKRDY does not work, check revision B4 Silicon ERRATA. */
	_delay_ms(50);
	/* Setup CLK out: 2 is 12.5MHz. */
	avr8_com_enc28j60_set_bank(3);
	avr8_com_enc28j60_write_reg(ECOCON, 2);
	avr8_com_enc28j60_set_bank(0); /* Do bank 0 stuff. */
	/**
	 * Initialize receive buffer 16-bit transfers, must write low
	 * byte first, set receive buffer start address.
	 */
	global_next_packet_ptr = RXSTART_INIT;
	avr8_com_enc28j60_write_reg(ERXSTL, RXSTART_INIT & 0xFF); /* RX start. */
	avr8_com_enc28j60_write_reg(ERXSTH, RXSTART_INIT >> 8);
	/* Set receive pointer address. */
	avr8_com_enc28j60_write_reg(ERXRDPTL, RXSTART_INIT & 0xFF);
	avr8_com_enc28j60_write_reg(ERXRDPTH, RXSTART_INIT >> 8);
	avr8_com_enc28j60_write_reg(ERXNDL, RXSTOP_INIT & 0xFF);
	avr8_com_enc28j60_write_reg(ERXNDH, RXSTOP_INIT >> 8); /* RX end. */
	avr8_com_enc28j60_write_reg(ETXSTL, TXSTART_INIT & 0xFF); /* TX start. */
	avr8_com_enc28j60_write_reg(ETXSTH, TXSTART_INIT >> 8);
	/* TX end (initialize for a packet with a PAYLOAD of 1 byte). */
	uint16_t address = (TXSTART_INIT + AVR8_COM_ETH_HEADER_LEN + 1);
	avr8_com_enc28j60_write_reg(ETXNDL, address & 0xFF);
	avr8_com_enc28j60_write_reg(ETXNDH, address >> 8);
	/**
	 * Prepare parts of transmit packet that never change.
	 * Write per-packet control byte (0x00 means use macon3 settings).
	 */
	avr8_com_enc28j60_write_reg(EWRPTL, (TXSTART_INIT) & 0xFF);
	avr8_com_enc28j60_write_reg(EWRPTH, (TXSTART_INIT) >> 8);
	avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	/* Write broadcast address as DST MAC to have some default setting. */
	uint8_t i = 0;
	while (i < 6) {
		avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0xFF);
		i++;
	}
	/**
	 * Set our MAC address as SRC MAC into transmit buffer.
	 * Set write pointer to start of transmit buffer area.
	 */
	avr8_com_enc28j60_write_buffer(6, mac_addr);
	/* First EtherType/length byte is always 0, initialize second byte to 1. */
	avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	avr8_com_enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x01);
	avr8_com_enc28j60_set_bank(1); /* Do bank 1 stuff, packet filter. */
	/* Only allow UNICAST packets destined for us and that have correct CRC. */
	avr8_com_enc28j60_write_reg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN);
	avr8_com_enc28j60_set_bank(2); /* Do bank 2 stuff. */
	/* Enable MAC receive, disable flow control (only needed in FULL-DUPLEX). */
	avr8_com_enc28j60_write_reg(MACON1, MACON1_MARXEN);
	avr8_com_enc28j60_write_reg(MACON2, 0x00); /* Bring MAC out of reset. */
	/**
	 * Enable automatic padding to 60 bytes and CRC operations
	 * also, force HALF-DUPLEX operation.
	 */
	avr8_com_enc28j60_write_reg(
		MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN
	);
	/* HALF-DUPLEX only: back-off settings. */
	avr8_com_enc28j60_write_reg(
		MACON4, MACON4_DEFER | MACON4_BPEN | MACON4_NOBKOFF
	);
	/**
	 * Set the maximum packet size which the controller will accept.
	 * Do not send packets longer than AVR8_COM_ENC28J60_MAX_PACKET_LEN.
	 */
	avr8_com_enc28j60_write_reg(
		MAMXFLL, AVR8_COM_ENC28J60_MAX_PACKET_LEN & 0xFF
	);
	avr8_com_enc28j60_write_reg(MAMXFLH, AVR8_COM_ENC28J60_MAX_PACKET_LEN >> 8);
	/* Set INTER-FRAME gap (non-back-to-back). */
	avr8_com_enc28j60_write_reg(MAIPGL, 0x12);
	avr8_com_enc28j60_write_reg(MAIPGH, 0x0C);
	/* Set INTER-FRAME gap (back-to-back). */
	avr8_com_enc28j60_write_reg(MABBIPG, 0x12);
	avr8_com_enc28j60_set_bank(3); /* Do bank 3 stuff. */
	/* Write MAC address, NOTE: MAC address in ENC28J60 is byte-backward. */
	avr8_com_enc28j60_write_reg(MAADR5, mac_addr[0]);
	avr8_com_enc28j60_write_reg(MAADR4, mac_addr[1]);
	avr8_com_enc28j60_write_reg(MAADR3, mac_addr[2]);
	avr8_com_enc28j60_write_reg(MAADR2, mac_addr[3]);
	avr8_com_enc28j60_write_reg(MAADR1, mac_addr[4]);
	avr8_com_enc28j60_write_reg(MAADR0, mac_addr[5]);
	/**
	 * Magic led configuration, LEDB=yellow LEDA=green
	 * 0x476 is PHLCON LEDA=link status, LEDB=receive/transmit.
	 */
	avr8_com_enc28j60_phy_write(PHLCON, 0x476);
	/* No LOOPBACK of transmitted frames. */
	avr8_com_enc28j60_phy_write(PHCON2, PHCON2_HDLDIS);
	/* Enable interrupts. */
	avr8_com_enc28j60_write_op(
		ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE
	);
	/* Enable packet reception. */
	avr8_com_enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

/* AVR8_COM API - Checking is link up. */
uint8_t avr8_com_enc28j60_is_link_up(void) {
	return (avr8_com_enc28j60_phy_read_high(PHSTAT2) & PHSTAT2H_LSTAT);
}
