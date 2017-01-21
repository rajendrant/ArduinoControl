from message import *
import myutil
import random
import socket
import struct
import time

class BoardClient(object):
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.init_sock()

    def init_sock(self):
        raise NotImplementedError

    def ping_test(self):
        """
        Sends a random number in the ping request and expects the same number in
        the ping response. Returns None on failure. On success returns the
        latency time in milliseconds.
        """
        start_time=time.time()
        m = PingTest(random.randrange(1000000))
        resp = self.send_and_recv(m)
        if resp and m.ping_test==resp.ping_test:
            return (time.time()-start_time)*1000
        return None

    def get_pin(self, pin):
        return BoardPin(pin, self)

    def get_servo(self, pin):
        return Servo(pin, self)

    def get_this_address(self):
        m = ThisAddress(0)
        resp = self.send_and_recv(m)
        if resp:
            return resp.this_address
        return 0
    
    def get_system_uptime(self):
        m = SystemUptime(0)
        resp = self.send_and_recv(m)
        if resp:
            return resp.system_uptime
        return 0

    def send_and_recv(self, msg):
        for connect_tries in range(3):
            if not self.sock.is_connected():
                try:
                    self.init_sock()
                except socket.error as e:
                    continue
            if self.sock.is_connected():
                for send_tries in range(2):
                    try:
                        self.send_msg(msg)
                        return self.recv_msg()
                    except socket.error as e:
                        print 'retrying send recv'
                        time.sleep(0.1)
                        continue
        raise socket.error('send_and_recv failed')

    def send_msg(self, msg):
        self.sock.send_msg(msg.pack())

    def recv_msg(self):
        msg = self.sock.recv_msg()
        if not msg:
            return None
        return unpack(msg)

class TCPClient(BoardClient):
    def __init__(self, host, port):
        super(TCPClient, self).__init__(host, port)

    def init_sock(self):
        for tries in range(4):
            try:
                self.sock = myutil.TcpSocket(self.host, self.port)
                return
            except socket.error as e:
                print 'retrying connect'
                time.sleep(0.1)
                continue
        raise socket.error('Could not connect even after retries')

class UDPClient(BoardClient):
    def __init__(self, host, port):
        super(UDPClient, self).__init__(host, port)

    def init_sock(self):
        self.sock = myutil.UdpSocket(self.host, self.port)

class NRF24Client(UDPClient):
    def __init__(self, gateway_host, gateway_port, nrf24_id):
        super(NRF24Client, self).__init__(gateway_host, gateway_port)
        self.nrf24_id = nrf24_id

    def send_msg(self, msg):
        self.sock.send_msg(self.nrf24_id + msg.pack())

class BoardPin:
    def __init__(self, pin, client):
        self.pin = pin
        self.client = client
    def mode_input(self):
        self._set_mode(PinModeControl.Mode.INPUT)
    def mode_input_pullup(self):
        self._set_mode(PinModeControl.Mode.INPUT_PULLUP)
    def mode_output(self):
        self._set_mode(PinModeControl.Mode.OUTPUT)
    def _set_mode(self, mode):
        return self.client.send_and_recv(PinModeControl(pin=self.pin, mode=mode))

    def digital_read(self):
        return self._io_read(IOReadWrite.Operation.DIGITAL_READ)
    def analog_read(self):
        return self._io_read(IOReadWrite.Operation.ANALOG_READ)
    def digital_write(self, val):
        return self._io_write(IOReadWrite.Operation.DIGITAL_WRITE, val)
    def analog_write(self):
        return self._io_write(IOReadWrite.Operation.ANALOG_WRITE)
    def _io_read(self, oper):
        m = self._io(oper)
        if m:
            return m.io_read_write.val
        return -1
    def _io_write(self, oper, val):
        m = self._io(oper, val)
        return not not m
    def _io(self, oper, val=0):
        return self.client.send_and_recv(IOReadWrite(pin=self.pin, operation=oper, val=val))

class Servo:
    def __init__(self, pin, client):
        self.pin = pin
        self.client = client
    def attach(self):
        m = self._io(ServoControl.Operation.ATTACH)
        return not not m
    def detach(self):
        m = self._io(ServoControl.Operation.DETACH)
        return not not m
    def write(self, val):
        m = self._io(ServoControl.Operation.WRITE, val)
        return not not m
    def read(self):
        m = self._io(ServoControl.Operation.READ)
        if m:
            return m.servo_control.val
        return -1
    def _io(self, oper, val=0):
        return self.client.send_and_recv(ServoControl(pin=self.pin, operation=oper, val=val))
