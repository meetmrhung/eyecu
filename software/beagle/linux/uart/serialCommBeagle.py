# File: serialCommBeagle.py
#
# Author: Mike Mozingo
#
# Description: Demonstrates usign python to communicate over UART1 on the beagle bone

import serial, os
import sys

PORT = '/dev/ttyO1'
BAUD_RATE = 9600

# enable the pins for the uart. Only uart0 is enabled by default
uart1_pin_mux = [('uart1_rxd', (0|(1<<5))),('uart1_txd',(0)),]

# write pin settings to file
for (fname, mode) in uart1_pin_mux:
	with open(os.path.join('/sys/kernel/debug/omap_mux',fname),'wb') as f:
		f.write("%X" % mode)

# open the serial port
ser = serial.Serial(PORT,BAUD_RATE)
count = 0
while True:
	# try to read
	a = ser.read()
#	print count, a
	# echo what we just read
	ser.write(a)
	count += 1
