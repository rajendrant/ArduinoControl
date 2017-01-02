import serial
import socket
import sys
import base64

def start_serial_gateway():
    dev = sys.argv[1] if len(sys.argv)>1 else '/dev/ttyUSB0'
    ser = serial.Serial(dev, 115200, timeout=1)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 6666))

    while True:
        req, address = sock.recvfrom(4096)
        if not req:
            continue
        ser.write(base64.b16encode(req))
        ser.write('\n')
        resp = ser.readline()
        sock.sendto(base64.b16decode(resp[:-2]), address)

start_serial_gateway()
