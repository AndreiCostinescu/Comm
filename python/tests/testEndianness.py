import socket
import sys
from comm.comm_socket.utils import prepareBuffer
from comm.comm_utils.NetworkData import NetworkData


def test():
    print("Hello World!")
    print("Am I big endian?", sys.byteorder)

    t = 48585
    print(t, socket.ntohs(t), socket.htons(t))
    t = 1024 * 48585
    print(t, socket.ntohs(t), socket.htons(t))

    buffer = b""
    bufferSize = 0

    x = 1 << 63
    print(x)
    for factor in range(2, 15):
        print("Factor =", factor)
        prevY = -1
        y = 0
        while y < x - 1:
            if prevY == y:
                break
            buffer, bufferSize = prepareBuffer(buffer, bufferSize, 8)
            buffer = NetworkData.longLongToNetworkBytes(buffer, 0, y)
            resLL = NetworkData.networkBytesToLongLong(buffer, 0)
            if resLL != y:
                print("Error at y = ", y, ", res = ", resLL, sep="")
            assert (resLL == y)
            prevY = y
            y = (y + 1) * factor
    print(x)

    buffer, bufferSize = prepareBuffer(buffer, bufferSize, 8)
    buffer = NetworkData.longLongToNetworkBytes(buffer, 0, x)
    assert (NetworkData.networkBytesToLongLong(buffer, 0) == x)
    print("Finished long-long-tests")

    for y in range(0, 1 << 20):
        buffer, bufferSize = prepareBuffer(buffer, bufferSize, 4)
        buffer = NetworkData.intToNetworkBytes(buffer, 0, y)
        res = NetworkData.networkBytesToInt(buffer, 0)
        if res != y:
            print("Error at y = ", y, ", res = ", res, sep="")
        assert (res == y)

    yI = 1 << 31
    print(yI)
    buffer, bufferSize = prepareBuffer(buffer, bufferSize, 4)
    buffer = NetworkData.intToNetworkBytes(buffer, 0, yI)
    assert (NetworkData.networkBytesToInt(buffer, 0) == yI)
    print("Finished int-tests")

    for y in range(0, 1 << 13):
        buffer, bufferSize = prepareBuffer(buffer, bufferSize, 2)
        buffer = NetworkData.shortToNetworkBytes(buffer, 0, y)
        resS = NetworkData.networkBytesToShort(buffer, 0)
        if resS != y:
            print("Error at y = ", y, ", res = ", resS, sep="")
        assert (resS == y)
    print("Finished short-tests")

    for factor in range(2, 5):
        z = 1.0
        while z > 1e-46:
            print(z)
            buffer, bufferSize = prepareBuffer(buffer, bufferSize, 4)
            buffer = NetworkData.floatToNetworkBytes(buffer, 0, z)
            resF = NetworkData.networkBytesToFloat(buffer, 0)
            if resF != z:
                print("Error at z = ", z, ", res = ", resF, sep="")
            assert (resF == z)
            z /= factor
    print("Finished float-tests")

    for factor in range(2, 5):
        z = 1.0
        while z > 1e-301:
            print(z)
            buffer, bufferSize = prepareBuffer(buffer, bufferSize, 8)
            buffer = NetworkData.doubleToNetworkBytes(buffer, 0, z)
            resD = NetworkData.networkBytesToDouble(buffer, 0)
            if resD != z:
                print("Error at z = ", z, ", res = ", resD, sep="")
            assert (resD == z)
            z /= factor
    print("Finished double-tests")


if __name__ == "__main__":
    test()
