from python.comm.data.CommunicationData import CommunicationData
from python.comm.data.utils import createCommunicationData
from python.comm.data.MessageType import MessageType
from typing import Dict


class DataCollection:
    def __init__(self):
        self.data = {}  # type: Dict[str, CommunicationData]

    def get(self, messageType: MessageType):
        communicationData = self.data.get(MessageType.messageTypeToString(messageType), None)
        if messageType != MessageType.NOTHING and communicationData is None:
            self.data[MessageType.messageTypeToString(messageType)] = createCommunicationData(messageType)
        return communicationData
