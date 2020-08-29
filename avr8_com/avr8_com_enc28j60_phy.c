/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_enc28j60_phy.c
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

/**
 * AVR8_COM API
 * Read upper 8 bits of a PHY register.
 *     address - address
 */
uint8_t avr8_com_enc28j60_phy_read_high(uint8_t address) {
	/* Set right address and start register read operation. */
	avr8_com_enc28j60_set_bank(2);
	avr8_com_enc28j60_write_reg(MIREGADR, address);
	avr8_com_enc28j60_write_reg(MICMD, MICMD_MIIRD);
	_delay_us(15); /* Wait until PHY read completes. */
	avr8_com_enc28j60_set_bank(3);
	while (avr8_com_enc28j60_read_reg(MISTAT) & MISTAT_BUSY)
		;
	/* Reset reading bit. */
	avr8_com_enc28j60_set_bank(2);
	avr8_com_enc28j60_write_reg(MICMD, 0x00);
	return (avr8_com_enc28j60_read_reg(MIRDH));
}

/**
 * AVR8_COM API
 * Write a PHY register.
 *     address - address
 *     data - data
 */
void avr8_com_enc28j60_phy_write(uint8_t address, uint16_t data) {
	avr8_com_enc28j60_set_bank(2);
	/* Set the PHY register address. */
	avr8_com_enc28j60_write_reg(MIREGADR, address);
	/* Write the PHY data. */
	avr8_com_enc28j60_write_reg(MIWRL, data);
	avr8_com_enc28j60_write_reg(MIWRH, data >> 8);
	/* Wait until the PHY write completes. */
	avr8_com_enc28j60_set_bank(3);
	while (avr8_com_enc28j60_read_reg(MISTAT) & MISTAT_BUSY) {
		_delay_us(15);
	}
}
