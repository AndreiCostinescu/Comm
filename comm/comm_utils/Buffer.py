from comm.comm_utils.utils import memcpy, memset
from comm.comm_utils.NetworkData import NetworkData
from typing import Optional


class Buffer:
    def __init__(self, bufferSize: int = 0):
        self.buffer = None  # type: Optional[bytes]
        self.bufferSize = 0
        self.bufferContentSize = 0
        self.referenceBuffer = None  # type: Optional[bytes]
        self.useReferenceBuffer = False
        self.prepareBuffer(bufferSize)

    def reset(self):
        self.buffer = None
        self.bufferSize = 0
        self.bufferContentSize = 0
        self.referenceBuffer = None
        self.useReferenceBuffer = False

    def copy(self):
        copy = Buffer(self.bufferSize)
        assert (copy.bufferSize == self.bufferSize)

        copy.buffer = memcpy(copy.buffer, 0, self.buffer, 0, self.bufferSize)

        copy.bufferContentSize = self.bufferContentSize

        copy.referenceBuffer = self.referenceBuffer
        copy.useReferenceBuffer = self.useReferenceBuffer

        return copy

    def setData(self, data: str or bytes, dataSize: int, start: int = 0, offset: int = 0):
        if isinstance(data, str):
            data = bytes(data, "ascii")
        self.setBufferContentSize(dataSize + start)
        self.buffer = memcpy(self.buffer, start, data, offset, dataSize)
        self.useReferenceBuffer = False

    def setReferenceToData(self, data: bytes, dataSize: int):
        self.referenceBuffer = data
        self.bufferContentSize = dataSize
        self.useReferenceBuffer = True

    def setChar(self, data: int, position: int):
        self.checkBufferContentSize(position + 1, True)
        self.buffer = NetworkData.charToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def setShort(self, data: int, position: int):
        self.checkBufferContentSize(position + 2, True)
        self.buffer = NetworkData.shortToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def setInt(self, data: int, position: int):
        self.checkBufferContentSize(position + 4, True)
        self.buffer = NetworkData.intToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def setLongLong(self, data: int, position: int):
        self.checkBufferContentSize(position + 8, True)
        self.buffer = NetworkData.longLongToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def setFloat(self, data: float, position: int):
        self.checkBufferContentSize(position + 4, True)
        self.buffer = NetworkData.floatToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def setDouble(self, data: float, position: int):
        self.checkBufferContentSize(position + 8, True)
        self.buffer = NetworkData.doubleToNetworkBytes(self.buffer, position, data)
        self.useReferenceBuffer = False

    def getChar(self, position: int):
        self.checkBufferContentSize(position + 1, False)
        return NetworkData.networkBytesToChar(self.buffer, position)

    def getShort(self, position: int):
        self.checkBufferContentSize(position + 2, False)
        short = NetworkData.networkBytesToShort(self.buffer, position)
        return short

    def getInt(self, position: int):
        self.checkBufferContentSize(position + 4, False)
        return NetworkData.networkBytesToInt(self.buffer, position)

    def getLongLong(self, position: int):
        self.checkBufferContentSize(position + 8, False)
        return NetworkData.networkBytesToLongLong(self.buffer, position)

    def getFloat(self, position: int):
        self.checkBufferContentSize(position + 4, False)
        return NetworkData.networkBytesToFloat(self.buffer, position)

    def getDouble(self, position: int):
        self.checkBufferContentSize(position + 8, False)
        return NetworkData.networkBytesToDouble(self.buffer, position)

    def empty(self):
        return self.bufferContentSize == 0

    def getBuffer(self):
        result = self.referenceBuffer if self.useReferenceBuffer else self.buffer
        self.useReferenceBuffer = False
        return result

    def getBufferSize(self):
        return self.bufferSize

    def getBufferContentSize(self):
        return self.bufferContentSize

    def setBufferContentSize(self, _bufferContentSize: int):
        self.prepareBuffer(_bufferContentSize)
        self.bufferContentSize = _bufferContentSize

    def prepareBuffer(self, desiredSize: int):
        if self.bufferSize < desiredSize:
            oldBuffer = self.buffer
            oldSize = self.bufferSize

            self.buffer = bytes(desiredSize)
            self.bufferSize = desiredSize

            if oldSize > 0:
                self.buffer = memcpy(self.buffer, 0, oldBuffer, 0, oldSize)
            self.buffer = memset(self.buffer, oldSize, 0, (self.bufferSize - oldSize))

    def checkBufferContentSize(self, size: int, modifySize: bool):
        if size > self.bufferContentSize:
            if modifySize:
                self.prepareBuffer(size)
                self.bufferContentSize = size
            else:
                print("Size of ", size, " is greater than the current buffer content size ", self.bufferContentSize,
                      "! The next assertion will fail...", sep="")

        if size > self.bufferContentSize:
            print("Requested size = ", size, " vs. self.bufferContentSize = ", self.bufferContentSize, sep="")
            assert not modifySize
        assert (size <= self.bufferContentSize)
