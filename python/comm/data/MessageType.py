import enum


class MessageType(enum.Enum):
    NOTHING = 0,
    STATUS = 1,
    IMAGE = 2,
    COORDINATE = 3,

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
        else:
            raise RuntimeError("Unknown MessageType " + str(messageType))
