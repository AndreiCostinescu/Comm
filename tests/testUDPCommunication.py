from comm.comm_data.StatusData import StatusData
from comm.communication.Communication import Communication, SocketType, SocketPartner
from comm.comm_utils.utils import strcmp
from threading import Thread
from time import sleep

verbose = True
port = 8400


def server_1(socketType: SocketType):
    x = "Thread 2"
    p = Communication()
    p.createSocket(socketType, None, port, 2000, 5000)
    print("Created partner communication...")
    status = StatusData()

    while True:
        print(x, ": In while loop, waiting for connection on ", p.getMyself(socketType).getStringAddress(), "...")
        assert (p.recvData(socketType, status, False, False, 0, verbose) or p.getErrorCode() == -1);
        if p.getErrorCode() == -1:
            sleep(0.5)
            continue
        if status.getData() is not None:
            print("\n\n")
            print(x, ": Created partner communication with ", p.getPartnerString(socketType))
            print(x, ": Received: \"", status.getData(), "\" from partner", sep="")
            assert (strcmp(status.getData(), "Ce faci?") == 0);
            sleep(0.5)
            print("\n\n")
            status.setData("Bine!")
            sleep(0.5)
            print("\n\n")
            assert (p.transmitData(socketType, status, False, False, -1, verbose))
            break
        sleep(0.5)

    print(x, ": Ending server normally!")


def test_1(socketType: SocketType):
    sleep(2)

    x = "Thread 1"
    status = StatusData()
    p = Communication()
    p.createSocket(socketType, SocketPartner.SocketPartner(("127.0.0.1", port), False), 0, -1, 5000)
    print(x, ": Created partner communication")
    status.setData("Ce faci?")
    print(x, ": Created status data")
    sleep(1)
    print("\n\n")
    assert (p.transmitData(socketType, status, False, False, -1, verbose))
    print(x, ": Sent status data")
    sleep(1)
    print("\n\n")
    assert (p.recvData(socketType, status, False, False, -1, verbose))
    print(x, ": Received status data")
    print(x, ": Received: \"", status.getData(), "\" from partner", sep="")
    assert (strcmp(status.getData(), "Bine!") == 0)

    print(x, ": Ending test_1 normally!")


def main():
    server = Thread(target=server_1, args=(SocketType.UDP,))
    server.start()
    test_1(SocketType.UDP)
    server.join()

    print("\n\n\n")
    sleep(2)

    server = Thread(target=server_1, args=(SocketType.UDP_HEADER,))
    server.start()
    test_1(SocketType.UDP_HEADER)
    server.join()


if __name__ == "__main__":
    main()
