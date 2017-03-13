import serial
import sys
import base64
import ArduinoControlClient
import time

class SerialSocket(object):
    def __init__(self, dev):
        self.ser = serial.Serial(dev, 115200, timeout=1)
        time.sleep(1)
        
    def is_connected(self):
        return self.ser.is_open

    def send_msg(self, m):
        self.ser.write(base64.b16encode(m))
        self.ser.write('\n')

    def recv_msg(self):
        resp = self.ser.readline()
        return base64.b16decode(resp[:-2])

class SerialBoardClient(ArduinoControlClient.BoardClient):
    def __init__(self, dev):
        self.dev = dev
        super(SerialBoardClient, self).__init__()

    def init_sock(self):
        self.sock = SerialSocket(dev)
    
dev = sys.argv[1] if len(sys.argv)>1 else '/dev/ttyUSB1'
b = SerialBoardClient(dev)

def ping_test(b):
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

ping_test(b)
print b.get_this_address()
print b.get_system_uptime()
