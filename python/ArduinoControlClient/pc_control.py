import ArduinoControlClient
import time
from enum import Enum
import struct

class TLVMessageType(Enum):
    # TYPE   REQ_FORMAT   RESULT_FORMAT
    MSG_MEASURE_VOLTAGE = (ArduinoControlClient.CommonTLVMessageType.MAX+0, '', 'II')
    MSG_PC_CONTROL      = (ArduinoControlClient.CommonTLVMessageType.MAX+1, 'B', 'B')

class PC(ArduinoControlClient.NRF24Client):
    pos_power_on, pos_power_off  = 70, 75
    pos_pc_turn_on, pos_pc_turn_off, pos_pc_voltage_on = 97, 115, 130
    def __init__(self, gateway_host, gateway_port, nrf24_id):
        super(PC, self).__init__(gateway_host, gateway_port, nrf24_id)
        self.volt = [0, 0]
        self.volt_get_time = 0

    def send_recv_tlv(self, tlv, *req):
        return self.get_tlv_message_response_by_format(tlv.value[0], tlv.value[1], tlv.value[2], *req)

    def get_voltage(self):
        if self.volt_get_time+10*60 > time.time():
            return self.volt
        self.volt_get_time = time.time()
        s = self.get_servo(2)
        s.attach()
        s.write(PC.pos_pc_voltage_on)
        time.sleep(0.2)
        self.volt = self.send_recv_tlv(TLVMessageType.MSG_MEASURE_VOLTAGE)
        s.write(PC.pos_pc_turn_off)
        time.sleep(0.2)
        s.detach()
        return self.volt

    def is_pc_power_on(self):
        s = self.get_servo(3)
        return s.read() <= PC.pos_power_on

    def pc_power_on(self, on):
        s = self.get_servo(3)
        if on == self.is_pc_power_on(): return
        s.attach()
        s.write(PC.pos_power_on if on else PC.pos_power_off)
        time.sleep(0.2)
        s.detach()

    def pc_turn_on(self):
        s = self.get_servo(2)
        s.attach()
        s.write(PC.pos_pc_turn_on)
        time.sleep(0.4)
        s.write(PC.pos_pc_turn_off)
        time.sleep(0.1)
        s.detach()

b = PC('192.168.1.105', 6666, '\x3C\x99\xF9')

print b.get_this_address()
print b.get_system_uptime()
print 'volt=', b.get_voltage()

"""
b.pc_power_on(True)
time.sleep(1)
b.pc_power_on(False)
time.sleep(1)
"""
