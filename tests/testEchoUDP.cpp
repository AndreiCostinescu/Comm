//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 20.04.21.
//

#include <cassert>
#include <comm/communication/Communication.h>
#include <comm/data/ImageDataWithOpenCV.h>
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

    ImageDataWithOpenCV image;
    StatusData status;
    MessageType messageType;
    while (!quit) {
        if (!comm.recvMessageType(socketType, messageType, false, false, 0, verbose)) {
            cout << "Error when recvMessageType: " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
            quit = true;
            break;
        } else if (messageType != MessageType::NOTHING) {
            cout << "Received message type " << messageTypeToString(messageType) << endl;
        }
        if (messageType == MessageType::STATUS) {
            cout << "Connection from " << comm.getPartner(socketType)->getStringAddress() << endl;
            comm.setOverwritePartner(socketType, false);
            if (!comm.recvData(socketType, &status, false, false, true, -1, verbose)) {
                cout << "Error when recvData (status): " << comm.getErrorCode() << ", " << comm.getErrorString()
                     << endl;
                quit = true;
                break;
            }
            cout << "Received " << status.getDataSize() << " bytes: " << status.getData() << endl;
            comm.setOverwritePartner(socketType, true);
        } else if (messageType == MessageType::IMAGE) {
            cout << "Connection from " << comm.getPartner(socketType)->getStringAddress() << endl;
            comm.setOverwritePartner(socketType, false);
            if (!comm.recvData(socketType, &image, false, false, true, -1, verbose)) {
                cout << "Error when recvData (image): " << comm.getErrorCode() << ", " << comm.getErrorString()
                     << endl;
                quit = true;
                break;
            } else if (!image.isImageDeserialized()) {
                cout << "Error when recvData (image): image is not deserialized!" << endl;
            } else {
                imshow("Received Image", image.getImage());
                cv::waitKey(2);
            }
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