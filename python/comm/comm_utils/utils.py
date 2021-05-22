import sys


def strToCStr(s: str or bytes):
    if isinstance(s, bytes):
        to_find = 0
    else:
        to_find = "\0"
    index = s.find(to_find)
    return s if index == -1 else s[:index]


def strToInt(s: str or bytes):
    return int(strToCStr(s))


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


def memcpy(destBuffer: bytes or bytearray, destStart: int, srcBuffer: bytes, srcStart: int, size: int) -> bytes:
    """
    initSize = len(destBuffer)
    newBuffer = destBuffer[:destStart] + srcBuffer[srcStart:srcStart + size] + destBuffer[destStart + size:]
    newSize = len(newBuffer)
    assert (initSize == newSize), "{} vs. {}".format(initSize, newSize)
    """
    if isinstance(destBuffer, bytearray):
        destBuffer[destStart:destStart + size] = srcBuffer[srcStart:srcStart + size]
        return destBuffer
    return destBuffer[:destStart] + srcBuffer[srcStart:srcStart + size] + destBuffer[destStart + size:]


def memset(destBuffer: bytes or bytearray, destStart: int, value: int, size: int) -> bytes:
    if isinstance(destBuffer, bytearray):
        destBuffer[destStart:destStart + size] = [value] * size
        return destBuffer
    return destBuffer[:destStart] + bytes([value] * size) + destBuffer[destStart + size:]


def hostToNetworkBytes(data: bytes):
    return data if sys.byteorder == "big" else data[::-1]


def networkToHostBytes(data: bytes):
    return data if sys.byteorder == "big" else data[::-1]
