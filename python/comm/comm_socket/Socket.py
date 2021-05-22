import socket
import struct
from comm.comm_socket.network_includes import *
from comm.comm_socket.SocketPartner import SocketPartner
from comm.comm_socket.SocketType import SocketType
from comm.comm_socket.utils import prepareBuffer, SOCKET_BUFFER_RECV_SIZE, SOCKET_BUFFER_SEND_SIZE, \
    CLIENT_MAX_MESSAGE_BYTES, comparePartners
from comm.comm_utils.Buffer import Buffer
from comm.comm_utils.SerializationHeader import SerializationHeader
from comm.comm_utils.utils import memcpy
from typing import Optional, Tuple


def printSocketDetails(s: socket.socket):
    partner = s.getpeername()
    myself = s.getsockname()
    print("Socket partner: ", partner[0], ":", partner[1], " local addr.: ", myself[0], ":", myself[1], sep="")


class Socket:
    @staticmethod
    def Socket(protocol: SocketType, partner: SocketPartner = None, myPort: int = 0, sendTimeout: int = -1,
               recvTimeout: int = -1):
        new = Socket()
        int(0)
        new.initialize(protocol, partner, myPort, sendTimeout, recvTimeout)
        return new

    def __init__(self):
        self.protocol = SocketType.UDP
        self.socket = None  # type: Optional[socket.socket]
        self.partner = None  # type: Optional[SocketPartner]
        self.myself = None  # type: Optional[SocketPartner]
        self.sendTimeout = -1  # type: int
        self.recvTimeout = -1  # type: int
        self.isCopy = False  # type: bool
        self.initialized = False  # type: bool
        self.recvAddress = None  # type: Optional[Tuple[str, int]]
        self.sendBuffer = Buffer(CLIENT_MAX_MESSAGE_BYTES + 4)  # type: Buffer
        self.recvBuffer = Buffer(CLIENT_MAX_MESSAGE_BYTES + 4)  # type: Buffer

    def getSocket(self):
        return self.socket

    def cleanup(self, withBuffers: bool = False):
        self.close()
        self.partner = None
        self.myself = None
        self.isCopy = False
        self.initialized = False
        if withBuffers:
            self.sendBuffer = None
            self.recvBuffer = None

    def close(self):
        if self.socket:
            self.socket.close()
        self.socket = None

    def copy(self):
        copy = Socket()
        assert (copy.socket is None)
        copy.protocol = self.protocol
        copy.socket = self.socket

        copy.partner = self.partner.copy()
        copy.myself = self.myself.copy()

        copy.sendTimeout = self.sendTimeout
        copy.recvTimeout = self.recvTimeout

        copy.recvAddress = self.recvAddress

        copy.sendBuffer = self.sendBuffer.copy() if (self.sendBuffer is not None) else None
        copy.recvBuffer = self.recvBuffer.copy() if (self.recvBuffer is not None) else None

        copy.isCopy = True
        copy.initialized = True
        return copy

    def initialize_socket(self):
        self.createSocket()
        self.initMyself()

        if self.partner is not None:
            if self.protocol == SocketType.TCP:
                print("Socket Initialization: connecting tcp socket...")
                try:
                    self.socket.connect(self.partner.getPartner())
                except socket.error as se:
                    print("Caught exception:", se.errno, se.strerror, "when connecting to TCP")
                    print("Connection failed due to port and ip problems")
                    if se.errno in connection_refused:
                        # Connection refused
                        raise se
                    print("SocketException : " + str(se))
                    return False
                print("Socket Initialization: connected tcp socket to", self.partner.getPartnerString())

        Socket._setSocketTimeouts(self.socket, self.sendTimeout, self.recvTimeout)
        # printSocketDetails(self.socket)
        self.initialized = True
        return True

    def initialize(self, _protocol: SocketType, _partner: SocketPartner = None, _myPort: int = 0,
                   _sendTimeout: int = -1, _recvTimeout: int = -1):
        self.cleanup()
        self.protocol = _protocol
        self.partner = _partner
        if _myPort < 0 or _myPort >= 2 ** 16:
            _myPort = 0
        self.myself = SocketPartner.SocketPartner(("0.0.0.0", _myPort), False)
        self.sendTimeout = _sendTimeout
        self.recvTimeout = _recvTimeout
        self.isCopy = False
        self.initialized = False
        if self.sendBuffer is None:
            self.sendBuffer = Buffer(CLIENT_MAX_MESSAGE_BYTES + 4)
        if self.recvBuffer is None:
            self.recvBuffer = Buffer(CLIENT_MAX_MESSAGE_BYTES + 4)
        return self.initialize_socket()

    def setSocketTimeouts(self, _sendTimeout: int = -1, _recvTimeout: int = -1, modifySocket: bool = True):
        if _sendTimeout > 0:
            self.sendTimeout = _sendTimeout

        if _recvTimeout > 0:
            self.recvTimeout = _recvTimeout

        if modifySocket and self.initialized:
            Socket._setSocketTimeouts(self.socket, self.sendTimeout, self.recvTimeout)

    def setOverwritePartner(self, overwrite: bool):
        if self.isCopy:
            print("Can not change the partner of a copy-socket!")
            return

        if self.partner is None:
            self.partner = SocketPartner(False, False)

        self.partner.setOverwrite(overwrite)

    def setPartner(self, _partner: SocketPartner, overwrite: bool):
        if self.isCopy:
            print("Can not change the partner of a copy-socket!")
            return

        self.partner = _partner
        self.partner.setOverwrite(overwrite)

    def getPartner(self):
        return self.partner

    def getMyself(self):
        return self.myself

    def isInitialized(self):
        return self.initialized

    def accept(self, verbose: bool = False):
        acceptSocket = Socket.tcp_socket_with_partner(SocketPartner(False, True))
        print("Socket Initialization: accepting tcp connection...", acceptSocket.partner.getPartnerString())
        try:
            acceptSocket.socket, partner = self.socket.accept()
            acceptSocket.setPartner(SocketPartner.SocketPartner(partner, False), False)
            if verbose:
                print("Found connection...")
        except Exception:
            print("ERROR on accept")
            raise RuntimeError("Error on accept socket...")

        acceptSocket.initMyself(False)
        print("Socket Initialization: accepted tcp connection from", acceptSocket.partner.getPartnerString())
        # printSocketDetails(acceptSocket.socket)

        Socket._setSocketBufferSizes(acceptSocket.socket)
        acceptSocket.initialized = True
        return acceptSocket

    def sendBytes(self, buffer: Buffer or Tuple[bytes, int], errorCode: int,
                  header: Optional[SerializationHeader] = None, retries: int = 0,
                  verbose: bool = False) -> Tuple[bool, int]:
        if isinstance(buffer, tuple):
            return self._sendBytes(buffer[0], buffer[1], errorCode, header, retries, verbose)
        else:
            return self._sendBytes(buffer.getBuffer(), buffer.getBufferContentSize(), errorCode, header, retries,
                                   verbose)

    def receiveBytes(self, buffer: Buffer or Tuple[bytes, int, int], errorCode: int,
                     expectedHeader: Optional[SerializationHeader] = None, retries: int = 0,
                     verbose: bool = False) -> Tuple[bool, Buffer, int] or Tuple[bytes, Tuple[bytes, int], int]:
        if isinstance(buffer, tuple):
            dataBuffer, bufferLength = prepareBuffer(buffer[0], buffer[1], buffer[2])
            success, dataBuffer, errorCode = self._receiveBytes(dataBuffer, buffer[2], errorCode, expectedHeader,
                                                                retries, verbose)
            return success, (dataBuffer, bufferLength), errorCode
        else:
            # print("Expecting " + str(buffer.getBufferContentSize()) + "bytes...")
            success, dataBuffer, errorCode = self._receiveBytes(buffer.getBuffer(), buffer.getBufferContentSize(),
                                                                errorCode, expectedHeader, retries, verbose)
            # print("Expected", buffer.getBufferContentSize(), "bytes, got", len(strToCStr(dataBuffer)))
            buffer.buffer = dataBuffer
            return success, buffer, errorCode

    @staticmethod
    def _setSocketBufferSizes(sock: socket.socket):
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SOCKET_BUFFER_RECV_SIZE)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SOCKET_BUFFER_SEND_SIZE)
        print("Socket send buffer size:", sock.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF))
        print("Socket recv buffer size:", sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF))

    @staticmethod
    def _setSocketTimeouts(sock: socket.socket, sendTimeout: int = -1, recvTimeout: int = -1):
        if sendTimeout > 0:
            if os == "Windows":
                timeval = sendTimeout
            else:
                timeval = struct.pack('ll', 0, 1000 * sendTimeout)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)
            print("Socket send timeout:", sock.getsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO))
        if recvTimeout > 0:
            if os == "Windows":
                timeval = recvTimeout
            else:
                timeval = struct.pack('ll', 0, 1000 * recvTimeout)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
            print("Socket recv timeout:", sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO))

    @staticmethod
    def tcp_socket_with_partner(partner: SocketPartner):
        new = Socket()
        new.protocol = SocketType.TCP
        new.partner = SocketPartner.SocketPartner((partner.getIP(), partner.getPort()), partner.getOverwrite())
        return new

    def createSocket(self):
        if self.socket is not None:
            self.close()

        if self.protocol == SocketType.UDP or self.protocol == SocketType.UDP_HEADER:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
        elif self.protocol == SocketType.TCP:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        else:
            raise RuntimeError("Unknown protocol: " + str(self.protocol))

        if self.socket is None:
            print("Socket not created")
            raise RuntimeError("Socket not created!")

        Socket._setSocketBufferSizes(self.socket)

    def initMyself(self, withBind: bool = True):
        if withBind:
            print("Socket Initialization: binding socket to local ", self.myself.getPartnerString(), "...", sep="")
            try:
                self.socket.bind(self.myself.getPartner())
            except Exception as e:
                print("ERROR on binding:", e)
                raise RuntimeError("ERROR binding server socket")

        if self.myself is None or self.myself.getPort() == 0:
            new_address = self.socket.getsockname()
            if self.myself is None:
                self.myself = SocketPartner.SocketPartner(new_address, False)
            else:
                self.myself.setPartner(new_address)

        print("Socket Initialization: bound socket to local ", self.myself.getPartnerString(), "!", sep="")

    def performSend(self, buffer: bytes, localBytesSent: int, errorCode: int, header: Optional[SerializationHeader],
                    sendSize: int, sentBytes: int, sendIteration: int, verbose: bool = False) -> Tuple[bool, int, int]:
        withHeader = (header is not None or self.protocol == SocketType.UDP_HEADER)
        if withHeader:
            if header is not None:
                self.sendBuffer.setChar(header.getSerializationIteration(), 0)
            else:
                self.sendBuffer.setChar(0, 0)
            self.sendBuffer.setChar(sendIteration, 1)
            self.sendBuffer.setShort(sendSize, 2)
            self.sendBuffer.setData(buffer, sendSize, 4, sentBytes)
            localBytesOffset = 0
        else:
            self.sendBuffer.setReferenceToData(buffer, sendSize)
            localBytesOffset = sentBytes

        if verbose:
            print("Pre send: already sentBytes =", sentBytes, "; sending", sendSize, "B; buffer =", buffer,
                  "sentBytes =", sentBytes, "dataToBeSent: \"", buffer[sentBytes:sentBytes + sendSize], "\" to",
                  self.partner.getPartner())

        if self.protocol in [SocketType.UDP, SocketType.UDP_HEADER]:
            toAddress = self.partner.getPartner()
            assert (toAddress is not None)
            if toAddress is None:
                print("Send partner is null!")
                errorCode = 0
                return False, localBytesSent, errorCode
            localBytesSent = self.socket.sendto(
                self.sendBuffer.getBuffer()[localBytesOffset:localBytesOffset + self.sendBuffer.getBufferContentSize()],
                toAddress)
        elif self.protocol == SocketType.TCP:
            localBytesSent = self.socket.send(
                self.sendBuffer.getBuffer()[localBytesOffset:localBytesOffset + self.sendBuffer.getBufferContentSize()])
        else:
            raise RuntimeError("Unknown protocol: " + str(self.protocol))

        if withHeader and localBytesSent >= 4:
            localBytesSent -= 4

        if verbose:
            print("Post send: managed to send localBytesSent =", localBytesSent)
            """
            for i in range(localBytesSent):
                print(int(buffer[i + sentBytes]), ", ", sep="")
            print()
            # """

        return True, localBytesSent, errorCode

    def interpretSendResult(self, buffer: bytes, errorCode: int, header: Optional[SerializationHeader],
                            localBytesSent: int, retries: int, sendIteration: int, sendSize: int, sentBytes: int,
                            verbose: bool = False) -> Tuple[bool, int, int, int, int]:
        try:
            success, localBytesSent, errorCode = \
                self.performSend(buffer, localBytesSent, errorCode, header, sendSize, sentBytes, sendIteration, verbose)
            if not success:
                return False, errorCode, localBytesSent, retries, sendIteration
        except socket.error as se:
            errorCode = se.errno
            if errorCode in connection_errors:
                # Socket closed
                errorCode = 0
                return False, errorCode, localBytesSent, retries, sendIteration
            elif errorCode in timeout_errors:
                # send timeout
                errorCode = -1
                localBytesSent = 0
            else:
                if errorCode != 0:
                    print("SendBytes:", errorCode)
                return False, errorCode, localBytesSent, retries, sendIteration

        if localBytesSent == 0:
            if verbose:
                print("Sent 0 bytes???")
            if retries == 0:
                return False, errorCode, localBytesSent, retries, sendIteration
            if retries > 0:
                retries -= 1
        else:
            sendIteration += 1

        return True, errorCode, localBytesSent, retries, sendIteration

    def _sendBytes(self, buffer: bytes, bufferLength: int, errorCode: int, header: Optional[SerializationHeader] = None,
                   retries: int = 0, verbose: bool = False) -> Tuple[bool, int]:
        if not self.isInitialized():
            print("Can not send with an uninitialized socket!")
            errorCode = 0
            return False, errorCode
        # signal(SIGPIPE, signalHandler)
        if verbose:
            print("Entering _sendBytes function with buffer length =", bufferLength, "and sendPartner",
                  "any" if (self.partner is None) else self.partner.getStringAddress())

        sentBytes = 0
        localBytesSent = 0
        sendIteration = 0
        maxSendPerRound = CLIENT_MAX_MESSAGE_BYTES
        while sentBytes < bufferLength:
            sendSize = min(maxSendPerRound, bufferLength - sentBytes)
            success, errorCode, localBytesSent, retries, sendIteration = \
                self.interpretSendResult(buffer, errorCode, header, localBytesSent, retries, sendIteration, sendSize,
                                         sentBytes, verbose)
            if not success:
                return False, errorCode
            sentBytes += localBytesSent

        if verbose:
            # noinspection PyUnresolvedReferences
            print("Exiting _sendBytes function having sent = ", bufferLength, "B to ",
                  "any" if (self.partner is None) else self.partner.getStringAddress(), "!", sep="")

        return True, errorCode

    def checkCorrectReceivePartner(self, overwritePartner: bool, recvFirstMessage: bool):
        if self.protocol == SocketType.TCP:
            return True, overwritePartner

        assert (overwritePartner or self.partner is not None)
        if overwritePartner:
            assert (not recvFirstMessage)
            # if overwritePartner is desired, choose first partner and do not overwrite until recv is finished!
            if self.partner is None:
                self.partner = SocketPartner.SocketPartner(self.recvAddress, False)
            else:
                self.partner.setPartner(self.recvAddress)
            overwritePartner = False
        elif not comparePartners(self.recvAddress, self.partner.getPartner()):
            # do not consider received data from other partners than the saved partner
            return False, overwritePartner
        return True, overwritePartner

    def performReceive(self, buffer: bytes, expectedHeader: Optional[SerializationHeader], overwritePartner: bool,
                       receiveSize: int, receivedBytes: int, recvFirstMessage: bool,
                       verbose: bool = False) -> Tuple[bool, bytes, int, bool, bool]:

        recvFromCorrectPartner = True
        withHeader = (expectedHeader is not None or self.protocol == SocketType.UDP_HEADER)
        dataStart = 4 if withHeader else 0
        if withHeader and not recvFirstMessage and not self.recvBuffer.empty():
            localReceivedBytes = self.recvBuffer.getBufferContentSize()
        elif self.protocol in [SocketType.UDP, SocketType.UDP_HEADER]:
            assert (self.recvBuffer.empty())
            assert (self.recvBuffer.getBuffer() is not None)
            self.recvBuffer.buffer, self.recvAddress = self.socket.recvfrom(dataStart + receiveSize)
            localReceivedBytes = len(self.recvBuffer.buffer)
        elif self.protocol == SocketType.TCP:
            self.recvBuffer.buffer = self.socket.recv(dataStart + receiveSize)
            localReceivedBytes = len(self.recvBuffer.buffer)
        else:
            raise RuntimeError("Unknown protocol: " + str(self.protocol))

        if withHeader and localReceivedBytes >= 0:
            if localReceivedBytes < 4:
                print("Wrong protocol for this socket!!!")
                localReceivedBytes = -1
                return False, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner

            self.recvBuffer.setBufferContentSize(localReceivedBytes)
            localReceivedBytes -= 4
            if verbose:
                print("Should have received", self.recvBuffer.getShort(2), ", received" + localReceivedBytes)
        if verbose:
            print("Post receive... localReceivedBytes =", localReceivedBytes, "; overwritePartner =", overwritePartner)

        if localReceivedBytes < 0:
            return True, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner

        recvFromCorrectPartner, overwritePartner = self.checkCorrectReceivePartner(overwritePartner, recvFirstMessage)
        if not recvFromCorrectPartner:
            self.recvBuffer.setBufferContentSize(0)
            return True, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner

        if withHeader:
            # check serialization iteration
            if expectedHeader is not None and self.recvBuffer.getChar(0) != expectedHeader.getSerializationIteration():
                # received different serialization state...check if the partner is the same
                # interrupt receive! and don't reset the recvBuffer to keep data for next recv!
                localReceivedBytes = -2
                return True, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner

            # check send iteration
            sendIteration = self.recvBuffer.getChar(1)
            if recvFirstMessage and sendIteration == 0:
                # Received start of another message... interrupt receive!
                # And don't reset the recvBuffer to keep data for next recv!
                localReceivedBytes = -2
                return True, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner
            if not recvFirstMessage and sendIteration > 0:
                # wrong data, ignore, pretend like nothing was received (like recv from wrong partner)
                recvFromCorrectPartner = False
        if recvFromCorrectPartner:
            if verbose:
                print("Received data: ", end="")
                receivedBufferData = self.recvBuffer.getBuffer()[dataStart:]
                for x in receivedBufferData[:min(localReceivedBytes, 50)]:
                    print(int(x), end=" ")
                print()
            memcpy(buffer, receivedBytes, self.recvBuffer.getBuffer(), dataStart, localReceivedBytes)
        self.recvBuffer.setBufferContentSize(0)

        if verbose:
            print("Post receive... managed to receive localReceivedBytes =", localReceivedBytes)
            """
            for x in buffer[receivedBytes:receivedBytes + localReceivedBytes]:
                print(int(x), ", ", sep="", end="")
            print()
            """
        return True, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner

    def interpretReceiveResult(self, buffer: bytes, errorCode: int, expectedHeader: Optional[SerializationHeader],
                               localReceivedBytes: int, receiveSize: int, receivedBytes: int, overwritePartner: bool,
                               recvFirstMessage: bool, recvFromCorrectPartner: bool,
                               verbose: bool = False) -> Tuple[bool, bytes, int, int, bool, bool, bool]:
        try:
            success, buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner = \
                self.performReceive(buffer, expectedHeader, overwritePartner, receiveSize, receivedBytes,
                                    recvFirstMessage, verbose)
            if not success:
                return (False, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                        recvFromCorrectPartner)
        except socket.error as se:
            errorCode = se.errno
            if errorCode in connection_errors:
                # Socket closed
                errorCode = 0
                return (False, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                        recvFromCorrectPartner)
            elif errorCode in timeout_errors:
                # recv timeout
                errorCode = 0
                localReceivedBytes = -1
            elif errorCode in memory_size_too_large:
                # UDP_HEADER might get a too large message from another packet size
                errorCode = 0
                localReceivedBytes = -1
            elif errorCode != 0:
                print("ReceiveBytes:", errorCode)
                return (False, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                        recvFromCorrectPartner)

        if localReceivedBytes > 0:
            if recvFromCorrectPartner:
                if verbose and not recvFirstMessage:
                    print("Received first data!")
                recvFirstMessage = True
            else:
                # simulate receiving nothing!
                localReceivedBytes = -1
                errorCode = 0
        elif localReceivedBytes == 0:
            # socket closed
            return (False, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                    recvFromCorrectPartner)
        else:
            if localReceivedBytes != -1 and localReceivedBytes != -2:
                print("The next assertion will fail: localReceivedBytes =", localReceivedBytes)

            assert (localReceivedBytes == -1 or localReceivedBytes == -2)
            if localReceivedBytes == -2:
                errorCode = -2
                return (False, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                        recvFromCorrectPartner)

            # only set the errorCode here, because when there is an error, localReceivedBytes is set to - 1!
            assert (localReceivedBytes == -1)
            if verbose:
                print("ReceiveBytes:", errorCode)

        return True, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage, recvFromCorrectPartner

    @staticmethod
    def setRetries(retries: int, maxRetries: int, recvFromCorrectPartner: bool, localReceivedBytes: int,
                   verbose: bool = False) -> Tuple[int, bool]:
        # retry receiving if received data from someone else or (if received nothing and there are retries left)
        retry = (not recvFromCorrectPartner) or ((localReceivedBytes == -1) and (retries != 0))
        if verbose:
            print("Retry =", retry, "retries =", retries)
        if retry and retries > 0:
            if recvFromCorrectPartner:
                retries -= 1
            else:
                retries = maxRetries
        return retries, retry

    def _receiveBytes(self, buffer: bytes, expectedLength: int, errorCode: int,
                      expectedHeader: Optional[SerializationHeader] = None, retries: int = 0,
                      verbose: bool = False) -> Tuple[bool, bytes, int]:
        if not self.isInitialized():
            print("Can not receive with an uninitialized socket!")
            return False, buffer, errorCode

        # signal(SIGPIPE, signalHandler)
        if verbose:
            print("Entering _receiveBytes function with expected length =", expectedLength, "and receivePartner =",
                  "any" if (self.partner is None) else self.partner.getStringAddress())

        maxRetries = retries
        receivedBytes = 0
        maxPossibleReceiveBytes = CLIENT_MAX_MESSAGE_BYTES
        errorCode = 0
        recvFirstMessage = False
        recvFromCorrectPartner = True
        overwritePartner = self.partner is None or (not self.partner.isInitialized() or self.partner.getOverwrite())
        while receivedBytes < expectedLength:
            receiveSize = min(expectedLength - receivedBytes, maxPossibleReceiveBytes)
            # wait to receive data from socket (and retry when timeout occurs) do
            retry = True
            firstInnerIteration = True
            localReceivedBytes = 0
            # <- continue retry receiving until the first message has been received
            while firstInnerIteration or (not recvFirstMessage and retry):
                firstInnerIteration = False

                (success, buffer, errorCode, localReceivedBytes, overwritePartner, recvFirstMessage,
                 recvFromCorrectPartner) = \
                    self.interpretReceiveResult(buffer, errorCode, expectedHeader, localReceivedBytes, receiveSize,
                                                receivedBytes, overwritePartner, recvFirstMessage,
                                                recvFromCorrectPartner, verbose)
                if not success:
                    return False, buffer, errorCode

                retries, retry = self.setRetries(retries, maxRetries, recvFromCorrectPartner, localReceivedBytes,
                                                 verbose)

                if verbose and localReceivedBytes >= 0:
                    print("Total received bytes so far:", receivedBytes + localReceivedBytes)

            assert (localReceivedBytes == -1 or localReceivedBytes > 0)
            if localReceivedBytes == -1:
                # set timeout error code
                errorCode = -1
                return False, buffer, errorCode

            receivedBytes += localReceivedBytes

        assert (receivedBytes == expectedLength), "{} vs. {}".format(receivedBytes, expectedLength)
        if verbose:
            # noinspection PyUnresolvedReferences
            print("Exiting _receiveBytes function having received = ", expectedLength, "B from ",
                  "any" if (self.partner is None) else self.partner.getStringAddress(), "!", sep="")

        return True, buffer, errorCode
