from comm.comm_data.MessageType import MessageType
from comm.comm_data.BytesData import BytesData
from comm.comm_data.CoordinateData import CoordinateData
from comm.comm_data.ImageData import ImageData
from comm.comm_data.StatusData import StatusData


def createCommunicationData(messageType: MessageType):
    if messageType == MessageType.NOTHING:
        return None
    elif messageType == MessageType.STATUS:
        return StatusData()
    elif messageType == MessageType.IMAGE:
        return ImageData()
    elif messageType == MessageType.COORDINATE:
        return CoordinateData()
    elif messageType == MessageType.BYTES:
        return BytesData()
    else:
        raise RuntimeError("Unknown MessageType: " + str(messageType))
