from python.comm.data.CommunicationData import CommunicationData
from python.comm.data.MessageType import MessageType
from python.comm.data.Messages import Messages
from python.comm.socket.Buffer import Buffer
from python.comm.socket.utils import prepareBuffer, strcmp, memcpy, strToCStr
from typing import Optional


class StatusData(CommunicationData):
    headerSize = 4
    statusDataType = MessageType.STATUS.value[0]

    def __init__(self, value: str or int = None):
        super().__init__()
        self.data = None  # type: Optional[bytes]
        self.dataSize = 0
        self.dataLength = 0
        self.dataType = 0
        if isinstance(value, int) and value > 0:
            self.data, self.dataLength = prepareBuffer(self.data, self.dataLength, value)
        elif isinstance(value, str):
            self.setCommand(value)
        elif isinstance(value, bytes):
            self.setData(value)

    def getMessageType(self):
        # print("Status Data messageType:", self.getDataType())
        return MessageType.intToMessageType(self.getDataType())

    def serialize(self, buffer: Buffer, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(StatusData.headerSize)
            # print("This dataSize = " + str(self.dataSize))
            buffer.setInt(self.dataSize, 0)
            if verbose:
                dataBuffer = buffer.getBuffer()
                print("buffer int content: ", int(dataBuffer[0]), " ", int(dataBuffer[1]), " ", int(dataBuffer[2]), " ",
                      int(dataBuffer[3]))
            self.serializeState = 1
            return False
        elif self.serializeState == 1:
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

    def deserialize(self, buffer: Buffer, start: int, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.dataSize = buffer.getInt(start)
            self.deserializeState = 1
            return False
        elif self.deserializeState == 1:
            assert (buffer.getBufferContentSize() == self.dataSize)
            self.setData(buffer.getBuffer(), self.dataSize)
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state... " + str(self.deserializeState))
            self.resetDeserializeState()
            return False

    def reset(self):
        self.dataSize = -1
        self.dataType = MessageType.NOTHING.value[0]  # .value returns a tuple

    def setCommand(self, command: str):
        if command == "_reset":
            self.reset()
            return
        elif command == "ping":
            commandData = Messages.PING_MESSAGE
            self.dataSize = Messages.PING_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "quit":
            commandData = Messages.QUIT_MESSAGE
            self.dataSize = Messages.QUIT_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "start":
            commandData = Messages.START_MESSAGE
            self.dataSize = Messages.START_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "stop":
            commandData = Messages.STOP_MESSAGE
            self.dataSize = Messages.STOP_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "wait":
            commandData = Messages.WAIT_MESSAGE
            self.dataSize = Messages.WAIT_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "accept":
            commandData = Messages.ACCEPT_MESSAGE
            self.dataSize = Messages.ACCEPT_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "ready":
            commandData = Messages.READY_MESSAGE
            self.dataSize = Messages.READY_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "control":
            commandData = Messages.CONTROL_MESSAGE
            self.dataSize = Messages.CONTROL_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "upload":
            commandData = Messages.UPLOAD_MESSAGE
            self.dataSize = Messages.UPLOAD_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "select":
            commandData = Messages.SELECT_MESSAGE
            self.dataSize = Messages.SELECT_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif command == "reject":
            commandData = Messages.REJECT_MESSAGE
            self.dataSize = Messages.REJECT_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        else:
            raise RuntimeError("Unknown command: " + command)

        self.data, self.dataLength = prepareBuffer(self.data, self.dataLength, self.dataSize)
        self.data = memcpy(self.data, 0, bytes(commandData, "ascii"), 0, self.dataSize)
        assert (strcmp(commandData, self.data.decode()) == 0)
        assert (strcmp(bytes(commandData, "ascii"), self.data) == 0)

    def setStatus(self, status: str):
        if status == "_reset":
            self.reset()
            return
        elif status == "idle":
            statusData = Messages.IDLE_MESSAGE
            self.dataSize = Messages.IDLE_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif status == "active":
            statusData = Messages.ACTIVE_MESSAGE
            self.dataSize = Messages.ACTIVE_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        elif status == "done":
            statusData = Messages.DONE_MESSAGE
            self.dataSize = Messages.DONE_MESSAGE_LENGTH
            self.dataType = MessageType.STATUS.value[0]
        else:
            raise RuntimeError("Unknown status: " + status)

        self.data, self.dataLength = prepareBuffer(self.data, self.dataLength, self.dataSize)
        self.data = memcpy(self.data, 0, statusData, 0, self.dataSize)
        assert (strcmp(statusData, self.data) == 0)

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
        self.dataType = StatusData.statusDataType

    def getData(self) -> Optional[bytes]:
        if self.dataSize < 0:
            return None
        return strToCStr(self.data)

    def getDataSize(self) -> int:
        return self.dataSize

    def getDataType(self) -> int:
        return self.dataType
