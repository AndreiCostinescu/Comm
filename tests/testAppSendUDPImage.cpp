//
// Created by Andrei on 10-Apr-21.
//

#include <comm/communication/Communication.h>
#include <comm/communication/TCPServer.h>
#include <comm/data/ImageData.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <comm/data/messages.h>

using namespace comm;
using namespace cv;
using namespace std;

int port = 8400;
bool quit = false;
// SocketType socketType = SocketType::UDP;
SocketType socketType = SocketType::TCP;

void createUDPServer() {
    TCPServer s(port);
    Communication comm, *sendComm = nullptr;
    comm.createSocket(SocketType::UDP, SocketPartner(true, false), port, 2000, 50);

    Mat lena = cv::imread("../../data/Lena.png");
    ImageData image;
    StatusData status;
    MessageType messageType;
    while (!quit) {
        if (!comm.recvMessageType(SocketType::UDP, &messageType)) {
            cout << "Error when recvMessageType: " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
            quit = true;
            break;
        }
        if (messageType == MessageType::STATUS) {
            cout << "Connection from " << comm.getPartner(SocketType::UDP)->getStringAddress() << endl;
            comm.setOverwritePartner(SocketType::UDP, false);
            if (!comm.recvData(SocketType::UDP, &status, -1, true)) {
                cout << "Error when recvData (status): " << comm.getErrorCode() << ", " << comm.getErrorString()
                     << endl;
                quit = true;
                break;
            }
            if (strcmp(status.getData(), "TCP") == 0) {
                socketType = SocketType::TCP;
            } else if (strcmp(status.getData(), "UDP") == 0) {
                socketType = SocketType::UDP;
            } else {
                cout << "status data is not the expected TCP or UDP but " << status.getData() << endl;
                break;
            }
            sendComm = nullptr;
            if (socketType == SocketType::TCP) {
                while (sendComm == nullptr) {
                    sendComm = s.acceptCommunication();
                }
            } else {
                sendComm = &comm;
            }
            Mat toSendImage;
            lena.copyTo(toSendImage);
            image.setImage(toSendImage);
            image.setID(1);
            while (image.getID() <= 10) {
                this_thread::sleep_for(chrono::milliseconds(1000));
                if (!sendComm->sendData(socketType, &image, true)) {
                    cout << "Error when sendData (image): " << comm.getErrorCode() << ", " << comm.getErrorString()
                         << endl;
                    break;
                }
                cout << "Sent image: " << image.getID() << endl;
                image.setID(image.getID() + 1);
                toSendImage += 10;
                image.setImage(toSendImage);
            }
            // at the end
            if (socketType == SocketType::TCP) {
                sendComm->cleanup();
            }
            comm.setOverwritePartner(SocketType::UDP, true);
        }
    }
}

void createUDPClient() {
    this_thread::sleep_for(chrono::seconds(1));
    Communication comm;
    comm.createSocket(SocketType::UDP, SocketPartner("127.0.0.1", port, false), 0, 2000, 50);

    ImageData image;
    StatusData status("start");
    MessageType messageType;

    if (!comm.sendData(SocketType::UDP, &status, true)) {
        cout << "Error when sendData (status): " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
        quit = true;
        return;
    }
    if (!comm.recvMessageType(SocketType::UDP, &messageType)) {
        cout << "Error when recvMessageType: " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
        quit = true;
        return;
    }
    if (messageType != MessageType::IMAGE) {
        cout << "Expected image but received " << messageTypeToString(messageType) << endl;
        return;
    }
    if (!comm.recvData(SocketType::UDP, &image)) {
        cout << "Error when recvData (status): " << comm.getErrorCode() << ", " << comm.getErrorString() << endl;
        quit = true;
        return;
    } else if (!image.isImageDeserialized()) {
        cout << "Image is not completely deserialized..." << endl;
        return;
    } else if (image.getImage().empty()) {
        cout << "Received image is empty!" << endl;
        return;
    } else {
        cv::imshow("Received UDP image", image.getImage());
        cv::waitKey(0);
        destroyWindow("Received UDP image");
    }
}

int main() {
    cout << "Hello World!" << endl;

    thread server(createUDPServer);

    string x;
    while (true) {
        cin >> x;
        if (x == "q") {
            quit = true;
            break;
        } else if (x == "s") {
            thread client(createUDPClient);
            client.join();
        }
    }
    server.join();

    return 0;
}