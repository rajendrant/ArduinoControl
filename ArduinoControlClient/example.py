import ArduinoControlClient
import time

b = ArduinoControlClient.UDPClient('192.168.1.106', 6666)

def ping_test(b):
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

def blink_led(b):
    pin=b.get_pin(2)
    pin.mode_output()
    pin.digital_write(0)
    time.sleep(1)
    pin.digital_write(1)

def t(b):
    pin=b.get_pin(14)
    pin.mode_input_pullup()
    time.sleep(5)
    pin.mode_output()

def test():
    b = ArduinoControlClient.UDPClient('192.168.1.105', 6666)
    s.attach()
    s.write(170)
    time.sleep(0.5)
    s.write(0)
    time.sleep(0.5)
    s.detach()

def test2():
    b = ArduinoControlClient.UDPClient('192.168.1.105', 6666)
    b.get_pin(4).digital_write(0)
    b.get_pin(5).digital_write(0)
    b.get_pin(12).digital_write(0)
    b.get_pin(13).digital_write(0)

def test3():
    b = ArduinoControlClient.UDPClient('192.168.1.105', 6666)
    s=b.get_servo(14)
    s.attach()
    s.write(140)
    time.sleep(0.5)
    s.write(180)
    time.sleep(0.5)
    s.detach()

ping_test(b)
print b.get_this_address()
print b.get_system_uptime()
blink_led(b)
while True:
  print b.get_system_uptime()
  t(b)
  time.sleep(3)
