import ArduinoControlClient
import time

def ping_test():
    b = ArduinoControlClient.UDPClient('192.168.1.105', 6666)
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

def test2():
    b = ArduinoControlClient.UDPClient('192.168.1.107', 6666)
    b.get_pin(4).mode_output()
    b.get_pin(4).digital_write(0)

def test3():
  b = ArduinoControlClient.UDPClient('192.168.1.107', 6666)
  s = b.get_servo(4)
  while True:
    s.attach()
    s.write(0)
    time.sleep(1)
    s.write(80)
    time.sleep(1)
    s.detach()
  
#ping_test()
test3()
