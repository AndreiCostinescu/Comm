from comm.comm_utils.Buffer import Buffer
from comm.comm_utils.NetworkData import NetworkData
from comm.comm_utils.utils import memcpy, memset, networkToHostBytes, hostToNetworkBytes
from typing import Optional


class SerializationHeader:
    def __init__(self, **kwargs):
        buffer = kwargs.get("buffer", None)
        header = kwargs.get("header", None)
        serializationIteration = kwargs.get("serializationIteration", None)
        sendIteration = kwargs.get("sendIteration", None)
        sendSize = kwargs.get("sendSize", None)

        self.localBuffer = None  # type: Optional[bytes]
        self.created = False  # type: bool
        self.passedBuffer = None  # type: Optional[Buffer]

        if buffer is not None:
            self.created = False
            if isinstance(buffer, bytearray):
                self.localBuffer = buffer
            elif isinstance(buffer, Buffer):
                self.passedBuffer = buffer
            else:
                raise RuntimeError("Can not process buffer type: " + type(buffer))
        else:
            self.created = True
            self.localBuffer = bytearray(4)

        if header is not None:
            self.setInt(header)
        elif serializationIteration is not None and sendIteration is not None and sendSize is not None:
            self.setData(serializationIteration, sendIteration, sendSize)

    def reset(self):
        self.setInt(0)

    def setInt(self, header: int, local: bool = True):
        self.setSendSize(header & 65535, local)
        header = header >> 16
        self.setSendIteration(header & 255)
        self.setSerializationIteration(header >> 8)

    def setData(self, _serializationIteration: int, _sendIteration: int, _sendSize: int, local: bool = True):
        self.setSerializationIteration(_serializationIteration)
        self.setSendIteration(_sendIteration)
        self.setSendSize(_sendSize, local)

    def setSerializationIteration(self, _serializationIteration: int):
        if self.passedBuffer is None:
            self.localBuffer[0] = _serializationIteration
        else:
            self.passedBuffer.setChar(_serializationIteration, 0)

    def setSendIteration(self, _sendIteration: int):
        if self.passedBuffer is None:
            self.localBuffer[1] = _sendIteration
        else:
            self.passedBuffer.setChar(_sendIteration, 1)

    def setSendSize(self, _sendSize: int, setLocal: bool = True):
        if self.passedBuffer is None:
            sendSize = _sendSize.to_bytes(2, "big") if setLocal else networkToHostBytes(_sendSize.to_bytes(2, "big"))
            self.localBuffer = memcpy(self.localBuffer, 2, sendSize, 0, 2)
        else:
            sendSize = _sendSize if setLocal else int.from_bytes(networkToHostBytes(_sendSize.to_bytes(2, "big")),
                                                                 "big")
            self.passedBuffer.setShort(sendSize, 2)

    def setBuffer(self, buffer: bytes, setLocal: bool, start: int = 0):
        if isinstance(buffer, Buffer):
            buffer.setInt(self.getInt(setLocal), start)
            return buffer
        buffer = memset(buffer, start, self.getSerializationIteration(), 1)
        buffer = memset(buffer, start + 1, self.getSendIteration(), 1)
        x = self.getSendSize(setLocal)
        buffer = memcpy(buffer, start + 2, x.to_bytes(2, "big"), 0, NetworkData.shortSize)
        return buffer

    def getSerializationIteration(self) -> int:
        if self.passedBuffer is None:
            return int(self.localBuffer[0])
        else:
            return self.passedBuffer.getChar(0)

    def getSendIteration(self) -> int:
        if self.passedBuffer is None:
            return int(self.localBuffer[1])
        else:
            return self.passedBuffer.getChar(1)

    def getSendSize(self, getLocal: bool = True) -> int:
        if self.passedBuffer is None:
            sendSize = int.from_bytes(self.localBuffer[2:4], "big")
        else:
            sendSize = self.passedBuffer.getShort(2)
        if getLocal:
            return sendSize
        else:
            return int.from_bytes(hostToNetworkBytes(sendSize.to_bytes(2, "big")), "big")

    def getInt(self, getLocal: bool = True):
        return (self.getSerializationIteration() << 24) + (self.getSendIteration() << 16) + self.getSendSize(getLocal)
