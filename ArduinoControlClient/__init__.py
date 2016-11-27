import message_pb2
import myutil
import random
import socket
import struct
import time

class BoardClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        for tries in range(4):
            try:
                self.sock = myutil.TcpSocket(host, port)
                return
            except socket.error as e:
                print 'retrying connect'
                time.sleep(1)
                continue
        raise socket.error('Could not connect even after retries')

    def ping_test(self):
        m = message_pb2.Message()
        m.ping_test.num = random.randrange(1000000)
        resp = self.send_and_recv(m)
        return resp and resp.HasField('ping_test') and m.ping_test.num==resp.ping_test.num

    def get_pin(self, pin):
        return BoardPin(pin, self)

    def get_servo(self, pin):
        return Servo(pin, self)

    def send_and_recv(self, msg):
        for tries in range(4):
            try:
                self.send_msg(msg)
                return self.recv_msg()
            except socket.error as e:
                print 'retrying send recv'
                time.sleep(1)
                continue
        raise socket.error('send_and_recv failed')

    def send_msg(self, msg):
        data = msg.SerializeToString()
        self.sock.send(struct.pack('B', len(data)))
        self.sock.send(data)
        #print ''.join(x.encode('hex') for x in struct.pack('B', len(data)) + data)

    def recv_msg(self):
        msglen = self.sock.recvall(1, timeout=4000)
        if not msglen:
            return None
        msg = self.sock.recvall(ord(msglen), timeout=4000)
        if not msg:
            return None
        m = message_pb2.Message()
        m.ParseFromString(msg)
        return m

class BoardPin:
    def __init__(self, pin, client):
        self.pin = pin
        self.client = client
    def mode_input(self):
        self._set_mode(message_pb2.PinModeControl.INPUT)
    def mode_input_pullup(self):
        self._set_mode(message_pb2.PinModeControl.INPUT_PULLUP)
    def mode_output(self):
        self._set_mode(message_pb2.PinModeControl.OUTPUT)
    def _set_mode(self, mode):
        m = message_pb2.Message()
        m.pin_mode_control = message_pb2.PinModeControl(pin=self.pin, mode=mode)
        return self.client.send_and_recv(m)

    def digital_read(self):
        return self._io_read(message_pb2.IOReadWrite.DIGITAL_READ)
    def analog_read(self):
        return self._io_read(message_pb2.IOReadWrite.ANALOG_READ)
    def digital_write(self, val):
        return self._io_write(message_pb2.IOReadWrite.DIGITAL_WRITE, val)
    def analog_write(self):
        return self._io_write(message_pb2.IOReadWrite.ANALOG_WRITE)
    def _io_read(self, oper):
        m = self._io(oper)
        if m and m.HasField('io_read_write'):
            return m.io_read_write.val
        return -1
    def _io_write(self, oper, val):
        m = self._io(oper, val)
        return m and m.HasField('io_read_write')
    def _io(self, oper, val=0):
        m = message_pb2.Message()
        m.io_read_write.pin = self.pin
        m.io_read_write.operation = oper
        m.io_read_write.val = val
        return self.client.send_and_recv(m)

class Servo:
    def __init__(self, pin, client):
        self.pin = pin
        self.client = client
    def attach(self):
        m = self._io(message_pb2.ServoControl.ATTACH)
        return m and m.HasField('servo_control')
    def detach(self):
        m = self._io(message_pb2.ServoControl.DETACH)
        return m and m.HasField('servo_control')
    def write(self, val):
        m = self._io(message_pb2.ServoControl.WRITE, val)
        return m and m.HasField('servo_control')
    def read(self):
        m = self._io(message_pb2.ServoControl.READ)
        if m and m.HasField('servo_control'):
            return m.servo_control.val
        return -1
    def _io(self, oper, val=0):
        m = message_pb2.Message()
        m.servo_control.pin = self.pin
        m.servo_control.operation = oper
        m.servo_control.val = val
        return self.client.send_and_recv(m)

