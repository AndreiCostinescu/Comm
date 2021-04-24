from python.comm.comm_data.StatusData import StatusData
from python.comm.communication.Communication import Communication, SocketType, SocketPartner, MessageType
from threading import Thread

quitFlag = False
verbose = False
port = 8400


def createUDPEcho(socketType: SocketType):
    assert (socketType == SocketType.UDP or socketType == SocketType.UDP_HEADER)
    comm = Communication()
    comm.createSocket(socketType, SocketPartner(True, False), port, 2000, 50)
    status = StatusData()
    global quitFlag
    while not quitFlag:
        recvSuccess, messageType = comm.recvMessageType(socketType, 0, verbose)
        if not recvSuccess:
            print("Error when recvMessageType:", comm.getErrorCode(), "-", comm.getErrorString())
            quitFlag = True
            break
        elif messageType != MessageType.NOTHING:
            print("Received message type ", MessageType.messageTypeToString(messageType))

        if messageType == MessageType.STATUS:
            print("Connection from", comm.getPartner(socketType).getStringAddress())
            comm.setOverwritePartner(socketType, False)
            recvSuccess, status = comm.recvData(socketType, status, -1, verbose)  # type: bool, StatusData
            if not recvSuccess:
                print("Error when recvData (status): ", comm.getErrorCode(), ", ", comm.getErrorString(), sep="")
                quitFlag = True
                break

            print("Received", status.getDataSize(), "bytes:", status.getData().decode())
            comm.setOverwritePartner(socketType, True)


def main():
    global quitFlag
    server = Thread(target=createUDPEcho, args=(SocketType.UDP_HEADER,))
    server.start()
    while True:
        x = input()
        if x == "q":
            quitFlag = True
            break
    server.join()


if __name__ == "__main__":
    main()
