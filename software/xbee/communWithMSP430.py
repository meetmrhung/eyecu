#! /usr/bin/env python
"""\
Scan for serial ports.

Part of pySerial (http://pyserial.sf.net)
(C) 2002-2003 <cliechti@gmx.net>

The scan function of this module tries to open each port number
from 0 to 255 and it builds a list of those ports where this was
successful.
"""

import serial
from array import *
import time
import cursorCommand

# scans the available ports
def scan():
    """scan for available ports. return a list of tuples (num, name)"""
    available = []
    for i in range(256):
        try:
            s = serial.Serial(i)
            available.append( (i, s.portstr))
            s.close()   # explicit close 'cause of delayed GC in java
        except serial.SerialException:
            pass
    return available

if __name__=='__main__':
    print "Found ports:"
    for n,s in scan():
        print "(%d) %s" % (n,s)

totalBytes = 0;
Commands = array('i',[0, 0, 1, 1 ,1 , 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4])

# Define serial port communication with particular baudrate and timeout (defining how long we wait to read the port before program quits) 
ser = serial.Serial(n,baudrate = 9600,timeout = 1);

#Writing cursor commands into port
for i in range(0,len(Commands)):
	numBytes = ser.write(Commands[i]);
	totalBytes = totalBytes+numBytes;
	
#reading port and calling function to generate cursor command	
for j in range(0,totalBytes):
	value =  ser.read(1);
	cursorCommand.result(value);
ser.close()	

	
	
	


