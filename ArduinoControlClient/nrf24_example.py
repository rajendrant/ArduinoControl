import ArduinoControlClient
import time

def ping_test():
    b = ArduinoControlClient.BoardClient('127.0.0.1', 6666)
    for i in range(4):
        latency = b.ping_test()
        print 'ping_test', 'PASSED latency=%d'%(latency) if latency else 'FAILED'
        time.sleep(0.5)

ping_test()
