from comm.comm_socket.Buffer import Buffer
from comm.comm_socket.Socket import Socket
from comm.comm_socket.SocketPartner import SocketPartner
from comm.comm_socket.SocketType import SocketType
from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.MessageType import MessageType
from typing import Dict, Optional, Tuple


class Communication:
    def __init__(self):
        self.sockets = {}  # type: Dict[SocketType, Optional[Socket]]
        self.sendBuffer = Buffer()  # type: Buffer
        self.recvBuffer = Buffer()  # type: Buffer
        self.isCopy = False  # type: bool
        self.errorCode = 0  # type: int

    def cleanup(self):
        for socketType, socket in self.sockets.items():
            if socket is not None:
                socket.cleanup()
                self.sockets[socketType] = None
        self.sockets.clear()
        self.isCopy = False

    def copy(self) -> 'Communication':
        comm = Communication()
        self.copyData(comm)
        return comm

    def createSocket(self, socketType: SocketType, partner: SocketPartner = None, myPort: int = 0,
                     sendTimeout: int = -1, recvTimeout: int = -1):
        socket = self.getSocket(socketType)
        if socket:
            socket.cleanup()
        self.setSocket(socketType, Socket.Socket(socketType, partner, myPort, sendTimeout, recvTimeout))

    def setAllSocketTimeouts(self, sendTimeout: int = -1, recvTimeout: int = -1):
        for socketType in self.sockets:
            self.setSocketTimeouts(socketType, sendTimeout, recvTimeout)

    def setSocketTimeouts(self, socketType: SocketType, sendTimeout: int = -1, recvTimeout: int = -1):
        socket = self.getSocket(socketType)  # type: Socket
        if not socket:
            return
        socket.setSocketTimeouts(sendTimeout, recvTimeout)

    def sendMessageType(self, socketType: SocketType, messageType: Optional[MessageType], retries: int = 0,
                        verbose: bool = False) -> bool:
        if not messageType:
            return False
        self.errorCode = 0
        self.sendBuffer.setBufferContentSize(1)
        self.sendBuffer.setChar(messageType.value[0], 0)
        if verbose:
            print("Before sending message type...")
        if not self.send(socketType, retries, verbose):
            if self.errorCode == 0:
                if verbose:
                    print("Socket closed: Can not send messageType...", MessageType.messageTypeToString(messageType))
            else:
                print("Can not send messageType...", MessageType.messageTypeToString(messageType))
                if verbose:
                    print("Can not send messageType...", MessageType.messageTypeToString(messageType))
            return False
        return True

    def sendData(self, socketType: SocketType, data: CommunicationData, withMessageType: bool, retries: int = 0,
                 verbose: bool = False) -> bool:
        if not data:
            return False
        self.errorCode = 0

        if withMessageType:
            if verbose:
                print("Before sending message type...")
            if verbose:
                print("Message type =", MessageType.messageTypeToString(data.getMessageType()))
            if not self.sendMessageType(socketType, data.getMessageType(), retries, verbose):
                if self.errorCode == 0:
                    if verbose:
                        print("Socket closed: Can not send data message type bytes...")
                else:
                    print("Can not send data message type bytes... error", self.errorCode)
                return False
            if verbose:
                print("Sent message type!!!")

        serializeDone = False
        while not serializeDone:
            serializeDone = data.serialize(self.sendBuffer, verbose)
            self.errorCode = 0
            if not self.send(socketType, retries, verbose):
                if self.errorCode == 0:
                    if verbose:
                        print("Socket closed: Can not send data serialized bytes...")
                else:
                    print("Can not send data serialized bytes... error", self.errorCode)
                    data.resetSerializeState()
                return False

        return True

    def sendRaw(self, socketType: SocketType, data: bytes, dataSize: int, retries: int = 0,
                verbose: bool = False) -> bool:
        if not data:
            return False
        self.sendBuffer.setData(data, dataSize)
        self.errorCode = 0
        if not self.send(socketType, retries, verbose):
            if self.errorCode == 0:
                if verbose:
                    print("Socket closed: Can not send data serialized bytes...")
            else:
                print("Can not send data serialized bytes... error", self.errorCode)
            return False
        return True

    def recvMessageType(self, socketType: SocketType, retries: int = 0, verbose: bool = False) \
            -> Tuple[bool, MessageType]:
        self.errorCode = 0
        self.recvBuffer.setBufferContentSize(1)

        result = self.recv(socketType, retries, verbose)
        assert isinstance(result, bool)
        if not result:
            # why < 1 (why is also 0, when the socket closes, ok???)
            if self.errorCode < 1:
                return True, MessageType.NOTHING
            return False, MessageType.NOTHING
        return True, MessageType.intToMessageType(self.recvBuffer.getChar(0))

    def recvData(self, socketType: SocketType, data: CommunicationData, retries: int = 0, verbose: bool = False) \
            -> Tuple[bool, CommunicationData]:
        self.errorCode = 0
        localRetriesThreshold = 0  # 10
        localRetries = localRetriesThreshold
        deserializeState = 0

        deserializeDone = False
        receivedSomething = False

        while not deserializeDone and localRetries >= 0:
            if verbose:
                print("Communication::recvData: LocalRetries =", localRetries, "data->getMessageType() ",
                      MessageType.messageTypeToString(data.getMessageType()), " deserializeState =", deserializeState)
            self.errorCode = 0

            dataLocalDeserializeBuffer = data.getDeserializeBuffer()
            expectedSize = data.getExpectedDataSize()
            if verbose:
                print("In Communication::recvData: dataLocalDeserializeBuffer =", dataLocalDeserializeBuffer,
                      "expectedSize =", expectedSize, " deserializeState =", deserializeState)
            if dataLocalDeserializeBuffer is not None:
                receiveResult = self.recv((socketType, dataLocalDeserializeBuffer, expectedSize, expectedSize), retries,
                                          verbose)
                assert isinstance(receiveResult, tuple)
                recvSuccess = receiveResult[0]
                buffer = Buffer()
                buffer.setData(receiveResult[1], receiveResult[2])
                if verbose:
                    print("ReceiveResult = ", receiveResult)
            else:
                self.recvBuffer.setBufferContentSize(expectedSize)
                """
                print("Expecting " + str(self.recvBuffer.getBufferContentSize()) + "bytes... (expectedSize = " +
                      str(expectedSize) + ")")
                # """
                recvSuccess = self.recv(socketType, retries, verbose)
                assert isinstance(recvSuccess, bool)
                buffer = self.recvBuffer

            if not recvSuccess:
                if self.errorCode >= 0:
                    print("Stop loop: Can not recv data serialized bytes... error", self.errorCode,
                          "deserializeState =", deserializeState)
                    data.resetDeserializeState()
                    return False, data
                elif self.errorCode == -2:
                    print("Stop loop: Only part of the data has been received before new message started... "
                          "deserializeState =", deserializeState)
                    data.resetDeserializeState()
                    return False, data

            assert (recvSuccess or self.errorCode == -1)
            # if we received something...
            if self.errorCode != -1:
                if verbose:
                    print("Received something! data->getMessageType()",
                          MessageType.messageTypeToString(data.getMessageType()))
                receivedSomething = True
                deserializeDone = data.deserialize(buffer, 0, verbose)
                deserializeState += 1
                localRetries = localRetriesThreshold
            else:
                if verbose:
                    print("Received nothing, decrease local retries!")
                localRetries -= 1

        if receivedSomething and not deserializeDone:
            print("After loop: Could not recv data serialized bytes... error", self.errorCode, "deserializeState =",
                  deserializeState)
            print("After loop: Could not recv data serialized bytes... error", self.errorCode, "deserializeState =",
                  deserializeState)
            data.resetDeserializeState()
            return False, data
        elif not receivedSomething:
            assert self.errorCode == -1
            if verbose:
                print("Did not receive anything although expected ",
                      MessageType.messageTypeToString(data.getMessageType()))
            return False, data

        return True, data

    def setSocket(self, socketType: SocketType, socket: Socket):
        oldSocket = self.sockets.get(socketType, None)
        if oldSocket:
            oldSocket.cleanup()
        self.sockets[socketType] = socket

    def setPartner(self, socketType: SocketType, partner: Tuple[str, int], overwrite: bool):
        socket = self.getSocket(socketType)  # type: Optional[Socket]
        if not socket:
            return
        socket.setPartner(SocketPartner.SocketPartner(partner, overwrite), overwrite)

    def setOverwritePartner(self, socketType: SocketType, overwrite: bool):
        socket = self.getSocket(socketType)  # type: Optional[Socket]
        if not socket:
            return
        socket.setOverwritePartner(overwrite)

    def getSocket(self, socketType: SocketType) -> Optional[Socket]:
        return self.sockets.get(socketType, None)

    def getMyself(self, socketType: SocketType) -> Optional[SocketPartner]:
        socket = self.getSocket(socketType)
        if not socket or not socket.isInitialized():
            print(SocketType.socketTypeToString(socketType), "socket is not initialized! Can't getMyself...")
            return None
        return socket.getMyself()

    def getMyAddressString(self, socketType: SocketType) -> str:
        myself = self.getMyself(socketType)  # type: Optional[SocketPartner]
        assert myself
        return myself.getPartnerString()

    def getPartner(self, socketType: SocketType) -> Optional[SocketPartner]:
        socket = self.getSocket(socketType)
        if not socket or not socket.isInitialized():
            print(SocketType.socketTypeToString(socketType), "socket is not initialized! Can't getPartner...")
            raise RuntimeError(SocketType.socketTypeToString(socketType) +
                               "socket is not initialized! Can't getPartner...")
        return socket.getPartner()

    def getPartnerString(self, socketType: SocketType) -> str:
        partner = self.getPartner(socketType)  # type: Optional[SocketPartner]
        if not partner:
            return "???"
        return partner.getPartnerString()

    def getErrorCode(self) -> int:
        return self.errorCode

    def getErrorString(self) -> str:
        return "Error: " + str(self.errorCode)

    def copyData(self, copy: 'Communication'):
        copy.isCopy = True
        for socketType, socket in self.sockets.items():
            if socket:
                copy.sockets[socketType] = socket.copy()

    def send(self, sendData: SocketType or Tuple[SocketType, bytes, int], retries: int = 0,
             verbose: bool = False) -> bool:
        if isinstance(sendData, SocketType):
            socketType = sendData
            buffer = self.sendBuffer
        else:
            socketType, buffer, contentSize = sendData
            buffer = (buffer, contentSize)

        socket = self.getSocket(socketType)
        if not socket:
            return False
        success, self.errorCode = socket.sendBytes(buffer, self.errorCode, retries, verbose)
        return success

    def recv(self, recvData: SocketType or Tuple[SocketType, bytes, int, int], retries: int = 0,
             verbose: bool = False) -> bool or Tuple[bool, bytes, int]:
        if isinstance(recvData, SocketType):
            socket = self.getSocket(recvData)
            if not socket:
                return False
            buffer = self.recvBuffer
        else:
            socket = self.getSocket(recvData[0])
            if not socket:
                return False, recvData[1], recvData[2]
            buffer = (recvData[1], recvData[2], recvData[3])

        success, buffer, self.errorCode = socket.receiveBytes(buffer, self.errorCode, retries, verbose)
        if isinstance(buffer, tuple):
            return success, buffer[0], buffer[1]
        else:
            self.recvBuffer = buffer
            return success
