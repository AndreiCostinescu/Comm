from typing import Tuple

SOCKET_ACCEPT_TIMEOUT_SECONDS: int = 1
CLIENT_MAX_MESSAGE_BYTES: int = 65500
SOCKET_BUFFER_RECV_SIZE: int = 4 * 1024 * 1024
SOCKET_BUFFER_SEND_SIZE: int = 4 * 1024 * 1024


def newBuffer(desiredLength: int):
    return b"\0" * desiredLength


def prepareBuffer(buffer: bytes, bufferLength: int, desiredLength: int):
    if bufferLength < desiredLength:
        buffer = newBuffer(desiredLength)
        bufferLength = desiredLength
    return buffer, bufferLength


def comparePartners(p1: Tuple, p2: Tuple):
    return p1 == p2
