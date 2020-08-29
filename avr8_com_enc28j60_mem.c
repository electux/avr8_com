/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_mem.c
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
 * Read from Buffer Memory.
 *     len - length of data
 *     data - pointer to data
 */
void avr8_com_enc28j60_read_buffer(uint8_t len, uint8_t *data) {
	CSACTIVE;
	SPDR = ENC28J60_READ_BUF_MEM;
	waitspi();
	while (len) {
		len--;
		SPDR = 0x00;
		waitspi();
		*data = SPDR;
		data++;
	}
	CSPASSIVE;
}

/**
 * AVR8_COM API
 * Write to Buffer Memory.
 *     len - length of data
 *     data - pointer to data
 */
void avr8_com_enc28j60_write_buffer(uint8_t len, uint8_t *data) {
	CSACTIVE;
	SPDR = ENC28J60_WRITE_BUF_MEM;
	waitspi();
	while (len) {
		len--;
		SPDR = *data;
		data++;
		waitspi();
	}
	CSPASSIVE;
}
