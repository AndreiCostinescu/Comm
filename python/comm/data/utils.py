from python.comm.data.MessageType import MessageType
from python.comm.data.CoordinateData import CoordinateData
from python.comm.data.ImageData import ImageData
from python.comm.data.StatusData import StatusData


def createCommunicationData(messageType: MessageType):
    if messageType == MessageType.NOTHING:
        return None
    elif messageType == MessageType.STATUS:
        return StatusData()
    elif messageType == MessageType.IMAGE:
        return ImageData()
    elif messageType == MessageType.COORDINATE:
        return CoordinateData()
    else:
        raise RuntimeError("Unknown MessageType: " + str(messageType))
