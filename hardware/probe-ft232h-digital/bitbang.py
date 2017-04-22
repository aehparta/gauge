#!/usr/bin/env python3

import sys
import ftdi1
import time

def help(ftdi = None):
	print('Usage: %s <device-serial>' % (sys.argv[0]))

	if ftdi == None:
		sys.exit(1)

	devices = ftdi1.usb_find_all(ftdi, 0x0403, 0x7777)
	if devices[0] < 1:
		print('No connected devices found.')
		sys.exit(1)

	devices = devices[1]
	print('Devices:')
	device = devices.dev
	while device:
		ftdi1.usb_open_dev(ftdi, device)
		# if ftdi1.usb_open_dev(ftdi, device) != 0:
		# 	continue

		ftdi1.read_eeprom(ftdi)
		ftdi1.eeprom_decode(ftdi, 0)
		print('%s : %s' % (ftdi.eeprom.serial, ftdi.eeprom.product))

		device = device.next()

	sys.exit(1)

if __name__ == '__main__':
	ftdi = ftdi1.new()
	if len(sys.argv) != 2:
		help(ftdi)

	ok = ftdi1.usb_open_desc(ftdi, 0x0403, 0x7777, None, sys.argv[1])
	if ok != 0:
		print('Could not open device.')
		sys.exit(1)

	# set asynchronous bigbang mode using D0-3 as inputs and D4-7 as outputs
	ftdi1.set_bitmode(ftdi, 0xf0, 1)
	ftdi1.set_baudrate(ftdi, 10000)

	while True:
		ftdi1.write_data(ftdi, 'a', 1)
		time.sleep(0.0005)
		ftdi1.write_data(ftdi, '5', 1)
		time.sleep(0.0005)
