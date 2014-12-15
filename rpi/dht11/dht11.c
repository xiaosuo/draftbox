/**
 * DHT11 driver for Raspberry PI in userspace.
 * Copyright (C) 2014 Changli Gao <xiaosuo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <bcm2835.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sched.h>

#include <sys/param.h>

#ifndef DHT11_IO_PIN
#define DHT11_IO_PIN RPI_GPIO_P1_07
#endif /* DHT11_IO_PIN */

#define DHT11_MAX_WIDTH 1000
#define DHT11_ONE_LOW_WIDTH 200

struct dht11_data {
	uint8_t	humidity;
	uint8_t	temperature;
};

static int dht11_level_width(int level)
{
	int width, times;

	times = 0;
	width = 0;
	while (true) {
		if (bcm2835_gpio_lev(DHT11_IO_PIN) != level) {
			if (++times > 1)
				break;
		} else {
			times = 0;
		}
		if (++width >= DHT11_MAX_WIDTH)
			break;
	}

	return width;
}

#ifndef NDEBUG
static int bit_index = 0;
static int bits[40] = { 0 };
static int bits_low[40] = { 0 };
static void print_bits(void)
{
	int i;

	for (i = 0; i < 40; i++)
		printf("%d: %d, %d\n", i, bits[i], bits_low[i]);
}
#endif

static int dht11_read_bit()
{
	int width;

	width = dht11_level_width(LOW);
#ifndef NDEBUG
	bits_low[bit_index] = width;
#endif
	if (width >= DHT11_MAX_WIDTH)
		return -1;
	width = dht11_level_width(HIGH);
#ifndef NDEBUG
	bits[bit_index++] = width;
#endif
	if (width >= DHT11_MAX_WIDTH)
		return -1;
	else if (width < DHT11_ONE_LOW_WIDTH)
		return 0;
	else
		return 1;
}

static int dht11_ack()
{
	dht11_level_width(HIGH);
	if (dht11_level_width(LOW) >= DHT11_MAX_WIDTH ||
	    dht11_level_width(HIGH) >= DHT11_MAX_WIDTH)
		return -1;

	return 0;
}

static void dht11_start()
{
	bcm2835_gpio_fsel(DHT11_IO_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(DHT11_IO_PIN, LOW);
	delay(20);
	bcm2835_gpio_write(DHT11_IO_PIN, HIGH);
	bcm2835_gpio_fsel(DHT11_IO_PIN, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(DHT11_IO_PIN, BCM2835_GPIO_PUD_UP);
}

static uint8_t dht11_checksum(uint8_t result[5])
{
	return result[1] + result[2] + result[3] + result[4];
}

int dht11_measure(struct dht11_data *data)
{
	uint8_t result[5];
	int i, bit;

	dht11_start();
	if (dht11_ack() < 0)
		return -1;
	for (i = 39; i >= 0; --i) {
		bit = dht11_read_bit();
		switch (bit) {
		case 1:
			setbit(result, i);
			break;
		case 0:
			clrbit(result, i);
			break;
		default:
			return -1;
		}
	}
	if (dht11_checksum(result) != result[0])
		return -1;
	data->humidity = result[4];
	data->temperature = result[2];

	return 0;
}

int main(int argc, char *argv[])
{
	struct dht11_data data;
	struct sched_param param;

#ifndef NDEBUG
	atexit(print_bits);
#endif
	if (bcm2835_init() == -1)
		return EXIT_FAILURE;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sched_setscheduler(0, SCHED_FIFO, &param))
		return EXIT_FAILURE;
	if (dht11_measure(&data) < 0)
		return EXIT_FAILURE;
	printf("Humidity: %hhu\n",  data.humidity);
	printf("Temperature: %hhu\n",  data.temperature);

	return EXIT_SUCCESS;
}
