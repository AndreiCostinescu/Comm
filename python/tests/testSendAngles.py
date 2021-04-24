import numpy as np
from python.comm.comm_data.StatusData import StatusData
from python.comm.communication.Communication import Communication, SocketType, SocketPartner, MessageType
from python.comm.comm_socket.Buffer import Buffer
from threading import Thread
from time import sleep

port = 8400
quitFlag = False


def calculateAngle(buffer: Buffer, verbose: bool = False) -> Buffer:
    # """
    angle_vector = [np.pi / 6, np.pi / 4, np.pi / 3, np.pi / 2, np.pi / 1, 2 * np.pi,
                    np.pi / 6, np.pi / 4, np.pi / 3, np.pi / 2, np.pi / 1, 2 * np.pi]
    """
    angle_vector = [np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6,
                    np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6, np.pi / 6]
    # """
    if verbose:
        formatStr = "{:08.3f}"
        fullFormatStr = " ".join([formatStr] * 6)
        print(fullFormatStr.format(*angle_vector[:6]))

    assert(len(angle_vector) == 12)
    buffer.setBufferContentSize(len(angle_vector) * 8)
    for i, angle in enumerate(angle_vector):
        print("Buffer content:", buffer.getBuffer())
        buffer.setDouble(angle, 8 * i)
    print("Buffer content:", buffer.getBuffer())
    return buffer


def sendAngles():
    comm = Communication()
    comm.createSocket(SocketType.UDP, SocketPartner.SocketPartner(("10.157.9.139", 25001), False))
    angleData = StatusData()
    buffer = Buffer(12 * 8)
    print(buffer.bufferSize, buffer.buffer, len(buffer.buffer))

    global quitFlag
    while not quitFlag:
        buffer = calculateAngle(buffer, True)
        angleData.setData(buffer.getBuffer(), buffer.getBufferContentSize())
        if not comm.sendData(SocketType.UDP, angleData, False):
            print("Could not send udp data... ", comm.getErrorString())
        sleep(0.5)


def main():
    t = Thread(target=sendAngles)
    t.start()
    global quitFlag
    while True:
        x = input()
        if x == "q":
            quitFlag = True
            break
    t.join()


if __name__ == "__main__":
    main()
