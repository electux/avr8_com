# subdir.mk
# Copyright (C) 2020 Vladimir Roncevic <elektron.ronca@gmail.com>
#
# avr8_com is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# avr8_com is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program_name.  If not, see <http://www.gnu.org/licenses/>.

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../avr8_com_enc28j60.c \
../avr8_com_enc28j60_arp.c \
../avr8_com_enc28j60_mem.c \
../avr8_com_enc28j60_phy.c \
../avr8_com_enc28j60_reg.c \
../avr8_com_enc28j60_rx.c \
../avr8_com_enc28j60_tx.c \
../main.c 

OBJS += \
./avr8_com_enc28j60.o \
./avr8_com_enc28j60_arp.o \
./avr8_com_enc28j60_mem.o \
./avr8_com_enc28j60_phy.o \
./avr8_com_enc28j60_reg.o \
./avr8_com_enc28j60_rx.o \
./avr8_com_enc28j60_tx.o \
./main.o 

C_DEPS += \
./avr8_com_enc28j60.d \
./avr8_com_enc28j60_arp.d \
./avr8_com_enc28j60_mem.d \
./avr8_com_enc28j60_phy.d \
./avr8_com_enc28j60_reg.d \
./avr8_com_enc28j60_rx.d \
./avr8_com_enc28j60_tx.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


