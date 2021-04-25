from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.MessageType import MessageType
from comm.comm_socket.Buffer import Buffer


class BytesData(CommunicationData):
    headerSize = 4

    def __init__(self, value: int = 0):
        super().__init__()
        self.data = Buffer(value)  # type: Buffer
        self.expectedDataSize = 0

    def getMessageType(self):
        return MessageType.BYTES

    def serialize(self, buffer: Buffer, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(BytesData.headerSize)
            # print("This dataSize = " + str(self.dataSize))
            buffer.setInt(self.data.getBufferContentSize(), 0)
            if verbose:
                dataBuffer = buffer.getBuffer()
                print("buffer int content: ", int(dataBuffer[0]), " ", int(dataBuffer[1]), " ", int(dataBuffer[2]), " ",
                      int(dataBuffer[3]))
            self.serializeState = 1
            return False
        elif self.serializeState == 1:
            buffer.setReferenceToData(self.data.getBuffer(), self.data.getBufferContentSize())
            self.serializeState = 0
            return True
        else:
            print("Impossible serialize state...", self.serializeState)
            self.serializeState = 0
            return False

    def getExpectedDataSize(self) -> int:
        if self.deserializeState == 0:
            return BytesData.headerSize
        elif self.deserializeState == 1:
            return self.expectedDataSize
        else:
            raise RuntimeError("Impossible deserialize state... " + str(self.deserializeState))

    def deserialize(self, buffer: Buffer, start: int, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.expectedDataSize = buffer.getInt(start)
            self.deserializeState = 1
            return False
        elif self.deserializeState == 1:
            assert (buffer.getBufferContentSize() == self.expectedDataSize)
            self.data.setData(buffer.getBuffer(), self.expectedDataSize)
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state... " + str(self.deserializeState))
            self.resetDeserializeState()
            return False

    def reset(self):
        self.data.reset()
        self.expectedDataSize = 0

    def setData(self, _data: str or bytes, dataSize: int, start: int = 0, offset: int = 0):
        self.data.setData(_data, dataSize, start, offset)

    def setReferenceToData(self, _data: bytes, dataSize: int):
        self.data.setReferenceToData(_data, dataSize)

    def setChar(self, _data: int, position: int):
        self.data.setChar(_data, position)

    def setShort(self, _data: int, position: int):
        self.data.setShort(_data, position)

    def setInt(self, _data: int, position: int):
        self.data.setInt(_data, position)

    def setLongLong(self, _data: int, position: int):
        self.data.setLongLong(_data, position)

    def setFloat(self, _data: float, position: int):
        self.data.setFloat(_data, position)

    def setDouble(self, _data: float, position: int):
        self.data.setDouble(_data, position)

    def getChar(self, position: int):
        return self.data.getChar(position)

    def getShort(self, position: int):
        return self.data.getShort(position)

    def getInt(self, position: int):
        return self.data.getInt(position)

    def getLongLong(self, position: int):
        return self.data.getLongLong(position)

    def getFloat(self, position: int):
        return self.data.getFloat(position)

    def getDouble(self, position: int):
        return self.data.getDouble(position)

    def empty(self):
        return self.data.empty()

    def getBuffer(self):
        return self.data.getBuffer()

    def getBufferSize(self):
        return self.data.getBufferContentSize()
