import struct
import ArduinoControlClient

class NRF24LowPowerSleepMessage:
    def __init__(self):
        self._fmt = 'BBBBB'
        self._mode_auto = True
        self._sleep_duration_secs = 1
        self._wake_pulse_count = 3
        self._wake_up_recv_secs = 0.004
        self._sleep_after_inactivity_secs = 1

    def mode(self, AUTO=False, MANUAL=False):
        self.mode_auto = AUTO or not MANUAL
        return self

    def sleep_duration_secs(self, x):
        self._sleep_duration_secs = x
        return self

    def wake_pulse_count(self, x):
        self._wake_pulse_count = x
        return self

    def wake_up_recv_secs(self, x):
        self._wake_up_recv_secs = x
        return self

    def sleep_after_inactivity_secs(self, x):
        self._sleep_after_inactivity_secs = x
        return self

    def _pack(self):
        return struct.pack(self._fmt, self._mode_auto, self._sleep_duration_secs*10, self._wake_pulse_count, \
                           self._wake_up_recv_secs*1000, self._sleep_after_inactivity_secs)

    def _unpack(self, msg):
        self.mode_auto, self.sleep_duration_secs, self.wake_pulse_count, self.wake_up_recv_secs, \
            self.sleep_after_inactivity_secs = struct.unpack(self._fmt, msg)

class NRF24LowPowerClient(ArduinoControlClient.NRF24Client):
    def __init__(self, gateway_host, gateway_port, nrf24_id):
        super(NRF24LowPowerClient, self).__init__(gateway_host, gateway_port, nrf24_id)
        self.low_power_mode = NRF24LowPowerSleepMessage()

    def update_low_power_mode(self):
        msg =  self.send_and_recv(ArduinoControlClient.TLVMessage( \
                   ArduinoControlClient.CommonTLVMessageType.LOW_POWER_SLEEP_MODE, \
                   self.low_power_mode._pack()))
        self.low_power_mode._unpack(msg.val)
