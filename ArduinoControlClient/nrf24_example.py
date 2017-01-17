import ArduinoControlClient
import time

def ping_test(b):
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

b = ArduinoControlClient.NRF24Client('127.0.0.1', 6666, 's-1')

ping_test(b)
print b.get_this_address()
print b.get_system_uptime()
