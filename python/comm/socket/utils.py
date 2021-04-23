import struct
from typing import Tuple

SOCKET_ACCEPT_TIMEOUT_SECONDS: int = 1
CLIENT_MAX_MESSAGE_BYTES: int = 65500
SOCKET_BUFFER_RECV_SIZE: int = 4 * 1024 * 1024
SOCKET_BUFFER_SEND_SIZE: int = 4 * 1024 * 1024


def strToInt(s: str):
    index = s.find("\0")
    return int(s) if index == -1 else int(s[:index])


def strcmp(s1: str or bytes, s2: str or bytes):
    if isinstance(s1, str):
        s1 = bytes(s1, "ascii")
    if isinstance(s2, str):
        s2 = bytes(s2, "ascii")
    i = 0
    s1_len = len(s1)
    s2_len = len(s2)
    while True:
        if i == s1_len:
            return 0 if i == s2_len else -s2[i]
        if i == s2_len:
            return 0 if i == s1_len else s1[i]
        if s1[i] != s2[i]:
            return s1[i] - s2[i]
        i += 1


def memcpy(destBuffer: bytes, destStart: int, srcBuffer: bytes, srcStart: int, size: int) -> bytes:
    return destBuffer[:destStart] + srcBuffer[srcStart:srcStart + size] + destBuffer[destStart + size:]


def memset(destBuffer: bytes, destStart: int, value: int, size: int) -> bytes:
    res = b""
    byte_value = charToNetworkBytes(b"", 0, value)
    for i in range(size):
        res += byte_value
    return destBuffer[:destStart] + res + destBuffer[:destStart + size]


def charToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
    return buffer[:start] + value.to_bytes(1, 'big') + buffer[start + 1:]


def networkBytesToChar(buffer: bytes, start: int) -> int:
    return int.from_bytes(buffer[start:start + 1], 'big')


def shortToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
    return memcpy(buffer, start, value.to_bytes(2, 'big'), 0, 2)


def networkBytesToShort(buffer: bytes, start: int) -> int:
    return int.from_bytes(buffer[start:start + 2], 'big')


def intToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
    return memcpy(buffer, start, value.to_bytes(4, 'big'), 0, 4)


def networkBytesToInt(buffer: bytes, start: int) -> int:
    return int.from_bytes(buffer[start:start + 4], 'big')


def longLongToNetworkBytes(buffer: bytes, start: int, value: int) -> bytes:
    return memcpy(buffer, start, value.to_bytes(8, 'big'), 0, 8)


def networkBytesToLongLong(buffer: bytes, start: int) -> int:
    return int.from_bytes(buffer[start:start + 8], 'big')


def doubleToNetworkBytes(buffer: bytes, start: int, value: float) -> bytes:
    return memcpy(buffer, start, struct.pack("<d", value), 0, 8)


def networkBytesToDouble(buffer: bytes, start: int) -> float:
    return struct.unpack("<d", buffer[start:start + 8])[0]


def newBuffer(desiredLength: int):
    return b"\0" * desiredLength


def prepareBuffer(buffer: bytes, bufferLength: int, desiredLength: int):
    if bufferLength < desiredLength:
        buffer = newBuffer(desiredLength)
        bufferLength = desiredLength
    return buffer, bufferLength


def comparePartners(p1: Tuple, p2: Tuple):
    return p1 == p2
