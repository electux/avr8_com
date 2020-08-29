/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60.h
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

#ifndef AVR8_COM_ENC28J60_H
#define AVR8_COM_ENC28J60_H

#include <avr/io.h>
#include <inttypes.h>
#include "avr8_com_enc28j60_int.h"

/**
 * AVR8_COM API
 * Maximum packet length: this software has a limit of 255 PAYLOAD data bytes
 * and then there is ETHERNET overhead (srd, dst, len, fcs).
 */
#define AVR8_COM_ENC28J60_MAX_PACKET_LEN ((uint16_t) 273)

/* AVR8_COM API - Global bank */
extern uint8_t global_bank;

/* AVR8_COM API - Position where next packet will be written */
extern uint16_t global_next_packet_ptr;

/* AVR8_COM API - SPI handling macros */
#define CSACTIVE ENC28J60_PORT &= ~_BV(ENC28J60_PIN_CS)
#define CSPASSIVE ENC28J60_PORT |= _BV(ENC28J60_PIN_CS)
#define waitspi() while (!( SPSR & (1 << SPIF)))

/**
 * AVR8_COM API
 * Read a control register in controller assuming
 * that currently selected bank is correct.
 *     op - SPI operation
 *     address - address
 */
uint8_t avr8_com_enc28j60_read_op(uint8_t op, uint8_t address);

#define avr8_com_enc28j60_read_reg(ADDRESS) \
    avr8_com_enc28j60_read_op(ENC28J60_READ_CTRL_REG, ADDRESS)

/**
 * AVR8_COM API
 * Write a control register in controller assuming
 * that currently selected bank is correct.
 *     op - SPI operation
 *     address - address
 *     data - data
 */
void avr8_com_enc28j60_write_op(uint8_t op, uint8_t address, uint8_t data);

#define avr8_com_enc28j60_write_reg(ADDRESS, DATA) \
    avr8_com_enc28j60_write_op(ENC28J60_WRITE_CTRL_REG, ADDRESS, DATA)

/**
 * AVR8_COM API
 * Read from Buffer Memory.
 *     len - length of data
 *     data - pointer to data
 */
void avr8_com_enc28j60_read_buffer(uint8_t len, uint8_t *data);

/**
 * AVR8_COM API
 * Write to Buffer Memory.
 *     len - length of data
 *     data - pointer to data
 */
void avr8_com_enc28j60_write_buffer(uint8_t len, uint8_t *data);

/**
 * AVR8_COM API bank - Set the control register bank to use.
 *     bank - target bank
 */
void avr8_com_enc28j60_set_bank(uint8_t bank);

/**
 * AVR8_COM API
 * Read upper 8 bits of a PHY register.
 *     address - address
 */
uint8_t avr8_com_enc28j60_phy_read_high(uint8_t address);

/**
 * AVR8_COM API
 * Write a PHY register.
 *     address - address
 *     data - data
 */
void avr8_com_enc28j60_phy_write(uint8_t address, uint16_t data);

/**
 * AVR8_COM API
 *  Define this macro if the AVR might call avr8_com_enc28j60_transmit_packet()
 *  when there is still transmit in progress. Otherwise comment out completely.
 */
#define AVR8_COM_ENC28J60_HAS_PENDING_TRANSMIT_ON_TRANSMIT 1

/**
 * AVR8_COM API - Initialize the controller.
 *     mac_addr - avr8_com MAC
 */
extern void avr8_com_enc28j60_init(uint8_t *mac_addr);

/**
 * AVR8_COM API
 * Send gratuitous ARP request to teach any switches what avr8_com MAC is.
 *     mac_addr - avr8_com MAC
 */
extern void avr8_com_enc28j60_gratuitous_arp_request(uint8_t *mac_addr);

/* AVR8_COM API - Checking is link up. */
extern uint8_t avr8_com_enc28j60_is_link_up(void);

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
);

/**
 * AVR8_COM API
 * Transmitting packet, waiting for any previous transmission to end.
 *     to_mac - contains MAC of receiver
 *     len - is the length of data
 *     buff - buffer for data.
 */
extern void avr8_com_enc28j60_transmit_packet(
    uint8_t len, uint8_t *to_mac, uint8_t *buff
);

#endif
