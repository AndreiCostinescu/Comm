//
// Created by Andrei on 20.04.21.
//

#include <cassert>
#include <comm/communication/Communication.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <thread>

using namespace comm;
using namespace std;

int port = 8400;
bool quit = false;
bool verbose = false;

void createUDPEcho(SocketType socketType) {
    assert (socketType == SocketType::UDP || socketType == SocketType::UDP_HEADER);
    Communication comm;
    comm.createSocket(socketType, SocketPartner(true, false), port, 2000, 50);

    StatusData status;
    MessageType messageType;
    while (!quit) {
        if (!comm.recvMessageType(socketType, &messageType, 0, verbose)) {
            cout << "Error when recvMessageType: " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
            quit = true;
            break;
        } else if (messageType != MessageType::NOTHING) {
            cout << "Received message type " << messageTypeToString(messageType) << endl;
        }
        if (messageType == MessageType::STATUS) {
            cout << "Connection from " << comm.getPartner(socketType)->getStringAddress() << endl;
            comm.setOverwritePartner(socketType, false);
            if (!comm.recvData(socketType, &status, -1, verbose)) {
                cout << "Error when recvData (status): " << comm.getErrorCode() << ", " << comm.getErrorString()
                     << endl;
                quit = true;
                break;
            }
            cout << "Received " << status.getDataSize() << " bytes: " << status.getData() << endl;
            comm.setOverwritePartner(socketType, true);
        }
    }
}

int main() {
    cout << "Hello World!" << endl;

    thread server(createUDPEcho, SocketType::UDP_HEADER);

    string x;
    while (true) {
        cin >> x;
        if (x == "q") {
            quit = true;
            break;
        }
    }
    server.join();

    return 0;
}