import ArduinoControlClient
import time

def ping_test(b):
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

#b = ArduinoControlClient.NRF24Client('127.0.0.1', 6666, 's-1')
#b = ArduinoControlClient.NRF24Client('192.168.1.105', 6666, '\xB5\xDE\xDB')
#b = ArduinoControlClient.NRF24Client('192.168.1.105', 6666, 'MUX')
#b = ArduinoControlClient.NRF24Client('192.168.1.105', 6666, 'GAT')
b = ArduinoControlClient.NRF24Client('192.168.1.105', 6666, 's-1')
#b = ArduinoControlClient.NRF24Client('192.168.1.105', 6666, '\x3C\x99\xF9')

b.set_low_power_mode(False)
ping_test(b)
print b.get_this_address()
print b.get_system_uptime()
b.set_low_power_mode(True)
