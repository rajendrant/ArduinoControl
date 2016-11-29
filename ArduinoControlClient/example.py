import ArduinoControlClient
import time

def ping_test():
    b = ArduinoControlClient.BoardClient('192.168.1.105', 6666)
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

def test():
    b = ArduinoControlClient.BoardClient('192.168.1.105', 6666)
    s.attach()
    s.write(170)
    time.sleep(0.5)
    s.write(0)
    time.sleep(0.5)
    s.detach()

def test2():
    b = ArduinoControlClient.BoardClient('192.168.1.105', 6666)
    b.get_pin(4).digital_write(0)
    b.get_pin(5).digital_write(0)
    b.get_pin(12).digital_write(0)
    b.get_pin(13).digital_write(0)

def test3():
    b = ArduinoControlClient.BoardClient('192.168.1.105', 6666)
    s=b.get_servo(14)
    s.attach()
    s.write(140)
    time.sleep(0.5)
    s.write(180)
    time.sleep(0.5)
    s.detach()

ping_test()
