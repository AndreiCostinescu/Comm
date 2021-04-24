from abc import ABC, abstractmethod
from comm.comm_data.MessageType import MessageType
from comm.comm_socket.Buffer import Buffer
from typing import Optional


class CommunicationData(ABC):
    def __init__(self):
        self.serializeState = 0
        self.deserializeState = 0

    @abstractmethod
    def getMessageType(self) -> MessageType:
        pass

    def resetSerializeState(self):
        self.serializeState = 0

    @abstractmethod
    def serialize(self, buffer: Buffer, verbose: bool) -> bool:
        pass

    @abstractmethod
    def getExpectedDataSize(self) -> int:
        pass

    def getDeserializeBuffer(self) -> Optional[bytes]:
        return None

    def resetDeserializeState(self):
        self.deserializeState = 0

    @abstractmethod
    def deserialize(self, buffer: Buffer, start: int, verbose: bool) -> bool:
        pass
