import serial
import time

# scan for a ailable serial ports
def scan():
    """
        scan for available ports
        return a list of tuples (port num, name)
    """

    available = []

    for i in range(256):
        try:
            s = serial.Serial(i)
            available.append((i,s.portstr))
            s.close()

        except serial.SerialException:
            pass

    return available

ports = scan()
print ports
