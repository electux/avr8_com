/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_reg.c
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

/**
 * AVR8_COM API
 * Read a control register in controller assuming
 * that currently selected bank is correct.
 *     op - SPI operation
 *     address - address
 */
uint8_t avr8_com_enc28j60_read_op(uint8_t op, uint8_t address) {
	CSACTIVE;
	SPDR = op | (address & ADDR_MASK);
	waitspi();
	SPDR = 0x00;
	waitspi();
	/* Do dummy read if needed (for MAC and MII). */
	if (address & 0x80) {
		SPDR = 0x00;
		waitspi();
	}
	CSPASSIVE;
	return (SPDR);
}

/**
 * AVR8_COM API
 * Write a control register in controller assuming
 * that currently selected bank is correct.
 *     op - SPI operation
 *     address - address
 *     data - data
 */
void avr8_com_enc28j60_write_op(uint8_t op, uint8_t address, uint8_t data) {
	CSACTIVE;
	SPDR = op | (address & ADDR_MASK);
	waitspi();
	SPDR = data;
	waitspi();
	CSPASSIVE;
}
