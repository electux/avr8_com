/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
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
#include <avr/pgmspace.h>
#include "avr8_com_enc28j60.h"

/* Ethernet settings. */
const uint8_t mymac[6] PROGMEM = { 0x52, 0x4f, 0x47, 0x49, 0x45, 0x52 };

/* Buffer for user data. */
#define BUFFER_SIZE 3

int main(void) {
	uint8_t j = 0;
	uint8_t reciever_len;
	uint8_t buff[BUFFER_SIZE];
	uint8_t from_to_mac[6];
	/* Set the clock pre-scaler to divide-by-1. */
	CLKPR = (1 << CLKPCE);
	CLKPR = 0;
	_delay_ms(20);
	avr8_com_enc28j60_init((uint8_t *)&mymac);
	_delay_ms(50);
	DDRC = 0; /* Whole port C is input. */
	PORTC = 0xff; /* Pull up on. */
	DDRD = 0xff; /* Whole port D is output. */
	/**
	 * Wait until link is up then send a gratuitous ARP
	 * to inform any connected switch about existence.
	 */
	while (!avr8_com_enc28j60_is_link_up())
		;
	_delay_ms(50);
	avr8_com_enc28j60_gratuitous_arp_request((uint8_t *)&mymac);
	while (1) {
		reciever_len = avr8_com_enc28j60_poll_packet(
			BUFFER_SIZE, from_to_mac, buff
		);
		if (reciever_len == 0) {
			continue;
		}
		PORTD = buff[1]; /* Set pins-port D according to user data received. */
		j++;
		/* buff[0] contains a sequence number we pass it back. */
		buff[1] = PINC; /* Read all 8 pins, PC0 to PC7. */
		buff[2] = j; /* Data for demo purposes. */
		avr8_com_enc28j60_transmit_packet(BUFFER_SIZE, from_to_mac, buff);
	}
	return (0);
}
