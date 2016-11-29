import socket
import select
import struct
import time

def logger(fn):
    from functools import wraps
    import inspect
    @wraps(fn)
    def wrapper(*args, **kwargs):
        #print time.time(), 'START', fn.__name__, '\r'
        out = apply(fn, args, kwargs)
        #print time.time(), 'END', fn.__name__, '\r'
        return out
    return wrapper


class TcpSocket(object):
    def __init__(self, host, port):
        self.sock = socket.create_connection((host, port), 2000)
        self.poll = select.poll()
        self.poll.register(self.sock, select.POLLIN | select.POLLPRI | select.POLLHUP |
                           select.POLLERR | select.POLLNVAL)
        self.rd_buf = b''
        self._is_err = False

    def __del__(self):
        self.shutdown_and_close()

    def is_connected(self):
        if self._is_err:
            return False
        p = self.poll.poll(1)
        if p and p[0][1] & (select.POLLHUP | select.POLLERR):
            self._is_err = True
        return not self._is_err

    def send(self, data):
        """
        Returns None on success
        """
        if not self.is_connected():
            return True
        return self.sock.sendall(data)

    def recvall(self, count, timeout=2000):
        start = time.time()
        while len(self.rd_buf) < count:
            newbuf = self._recv(count-len(self.rd_buf), timeout)
            if not newbuf:
                end = time.time()
                if (end - start)*1000 < timeout and (end - start)*1000 < 1000:
                    # Wierd that poll() did not detect this sock failure.
                    self._is_err = True
                self.rd_buf = b''
                return None
            self.rd_buf += newbuf
        assert len(self.rd_buf) == count
        buf = self.rd_buf
        self.rd_buf = b''
        return buf

    def _recv(self, count, timeout):
        while self.poll.poll(timeout) and self.is_connected():
            return self.sock.recv(count)
        return None

    def shutdown_and_close(self):
        self._is_err = True
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()


class TcpSocketLineReader(TcpSocket):
    def __init__(self, host, port):
        super(TcpSocketLineReader, self).__init__(host, port)
        self.rd_record = []
        self.rd_buf = ''

    def clear_buffer(self):
        del self.rd_record[:]
        self.rd_buf = ''

    def read_record(self, timeout):
        delimiter = '\n'
        if self.rd_record:
            return self.rd_record.pop(0)
        while self.poll.poll(timeout) and self.is_connected():
            d = self.sock.recv(65536)
            if not d:
                self._is_err = True
                break
            self.rd_buf += d
            if self.rd_buf.endswith(delimiter):
                break
        split = self.rd_buf.split(delimiter)
        self.rd_record.extend(split[:-1])
        self.rd_buf = split[-1]
        if self.rd_record:
            return self.rd_record.pop(0)


class TcpSocketMessageReader(TcpSocket):
    """
    http://stupidpythonideas.blogspot.com/2013/05/sockets-are-byte-streams-not-message.html
    """
    def __init__(self, host, port):
        super(TcpSocketMessageReader, self).__init__(host, port)

    def read_message(self, timeout=2000):
        """
        Returns tuple of (msgtype, msgdata) on success. On failure None is returned.
        A single message is defined as 6_byte_header + message_data.
        The header has 2_byte_message_type + 4_byte_message_length.
        If the mesage is not received within timeout, None will be returned.
        """
        hdr = self.recvall(6, timeout)
        if not hdr:
            return None
        msgtype, msglen = struct.unpack('!H I', hdr)
        msgdata = self.recvall(msglen, timeout)
        if not msgdata:
            return None
        return msgtype, msgdata

