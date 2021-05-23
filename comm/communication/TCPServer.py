import select
import socket
from comm.communication.Communication import Communication
from comm.comm_socket.Socket import Socket
from comm.comm_socket.SocketType import SocketType
from comm.comm_socket.utils import SOCKET_ACCEPT_TIMEOUT_SECONDS
from typing import Optional


class TCPServer:
    def __init__(self, port: int, backlog: int = 5):
        self.port = port  # type: int
        self.backlog = backlog  # type: int
        self.socket = None  # type: Optional[Socket]
        self.socketAcceptTimeout = SOCKET_ACCEPT_TIMEOUT_SECONDS  # type: float
        self.initServerSocket()

    def cleanup(self):
        self.socket.cleanup()
        self.socket = None

    def acceptCommunication(self):
        try:
            read, _, _ = select.select([self.socket.getSocket()], [], [], self.socketAcceptTimeout)
            if read:
                comm = Communication()
                comm.setSocket(SocketType.TCP, self.socket.accept())
                return comm
        except socket.error as se:
            print("Socket Exception in Accept Communication:", se.errno, se.strerror)
            self.initServerSocket()
        return None

    def initServerSocket(self):
        if self.socket is not None:
            self.socket.cleanup()
            self.socket.initialize(SocketType.TCP, None, self.port)
        else:
            self.socket = Socket.Socket(SocketType.TCP, None, self.port)
        self.listen()

    def listen(self):
        self.socket.getSocket().listen(self.backlog)
