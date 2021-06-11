from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.MessageType import MessageType
from comm.comm_data.Messages import Messages
from comm.comm_socket.utils import prepareBuffer
from comm.comm_utils.Buffer import Buffer
from comm.comm_utils.utils import strcmp, memcpy, strToCStr
from typing import Optional


class StatusData(CommunicationData):
    headerSize = 4

    def __init__(self, value: str or int = None):
        super().__init__()
        self.data = None  # type: Optional[bytes]
        self.dataSize = 0
        self.dataLength = 0
        if isinstance(value, int) and value > 0:
            self.data, self.dataLength = prepareBuffer(self.data, self.dataLength, value)
        elif isinstance(value, str):
            self.setData(value)
        elif isinstance(value, bytes):
            self.setData(value)

    def getMessageType(self):
        return MessageType.STATUS

    def serialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(StatusData.headerSize)
            # print("This dataSize = " + str(self.dataSize))
            buffer.setInt(self.dataSize, start)
            if verbose:
                dataBuffer = buffer.getBuffer()
                print("buffer int content: ", int(dataBuffer[start]), " ", int(dataBuffer[start + 1]), " ",
                      int(dataBuffer[start + 2]), " ", int(dataBuffer[start + 3]))
            self.serializeState = 1
            return False
        elif self.serializeState == 1:
            if forceCopy:
                buffer.setData(self.data, self.dataSize, start)
            else:
                if start != 0:
                    raise RuntimeError("Can not set a reference to data not starting at the first position!")
                buffer.setReferenceToData(self.data, self.dataSize)
            self.serializeState = 0
            return True
        else:
            print("Impossible serialize state...", self.serializeState)
            self.serializeState = 0
            return False

    def getExpectedDataSize(self) -> int:
        if self.deserializeState == 0:
            return StatusData.headerSize
        elif self.deserializeState == 1:
            return self.dataSize
        else:
            raise RuntimeError("Impossible deserialize state... " + str(self.deserializeState))

    def deserialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.dataSize = buffer.getInt(start)
            self.deserializeState = 1
            return False
        elif self.deserializeState == 1:
            assert ((buffer.getBufferContentSize() - start) == self.dataSize)
            self.setData(buffer.getBuffer(), self.dataSize)
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state... " + str(self.deserializeState))
            self.resetDeserializeState()
            return False

    def reset(self):
        self.dataSize = -1

    def setData(self, _data: str or bytes, _dataSize: int = -1):
        if _data is None:
            self.dataSize = -1
            return

        if isinstance(_data, str):
            _data = bytes(_data, "ascii")
        if not _data.endswith(b"\x00"):
            _data += b"\x00"
        if _dataSize == -1:
            _dataSize = len(_data)

        # print("Setting status data with:", _data, _dataSize)
        self.data, self.dataLength = prepareBuffer(self.data, self.dataLength, _dataSize)
        self.data = memcpy(self.data, 0, _data, 0, _dataSize)
        self.dataSize = _dataSize

    def getData(self) -> Optional[bytes]:
        if self.dataSize < 0:
            return None
        return strToCStr(self.data)

    def getDataSize(self) -> int:
        return self.dataSize
