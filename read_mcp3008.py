#!/usr/bin/env python3
import time
import spidev

spi = spidev.SpiDev()
spi.open (0, 0)
spi.max_speed_hz = 1000000

def read_adc(channel):
	adc = spi.xfer2 ([1, (8+channel) << 4, 0])
	data = ((adc[1] & 3) << 8) + adc[2]
	return data

while True:
	time.sleep(0.2)
	print(read_adc(0))
