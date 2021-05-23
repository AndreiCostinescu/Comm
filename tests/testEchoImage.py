import cv2 as cv
from comm.comm_data.ImageData import ImageData
from comm.comm_data.Messages import Messages
from comm.comm_data.StatusData import StatusData
from comm.communication.Communication import Communication, SocketType, SocketPartner, MessageType
from comm.communication.TCPServer import TCPServer
from comm.comm_utils.utils import strcmp
from threading import Thread
from time import sleep

quitFlag = False
verbose = False
port = 8400


def createUDPEcho(socketType: SocketType):
    assert (socketType == SocketType.UDP or socketType == SocketType.UDP_HEADER)
    comm = Communication()
    comm.createSocket(socketType, SocketPartner(True, False), port, 2000, 50)
    image = ImageData()
    npImage = None
    global quitFlag
    while not quitFlag:
        if npImage is not None:
            cv.imshow("Received Image", npImage)
            cv.waitKey(5)
        recvSuccess, messageType = comm.recvMessageType(socketType, False, 0, verbose)
        if not recvSuccess:
            print("Error when recvMessageType:", comm.getErrorCode(), "-", comm.getErrorString())
            quitFlag = True
            break
        elif messageType != MessageType.NOTHING:
            print("Received message type", MessageType.messageTypeToString(messageType))
        else:
            sleep(0.5)
        if messageType == MessageType.IMAGE:
            print("Connection from", comm.getPartner(socketType).getStringAddress())
            comm.setOverwritePartner(socketType, False)
            recvSuccess = comm.recvData(socketType, image, False, True, -1, verbose)  # type: bool
            if not recvSuccess:
                if comm.getErrorCode() >= 0:
                    print("Error when recvData (status): ", comm.getErrorCode(), ", ", comm.getErrorString(), sep="")
                    quitFlag = True
                    break
                else:
                    comm.setOverwritePartner(socketType, True)
                    continue
            if not image.isImageDeserialized():
                print("Could not completely deserialize the image...")
                continue
            comm.setOverwritePartner(socketType, True)
            print("Received image with id", image.id, "and shape", image.image.shape)
            npImage = image.image


def imageStream(comm: Communication, threadID: int):
    socketType = SocketType.TCP
    print("Connection from", comm.getPartner(socketType).getStringAddress())
    image = ImageData()
    status = StatusData()
    npImage = None

    while not quitFlag:
        if npImage is not None:
            cv.imshow("Received Image " + str(threadID), npImage)
            cv.waitKey(5)
        recvSuccess, messageType = comm.recvMessageType(socketType, False, 0, verbose)
        if not recvSuccess:
            print("Error when recvMessageType:", comm.getErrorCode(), "-", comm.getErrorString())
            break
        elif messageType != MessageType.NOTHING:
            print("Received message type", MessageType.messageTypeToString(messageType))
        else:
            sleep(0.5)
        if messageType == MessageType.IMAGE:
            recvSuccess = comm.recvData(socketType, image, False, True, -1, verbose)  # type: bool
            if not recvSuccess:
                if comm.getErrorCode() >= 0:
                    print("Error when recvData (image): ", comm.getErrorCode(), ", ", comm.getErrorString(), sep="")
                    break
                else:
                    continue
            if not image.isImageDeserialized():
                print("Could not completely deserialize the image...")
                continue
            print("Received image with id", image.id, "and shape", image.image.shape)
            npImage = image.image
        elif messageType == MessageType.STATUS:
            recvSuccess = comm.recvData(socketType, status, False, True, -1, verbose)  # type: bool
            if not recvSuccess:
                if comm.getErrorCode() >= 0:
                    print("Error when recvData (status): ", comm.getErrorCode(), ", ", comm.getErrorString(), sep="")
                    break
                else:
                    continue
            if strcmp(status.getData(), Messages.QUIT_MESSAGE) == 0:
                break


def createTCPEcho():
    tcpServer = TCPServer(port)
    threadList = []
    global quitFlag
    while not quitFlag:
        comm = tcpServer.acceptCommunication()
        if comm is not None:
            t = Thread(target=imageStream, args=(comm, len(threadList)))
            t.start()
            threadList.append(t)
    for t in threadList:
        t.join()


def main():
    global quitFlag
    tcpServer = Thread(target=createTCPEcho)
    tcpServer.start()
    udpServer = Thread(target=createUDPEcho, args=(SocketType.UDP_HEADER,))
    udpServer.start()
    while True:
        x = input()
        if x == "q":
            quitFlag = True
            break
    udpServer.join()
    tcpServer.join()


if __name__ == "__main__":
    main()
