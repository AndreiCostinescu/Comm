import enum


class MessageType(enum.Enum):
    NOTHING = 0,
    STATUS = 1,
    IMAGE = 2,
    COORDINATE = 3,
    BYTES = 4,

    @staticmethod
    def intToMessageType(i: int):
        if i == 0:
            return MessageType.NOTHING
        elif i == 1:
            return MessageType.STATUS
        elif i == 2:
            return MessageType.IMAGE
        elif i == 3:
            return MessageType.COORDINATE
        elif i == 4:
            return MessageType.BYTES
        else:
            raise RuntimeError("Unknown MessageType " + str(i))

    @staticmethod
    def stringToMessageType(s: str):
        if s == "nothing":
            return MessageType.NOTHING
        elif s == "status":
            return MessageType.STATUS
        elif s == "image":
            return MessageType.IMAGE
        elif s == "coordinate":
            return MessageType.COORDINATE
        elif s == "bytes":
            return MessageType.BYTES
        else:
            raise RuntimeError("Unknown MessageType " + s)

    @staticmethod
    def messageTypeToString(messageType: 'MessageType'):
        if messageType == MessageType.NOTHING:
            return "nothing"
        elif messageType == MessageType.STATUS:
            return "status"
        elif messageType == MessageType.IMAGE:
            return "image"
        elif messageType == MessageType.COORDINATE:
            return "coordinate"
        elif messageType == MessageType.BYTES:
            return "bytes"
        else:
            raise RuntimeError("Unknown MessageType " + str(messageType))
