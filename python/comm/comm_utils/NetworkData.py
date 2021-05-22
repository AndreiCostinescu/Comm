import struct
from utils import memcpy


class NetworkData:
    charSize = 1
    shortSize = 2
    intSize = 4
    longLongSize = 8
    floatSize = 4
    doubleSize = 8

    @staticmethod
    def charToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return buffer[:start] + value.to_bytes(1, 'big') + buffer[start + 1:]

    @staticmethod
    def networkBytesToChar(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + 1], 'big')

    @staticmethod
    def shortToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(2, 'big'), 0, 2)

    @staticmethod
    def networkBytesToShort(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + 2], 'big')

    @staticmethod
    def intToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(4, 'big'), 0, 4)

    @staticmethod
    def networkBytesToInt(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + 4], 'big')

    @staticmethod
    def longLongToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
        return memcpy(buffer, start, value.to_bytes(8, 'big'), 0, 8)

    @staticmethod
    def networkBytesToLongLong(buffer: bytes, start: int) -> int:
        return int.from_bytes(buffer[start:start + 8], 'big')

    @staticmethod
    def floatToNetworkBytes(buffer: bytes, start: int, value: float) -> bytes:
        return memcpy(buffer, start, struct.pack("<f", value), 0, 4)

    @staticmethod
    def networkBytesToFloat(buffer: bytes, start: int) -> float:
        return struct.unpack("<f", buffer[start:start + 4])[0]

    @staticmethod
    def doubleToNetworkBytes(buffer: bytes, start: int, value: float) -> bytes:
        return memcpy(buffer, start, struct.pack("<d", value), 0, 8)

    @staticmethod
    def networkBytesToDouble(buffer: bytes, start: int) -> float:
        return struct.unpack("<d", buffer[start:start + 8])[0]
