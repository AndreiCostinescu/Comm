//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 02-Apr-21.
//

#include <cassert>
#include <cstring>
#include <iostream>
#include <comm/socket/utils.h>
#include <comm/utils/NetworkIncludes.h>
#include <thread>

using namespace comm;
using namespace std;

int port = 8403;

void printSocketDetails(SOCKET s) {
    SocketAddress tmpPartner, tmpLocal;
    SocketAddressLength tmpLength = sizeof(SocketAddress);
    getpeername(s, (sockaddr *) (&tmpPartner), &tmpLength);
    cout << this_thread::get_id() << ": Socket partner is " << getStringAddress(tmpPartner) << endl;
    getsockname(s, (sockaddr *) (&tmpLocal), &tmpLength);
    cout << this_thread::get_id() << ": Socket local addr. is " << getStringAddress(tmpLocal) << endl;
}

void serverInit(SOCKET &serverSocket, const int portNumber) {
    struct sockaddr_in serverAddress{};
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << this_thread::get_id() << ": " << "ERROR opening socket" << endl;
        exit(-1);
    } else {
        cout << this_thread::get_id() << ": " << "Opened Socket" << endl;
    }

    memset((char *) &serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        cerr << this_thread::get_id() << ": ERROR on binding" << endl;
        exit(-1);
    } else {
        cout << this_thread::get_id() << ": Bound socket!" << endl;
    }

    cout << this_thread::get_id() << ": Initialized serverSocket " << getStringAddress(serverAddress) << endl;
    printSocketDetails(serverSocket);
}

void server() {
    startupSockets();

    SOCKET serverSocket, clientSocket;
    SocketAddress clientAddress;
    SocketAddressLength clientAddressLength = sizeof(clientAddress);
    thread::id x = this_thread::get_id();
    struct timeval socketAcceptTimeout{};
    fd_set readFileDescriptors;

    serverInit(serverSocket, port);

    cout << x << ": Created tcp server" << endl;

    listen(serverSocket, 5);
    socketAcceptTimeout.tv_sec = SOCKET_ACCEPT_TIMEOUT_SECONDS;
    socketAcceptTimeout.tv_usec = 0;
    cout << x << ": listening for connections..." << endl;

    // wait for first connection
    while (true) {
        cout << x << ": In while loop, waiting for connection..." << endl;
        FD_ZERO(&readFileDescriptors);
        FD_SET(serverSocket, &readFileDescriptors);
        int selectResult = select(serverSocket + 1, &readFileDescriptors, (fd_set *) nullptr, (fd_set *) nullptr,
                                  &socketAcceptTimeout);
        if (selectResult > 0) {
            // The accept() call actually accepts an incoming connection
            clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
            cout << x << ": " << "Found connection... " << getStringAddress(clientAddress) << endl;
            break;
        } else if (selectResult == SOCKET_ERROR) {
            cout << "lastError = " << getLastError() << endl;
            exit(-1);
        } else {
            // Timeout occurred!
            assert (selectResult == 0);
        }
    }
    cout << x << ": Initialized client socket " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

    cout << endl << endl;
    this_thread::sleep_for(std::chrono::seconds(1));

    const char *data = "Hello";
    int dataSize = 6;
    char *buffer = new char[100];

    recv(clientSocket, buffer, dataSize, 0);
    cout << x << ": After recv from " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

    cout << endl << endl;
    this_thread::sleep_for(std::chrono::seconds(1));

    send(clientSocket, data, dataSize, 0);
    cout << x << ": After send to " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

#if defined(_WIN32) || defined(_WIN64)
    closesocket(clientSocket);
    closesocket(serverSocket);
#else
    close(clientSocket);
    close(serverSocket);
#endif

    cout << x << ": server finished normally!" << endl;
}

void test_1() {
    startupSockets();

    this_thread::sleep_for(std::chrono::seconds(2));
    thread::id x = this_thread::get_id();
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    SocketAddress clientAddress;
    memset((char *) &clientAddress, '\0', sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddress.sin_port = htons(port);

    if (connect(clientSocket, (struct sockaddr *) (&clientAddress), sizeof(SocketAddress)) < 0) {
        (*cerror) << "Connection failed due to port and ip problems" << endl;
        throw runtime_error("Connection failed due to port and ip problems!");
    }
    cout << x << ": Connected to " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

    cout << endl << endl;
    this_thread::sleep_for(std::chrono::seconds(1));

    const char *data = "Hello";
    int dataSize = 6;
    char *buffer = new char[100];

    send(clientSocket, data, dataSize, 0);
    cout << x << ": After send to " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

    cout << endl << endl;
    this_thread::sleep_for(std::chrono::seconds(1));

    recv(clientSocket, buffer, dataSize, 0);
    cout << x << ": After recv from " << getStringAddress(clientAddress) << endl;
    printSocketDetails(clientSocket);

#if defined(_WIN32) || defined(_WIN64)
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif

    cout << x << ": test_1 finished normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t(server);
    test_1();
    t.join();

    return 0;
}
