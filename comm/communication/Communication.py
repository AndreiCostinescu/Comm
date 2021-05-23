from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.DataCollection import DataCollection
from comm.comm_data.MessageType import MessageType
from comm.comm_socket.Socket import Socket
from comm.comm_socket.SocketPartner import SocketPartner
from comm.comm_socket.SocketType import SocketType
from comm.comm_utils.Buffer import Buffer
from comm.comm_utils.SerializationHeader import SerializationHeader
from typing import Dict, Optional, Tuple


class Communication:
    def __init__(self):
        self.sockets = {}  # type: Dict[SocketType, Optional[Socket]]
        self.sendBuffer = Buffer()  # type: Buffer
        self.recvBuffer = Buffer()  # type: Buffer
        self.isCopy = False  # type: bool
        self.errorCode = 0  # type: int
        self.sendHeader = SerializationHeader(created=True)
        self.recvHeader = SerializationHeader(created=True)

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

    def transmitData(self, socketType: SocketType, data: CommunicationData, withHeader: bool,
                     withMessageType: bool = True, retries: int = 0, verbose: bool = False) -> bool:
        if data is None:
            return False
        self.errorCode = 0

        serializeDone = False
        dataStart = 4 if withHeader else 0
        serializationState = 0
        while not serializeDone:
            if withHeader:
                self.sendHeader.setData(serializationState, 0, 0)
            if serializationState == 0 and withMessageType:
                self.sendBuffer.setBufferContentSize(dataStart + 1)
                self.sendBuffer.setChar(data.getMessageType().value[0], dataStart)
            else:
                serializeDone = data.serialize(self.sendBuffer, dataStart, withHeader, verbose)

            self.errorCode = 0
            if not self.send(socketType, withHeader, retries, verbose):
                if self.errorCode == 0:
                    if verbose:
                        print("Socket closed: Can not send data serialized bytes...")
                else:
                    print("Can not send data serialized bytes... error", self.errorCode)
                data.resetSerializeState()
                return False
            serializationState += 1
        return True

    def sendRaw(self, socketType: SocketType, data: bytes, dataSize: int, retries: int = 0,
                verbose: bool = False) -> bool:
        if not data:
            return False
        self.sendBuffer.setData(data, dataSize)
        self.errorCode = 0
        if not self.send(socketType, None, retries, verbose):
            if self.errorCode == 0:
                if verbose:
                    print("Socket closed: Can not send data serialized bytes...")
            else:
                print("Can not send data serialized bytes... error", self.errorCode)
            return False
        return True

    def recvMessageType(self, socketType: SocketType, withHeader: bool, retries: int = 0,
                        verbose: bool = False) -> Tuple[bool, MessageType]:
        dataStart = 4 if withHeader else 0
        self.errorCode = 0
        if withHeader:
            self.recvHeader.setData(0, 0, 0)
        dataLocalDeserializeBuffer, expectedSize = self.preReceiveMessageType(dataStart)
        assert (dataLocalDeserializeBuffer is None)
        receiveResult = self.doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries,
                                       verbose)
        return self.postReceiveMessageType(receiveResult, dataStart)

    def recvData(self, socketType: SocketType, data: CommunicationData, withHeader: bool, gotMessageType: bool = True,
                 retries: int = 0, verbose: bool = False):
        deserializationDone = False
        receivedSomething = False
        deserializeState = int(gotMessageType)
        localRetriesThreshold = 0
        localRetries = localRetriesThreshold
        dataStart = 4 if withHeader else 0
        messageType = data.getMessageType()
        while not deserializationDone and localRetries >= 0:
            self.errorCode = 0
            if withHeader:
                self.recvHeader.setData(deserializeState, 0, 0)
            dataLocalDeserializeBuffer, expectedSize = self.preReceiveData(dataStart, data, withHeader)
            if dataLocalDeserializeBuffer is not None:
                receiveResult, dataLocalDeserializeBuffer, expectedSize = \
                    self.doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries, verbose)
            else:
                receiveResult = self.doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader,
                                               retries, verbose)

            postReceiveResult, data, deserializeState, localRetries, receivedSomething, deserializationDone = \
                self.postReceiveData(data, deserializeState, localRetries, receivedSomething, deserializationDone,
                                     messageType, dataStart, localRetriesThreshold, receiveResult, withHeader,
                                     verbose)
            if not postReceiveResult:
                return False

        if receivedSomething and not deserializationDone:
            print("After loop: Could not recv", MessageType.messageTypeToString(messageType),
                  "serialized bytes... error", self.errorCode, "; deserializeState =", deserializeState)
            print("After loop: Could not recv", MessageType.messageTypeToString(messageType),
                  "serialized bytes... error", self.errorCode, "; deserializeState =", deserializeState)
            if data is not None:
                data.resetDeserializeState()
            return False
        elif not receivedSomething:
            assert self.errorCode == -1
            if verbose:
                print("Did not receive anything although expected", MessageType.messageTypeToString(messageType))
            return False

        return True

    def receiveData(self, socketType: SocketType, data: DataCollection, withHeader: bool, retries: int = 0,
                    verbose: bool = False):
        deserializationDone = False
        receivedSomething = False
        deserializeState = 0
        dataStart = 0
        localRetriesThreshold = 0  # 10
        localRetries = localRetriesThreshold
        messageType = None
        recvData = None
        if withHeader:
            dataStart = 4

        while not deserializationDone and localRetries >= 0:
            self.errorCode = 0

            # receive setup
            if deserializeState == 0:
                dataLocalDeserializeBuffer, expectedSize = self.preReceiveMessageType(dataStart)
            else:
                dataLocalDeserializeBuffer, expectedSize = self.preReceiveData(dataStart, recvData, withHeader)
                if verbose:
                    print("In Communication::fullReceiveData: dataLocalDeserializeBuffer =", dataLocalDeserializeBuffer,
                          "; expectedSize =", expectedSize, "; deserializeState =", deserializeState)

            # do receive
            if dataLocalDeserializeBuffer is not None:
                receiveResult, dataLocalDeserializeBuffer, expectedSize = \
                    self.doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries, verbose)
            else:
                receiveResult = self.doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader,
                                               retries, verbose)

            # post receive
            if deserializeState == 0:
                postReceiveResult, messageType = self.postReceiveMessageType(receiveResult, dataStart)
                if not postReceiveResult:
                    return False

                if messageType == MessageType.NOTHING:
                    deserializationDone = True
                    break
                else:
                    recvData = data.get(messageType)
            else:
                assert messageType is not None
                postReceiveResult, recvData, deserializeState, localRetries, receivedSomething, deserializationDone = \
                    self.postReceiveData(recvData, deserializeState, localRetries, receivedSomething,
                                         deserializationDone, messageType, dataStart, localRetriesThreshold,
                                         receiveResult, withHeader, verbose)
                if not postReceiveResult:
                    return False

            deserializeState += 1

        if receivedSomething and not deserializationDone:
            print("After loop: Could not recv", MessageType.messageTypeToString(messageType), "serialized bytes...",
                  "error", self.errorCode, "; deserializeState =", deserializeState)
            print("After loop: Could not recv", MessageType.messageTypeToString(messageType), "serialized bytes...",
                  "error", self.errorCode, "; deserializeState =", deserializeState)
            if recvData is not None:
                recvData.resetDeserializeState()
            return False
        elif not receivedSomething:
            assert self.errorCode == -1
            if verbose:
                print("Did not receive anything although expected", MessageType.messageTypeToString(messageType))
            return False
        return True

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

    def send(self, sendData: SocketType or Tuple[SocketType, bytes, int], header: bool or Optional[SerializationHeader],
             retries: int = 0, verbose: bool = False) -> bool:
        if isinstance(sendData, SocketType):
            socketType = sendData
            buffer = self.sendBuffer
        else:
            socketType, buffer, contentSize = sendData
            buffer = (buffer, contentSize)

        socket = self.getSocket(socketType)
        if not socket:
            return False
        if isinstance(header, bool):
            header = self.sendHeader if header else None
        success, self.errorCode = socket.sendBytes(buffer, self.errorCode, header, retries, verbose)
        return success

    def preReceiveMessageType(self, dataStart: int) -> Tuple[Optional[bytes], int]:
        return None, dataStart + 1

    def preReceiveData(self, dataStart: int, recvData: CommunicationData,
                       withHeader: bool) -> Tuple[Optional[bytes], int]:
        dataLocalDeserializeBuffer = recvData.getDeserializeBuffer()
        # don't to the if before, because some CommunicationData depend on calling getDeserializeBuffer!
        if withHeader:
            dataLocalDeserializeBuffer = None
        expectedSize = dataStart + recvData.getExpectedDataSize()
        return dataLocalDeserializeBuffer, expectedSize

    def doReceive(self, socketType: SocketType, dataLocalDeserializeBuffer: Optional[bytes], expectedSize: int,
                  withHeader: bool, retries: int, verbose: bool) -> bool or Tuple[bool, bytes, int]:
        if dataLocalDeserializeBuffer is not None:
            assert (not withHeader)
            return self.recv((socketType, dataLocalDeserializeBuffer, expectedSize, expectedSize), None, retries,
                             verbose)
        else:
            self.recvBuffer.setBufferContentSize(expectedSize)
            return self.recv(socketType, withHeader, retries, verbose)

    def postReceiveMessageType(self, receiveResult: bool, dataStart: int) -> Tuple[bool, MessageType]:
        if not receiveResult:
            if self.getErrorCode() < 1:
                messageType = MessageType.NOTHING
            else:
                return False, MessageType.NOTHING
        else:
            messageType = MessageType.intToMessageType(self.recvBuffer.getChar(dataStart))
        return True, messageType

    def postReceiveData(self, recvData: CommunicationData, deserializeState: int, localRetries: int,
                        receivedSomething: bool, deserializationDone: bool, messageType: MessageType, dataStart: int,
                        localRetriesThreshold: int, receiveResult: bool, withHeader: bool,
                        verbose: bool) -> Tuple[bool, CommunicationData, int, int, bool, bool]:
        if not receiveResult:
            if self.getErrorCode() >= 0:
                print("Stop loop: Can not recv data serialized bytes... error", self.getErrorCode(),
                      "; deserializeState =", deserializeState)
                recvData.resetDeserializeState()
                return False, recvData, deserializeState, localRetries, receivedSomething, deserializationDone
            elif self.getErrorCode() == -2:
                print("Stop loop: Only part of the data has been received before new message started...",
                      "deserializeState =", deserializeState)
                recvData.resetDeserializeState()
                return False, recvData, deserializeState, localRetries, receivedSomething, deserializationDone

        assert (receiveResult or self.getErrorCode() == -1)
        # if we received something...
        if self.getErrorCode() != -1:
            if verbose:
                print("Received something! data->getMessageType()", MessageType.messageTypeToString(messageType))
            receivedSomething = True
            deserializationDone = recvData.deserialize(self.recvBuffer, dataStart, withHeader, verbose)
            deserializeState += 1
            localRetries = localRetriesThreshold
        else:
            if verbose:
                print("Received nothing, decrease local retries!")
            localRetries -= 1
        return True, recvData, deserializeState, localRetries, receivedSomething, deserializationDone

    def recv(self, recvData: SocketType or Tuple[SocketType, bytes, int, int],
             expectedHeader: bool or Optional[SerializationHeader], retries: int = 0,
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

        if isinstance(expectedHeader, bool):
            expectedHeader = self.recvHeader if expectedHeader else None
        success, buffer, self.errorCode = socket.receiveBytes(buffer, self.errorCode, expectedHeader, retries, verbose)
        if isinstance(buffer, tuple):
            return success, buffer[0], buffer[1]
        else:
            self.recvBuffer = buffer
            return success
