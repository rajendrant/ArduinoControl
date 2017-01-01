import serial
import socket
import sys

def start_serial_gateway():
    dev = sys.argv[1] if len(sys.argv)>1 else '/dev/ttyUSB0'
    ser = serial.Serial(dev, 115200, timeout=1)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 6666))

    while True:
        req, address = sock.recvfrom(4096)
        if not req:
            continue
        print(len(req), type(req))
        print ''.join("{:02x}".format(ord(c)) for c in req[:-1])
        ser.write(''.join("{:02x}".format(ord(c)) for c in req[:-1]))
        ser.write('\n')
        resp = ser.readline()
        print resp, len(resp)
        sock.sendto(req, address)

start_serial_gateway()
