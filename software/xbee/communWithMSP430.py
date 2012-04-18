

import serial
from array import *

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

cursorCommands = array('i',[3,4,5,6,3,4,5,6,3,4,5,6])
for i in range(0,len(cursorCommands)):
	ser = serial.Serial(2);
	numBytes = ser.write(cursorCommands[i]);
	print ser.read(numBytes)
	ser.close()


