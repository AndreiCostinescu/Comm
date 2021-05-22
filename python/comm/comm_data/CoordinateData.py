from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.MessageType import MessageType
from comm.comm_socket.Buffer import Buffer


class CoordinateData(CommunicationData):
    headerSize = 2 * 4 + 2 * 8 + 8  # id & (buttonDwn, buttonFwd, touch): int, x & y: double, time: long long

    def __init__(self):
        super().__init__()
        self.id = -1
        self.time = 0
        self.x = 0
        self.y = 0
        self.touch = False
        self.buttonFwd = False
        self.buttonDwn = False

    def getMessageType(self) -> MessageType:
        return MessageType.COORDINATE

    def serialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(CoordinateData.headerSize)
            if verbose:
                print("Serialize: ", self.id, ", ", self.time, ", ", self.x, ", ", self.y, ", ", self.touch, ", ",
                      self.buttonFwd, ", ", self.buttonDwn)
            buffer.setInt(self.id, start)
            buffer.setLongLong(self.time, start + 4)
            buffer.setDouble(self.x, start + 12)
            buffer.setDouble(self.y, start + 20)
            buffer.setInt((self.buttonDwn << 2) + (self.buttonFwd << 1) + self.touch, start + 28)
            self.serializeState = 0
            return True
        else:
            print("Impossible serialize state...", self.serializeState)
            self.serializeState = 0
            return False

    def getExpectedDataSize(self):
        if self.serializeState == 0:
            return CoordinateData.headerSize
        else:
            raise RuntimeError("Impossible deserialize state... " + str(self.deserializeState))

    def deserialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.id = buffer.getInt(start)
            self.time = buffer.getLongLong(start + 4)
            self.x = buffer.getDouble(start + 12)
            self.y = buffer.getDouble(start + 20)
            tmp = buffer.getInt(start + 28)
            self.touch = tmp & 1
            self.buttonFwd = tmp & 2
            self.buttonDwn = tmp & 4
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state...", self.deserializeState)
            self.resetDeserializeState()
            return False
