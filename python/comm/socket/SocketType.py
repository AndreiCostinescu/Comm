import enum


class SocketType(enum.Enum):
    UDP = 0,
    UDP_HEADER = 1,
    TCP = 2

    @staticmethod
    def intToMessageType(i: int):
        if i == 0:
            return SocketType.UDP
        elif i == 1:
            return SocketType.UDP_HEADER
        elif i == 2:
            return SocketType.TCP
        else:
            raise RuntimeError("Unknown SocketType " + str(i))

    @staticmethod
    def stringToSocketType(s: str):
        if s == "UDP":
            return SocketType.UDP
        elif s == "UDP_HEADER":
            return SocketType.UDP_HEADER
        elif s == "TCP":
            return SocketType.TCP
        else:
            raise RuntimeError("Unknown SocketType " + s)

    @staticmethod
    def socketTypeToString(socketType: 'SocketType'):
        if socketType == SocketType.UDP:
            return "UDP"
        elif socketType == SocketType.UDP_HEADER:
            return "UDP_HEADER"
        elif socketType == SocketType.TCP:
            return "TCP"
        else:
            raise RuntimeError("Unknown SocketType " + str(socketType))