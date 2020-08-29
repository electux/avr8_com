/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * avr8_com_eth.h
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

#ifndef AVR8_COM_ETH_H
#define AVR8_COM_ETH_H

#define AVR8_COM_ETH_HEADER_LEN 14
#define AVR8_COM_ETH_CHECKSUM_LEN 4

#define AVR8_COM_ETH_ENVELOPE_LEN \
	(AVR8_COM_ETH_HEADER_LEN + AVR8_COM_ETH_CHECKSUM_LEN)

#define AVR8_COM_ETH_PAYLOAD_MIN 46
#define AVR8_COM_ETH_PAYLOAD_MAX 1500
#define AVR8_COM_ETH_PACKET_MIN 64
#define AVR8_COM_ETH_PACKET_MAX 1518
#define AVR8_COM_ETH_DST_MAC_P 0
#define AVR8_COM_ETH_SRC_MAC_P 6
#define AVR8_COM_ETH_TYPE_H_P 12
#define AVR8_COM_ETH_TYPE_L_P 13

#endif
