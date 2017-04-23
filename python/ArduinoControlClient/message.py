from enum import IntEnum
import struct

class Type(IntEnum):
    PING_TEST = 0
    PIN_MODE_CONTROL = 1
    IO_READ_WRITE = 2
    SERVO_CONTROL = 3
    THIS_ADDRESS = 4
    SYSTEM_UPTIME = 5
    TLV_MESSAGE = 6

class PingTest:
    def __init__(self, ping_test=0):
        self.ping_test = ping_test

    @staticmethod
    def unpack(buf):
        return PingTest(*struct.unpack('!I', buf))

    def pack(self):
        return struct.pack('!BI', Type.PING_TEST, self.ping_test)

class PinModeControl:
    class Mode(IntEnum):
        INPUT  = 0
        OUTPUT = 1
        INPUT_PULLUP = 2

    def __init__(self, pin=0, mode=Mode.INPUT):
        self.pin = pin
        self.mode = mode

    @staticmethod
    def unpack(buf):
        return PinModeControl(*struct.unpack('!BB', buf))

    def pack(self):
        return struct.pack('!BBB', Type.PIN_MODE_CONTROL, self.pin, self.mode)

class IOReadWrite:
    class Operation(IntEnum):
        DIGITAL_READ  = 0
        DIGITAL_WRITE = 1
        ANALOG_READ   = 2
        ANALOG_WRITE  = 3

    def __init__(self, pin=0, operation=Operation.DIGITAL_READ, val=0):
        self.pin = pin
        self.operation = operation
        self.val = val

    @staticmethod
    def unpack(buf):
        return IOReadWrite(*struct.unpack('!BBB', buf))

    def pack(self):
        return struct.pack('!BBBB', Type.IO_READ_WRITE, self.pin, self.operation, self.val)

class ServoControl:
    class Operation(IntEnum):
        ATTACH = 0
        DETACH = 1
        WRITE  = 2
        READ   = 3
    
    def __init__(self, pin=0, operation=Operation.ATTACH, val=0):
        self.pin = pin
        self.operation = operation
        self.val = val

    @staticmethod
    def unpack(buf):
        return ServoControl(*struct.unpack('!BBB', buf))

    def pack(self):
        return struct.pack('!BBBB', Type.SERVO_CONTROL, self.pin, self.operation, self.val)

class ThisAddress:
    def __init__(self, this_address=0):
        self.this_address = this_address

    @staticmethod
    def unpack(buf):
        return ThisAddress(*struct.unpack('!I', buf))

    def pack(self):
        return struct.pack('!BI', Type.THIS_ADDRESS, self.this_address)

class SystemUptime:
    def __init__(self, system_uptime=0):
        self.system_uptime = system_uptime

    @staticmethod
    def unpack(buf):
        return SystemUptime(*struct.unpack('!I', buf))

    def pack(self):
        return struct.pack('!BI', Type.SYSTEM_UPTIME, self.system_uptime)

class TLVMessage:
    def __init__(self, typee, val):
        assert typee <= 0xF
        self.typee=typee
        self.val=val

    @staticmethod
    def unpack(msgtype, buf):
        return TLVMessage((msgtype&0xF0)>>4, buf)

    def pack(self):
        return struct.pack('!B', Type.TLV_MESSAGE|(self.typee<<4))+self.val

class CommonTLVMessageType(IntEnum):
    LOW_POWER_SLEEP_MODE = 0
    LOW_POWER_WAKE_PULSE = 1
    MAX = LOW_POWER_WAKE_PULSE+1

class GatewayMessageHeader:
    def __init__(self, address, send_retry, recv_retry, msg):
        self.address = address + '\x00'
        self.retry = (send_retry<<4) | recv_retry
        self.msg = msg

    def pack(self):
        return self.address + struct.pack('B', self.retry) + self.msg.pack()

def unpack(buf):
    msgtype = struct.unpack('!B', buf[0])[0]
    if msgtype == Type.PING_TEST:
        return PingTest.unpack(buf[1:])
    elif msgtype == Type.PIN_MODE_CONTROL:
        return PinModeControl.unpack(buf[1:])
    elif msgtype == Type.IO_READ_WRITE:
        return IOReadWrite.unpack(buf[1:])
    elif msgtype == Type.SERVO_CONTROL:
        return ServoControl.unpack(buf[1:])
    elif msgtype == Type.THIS_ADDRESS:
        return ThisAddress.unpack(buf[1:])
    elif msgtype == Type.SYSTEM_UPTIME:
        return SystemUptime.unpack(buf[1:])
    elif (msgtype&0xF) == Type.TLV_MESSAGE:
        return TLVMessage.unpack(msgtype, buf[1:])
    return None
