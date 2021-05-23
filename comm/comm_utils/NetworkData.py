import struct
from comm.comm_utils.utils import memcpy


class NetworkData:
    charSize = 1
    shortSize = 2
    intSize = 4
    longLongSize = 8
    floatSize = 4
    doubleSize = 8

    @staticmethod
    def charToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return buffer[:start] + value.to_bytes(NetworkData.charSize, 'big') + buffer[start + NetworkData.charSize:]

    @staticmethod
    def networkBytesToChar(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + NetworkData.charSize], 'big')

    @staticmethod
    def shortToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(NetworkData.shortSize, 'big'), 0, NetworkData.shortSize)

    @staticmethod
    def networkBytesToShort(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + NetworkData.shortSize], 'big')

    @staticmethod
    def intToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(NetworkData.intSize, 'big'), 0, NetworkData.intSize)

    @staticmethod
    def networkBytesToInt(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + NetworkData.intSize], 'big')

    @staticmethod
    def longLongToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(NetworkData.longLongSize, 'big'), 0, NetworkData.longLongSize)

    @staticmethod
    def networkBytesToLongLong(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + NetworkData.longLongSize], 'big')

    @staticmethod
    def floatToNetworkBytes(buffer: bytes, start: int, value: float) -> bytes:
        return memcpy(buffer, start, struct.pack("<f", value), 0, NetworkData.floatSize)

    @staticmethod
    def networkBytesToFloat(buffer: bytes, start: int) -> float:
        return struct.unpack("<f", buffer[start:start + NetworkData.floatSize])[0]

    @staticmethod
    def doubleToNetworkBytes(buffer: bytes, start: int, value: float) -> bytes:
        return memcpy(buffer, start, struct.pack("<d", value), 0, NetworkData.doubleSize)

    @staticmethod
    def networkBytesToDouble(buffer: bytes, start: int) -> float:
        return struct.unpack("<d", buffer[start:start + NetworkData.doubleSize])[0]
