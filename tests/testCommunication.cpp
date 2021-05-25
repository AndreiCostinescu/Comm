//
// Created by Andrei on 24.05.2021.
//

#include <cassert>
#include <comm/communication/Communication.h>
#include <comm/communication/TCPServer.h>
#include <comm/data/BytesData.h>
#include <comm/data/CoordinateData.h>
#include <comm/data/ImageData.h>
#include <comm/data/StatusData.h>
#include <comm/socket/utils.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

using namespace comm;
using namespace cv;
using namespace std;

int port = 8400, imageCap = 100, coordCap = 24, modVerbose = 20;
Mat *lena;

void receiveData(Communication &p, SocketType socketType, CommunicationData *data, bool withHeader,
                 bool withMessageType, MessageType expectedMessageType, int retries = 0, bool verbose = false) {
    if (withMessageType) {
        MessageType messageType;
        assert (p.recvMessageType(socketType, messageType, withHeader, retries, verbose));
        assert (messageType == expectedMessageType);
    }
    assert(p.recvData(socketType, data, withHeader, withMessageType, retries, verbose));
}

void sendCoordinate(Communication &p, SocketType socketType, CoordinateData &c, int cap, bool withHeader,
                    bool withMessageType, int incX, int incY, int retries = 0, bool verbose = false) {
    while (c.getID() <= cap) {
        c.setX(c.getX() + incX);
        c.setY(c.getY() + incY);
        assert (p.transmitData(socketType, &c, withHeader, withMessageType, retries, verbose));
        cout << "Sent message " << c.getID() << ": (" << c.getX() << ", " << c.getY() << ")" << endl;
        c.setID(c.getID() + 1);
        this_thread::sleep_for(chrono::milliseconds(20));
    }
}

void receiveCoordinate(Communication &p, SocketType socketType, CoordinateData &c, int cap, bool withHeader,
                       bool withMessageType, int retries = 0, bool verbose = false) {
    do {
        receiveData(p, socketType, &c, withHeader, withMessageType, MessageType::COORDINATE, retries, verbose);
        if (p.getErrorCode() == -1) {
            continue;
        }
        cout << "Received message " << c.getID() << ": (" << c.getX() << ", " << c.getY() << ")" << endl;
        this_thread::sleep_for(chrono::milliseconds(20));
    } while (c.getID() < cap);
}

void sendImage(Communication &p, SocketType socketType, ImageData &i, int cap, bool withHeader, bool withMessageType,
               int retries = 0, bool verbose = false) {
    if (socketType != SocketType::TCP) {
        return;
    }

    while (i.getID() <= cap) {
        assert (p.transmitData(socketType, &i, withHeader, withMessageType, retries, verbose));
        if (i.getID() % modVerbose == 0) {
            cout << "Sent message " << i.getID() << ": (" << i.getHeight() << "x" << i.getWidth() << ")" << endl;
        }
        i.setID(i.getID() + 1);
        this_thread::sleep_for(chrono::milliseconds(20));
    }
}

void receiveImage(Communication &p, SocketType socketType, ImageData &i, int cap, bool withHeader, bool withMessageType,
                  int retries = 0, bool verbose = false) {
    if (socketType != SocketType::TCP) {
        return;
    }

    do {
        receiveData(p, socketType, &i, withHeader, withMessageType, MessageType::IMAGE, retries, verbose);
        if (p.getErrorCode() == -1) {
            continue;
        }
        assert (i.isImageDeserialized());
        if (i.getID() % modVerbose == 0) {
            cout << "Received message " << i.getID() << ": (" << i.getHeight() << "x" << i.getWidth() << ")" << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(20));
    } while (i.getID() < cap);
}

void sender(SocketType socketType, bool withHeader, bool withMessageType) {
    this_thread::sleep_for(std::chrono::seconds(1));

    Communication p;
    p.createSocket(SocketType::TCP, SocketPartner("127.0.0.1", port, false), -1, 2000, 500);
    cout << "Initialized sender tcp client!" << endl;
    if (socketType != SocketType::TCP) {
        p.createSocket(socketType, nullptr, 0, 2000, 500);
        cout << "Initialized sender udp server!" << endl;
    }
    int commPort = p.getMyself(socketType)->getPort();

    StatusData s;
    s.setData(to_string(commPort).c_str());
    assert(p.transmitData(SocketType::TCP, &s, withHeader, withMessageType));

    receiveData(p, socketType, &s, withHeader, withMessageType, MessageType::STATUS);
    assert(strcmp(s.getData(), "hello") == 0);

    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    BytesData b;
    b.setChar(24, 0);
    b.setChar(5, 1);
    b.setShort(2021, 2);
    assert (p.transmitData(socketType, &b, withHeader, withMessageType, 0, false));
    cout << "Sent date bytes data\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    CoordinateData c;
    receiveCoordinate(p, socketType, c, coordCap, withHeader, withMessageType);

    c.setID(0);
    sendCoordinate(p, socketType, c, coordCap, withHeader, withMessageType, 0, 1);
    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    ImageData i;
    receiveImage(p, socketType, i, imageCap, withHeader, withMessageType);

    i.setID(i.getID() + 1);
    resize(*lena, *lena, Size(), 2, 2, INTER_CUBIC);
    i.setImage(*lena);
    sendImage(p, socketType, i, 2 * imageCap, withHeader, withMessageType);

    cout << "Sender finished normally" << endl;
}

void receiver(SocketType socketType, bool withHeader, bool withMessageType) {
    TCPServer server(port);
    Communication *comm, p;
    while (true) {
        comm = server.acceptCommunication();
        if (comm != nullptr) {
            cout << "Accepted connection" << endl;
            break;
        }
    }
    p = *comm;
    p.setSocketTimeouts(SocketType::TCP, 2000, 500);

    StatusData s;
    receiveData(p, SocketType::TCP, &s, withHeader, withMessageType, MessageType::STATUS);
    assert(s.getData() != nullptr);
    cout << "Received communication port: " << stoi(s.getData()) << endl;
    if (socketType != SocketType::TCP) {
        SocketPartner socketPartner(p.getPartner(SocketType::TCP)->getIP(), stoi(s.getData()), false);
        cout << "Connecting to " << socketPartner.getPartnerString() << endl;
        p.createSocket(socketType, socketPartner, -1, 2000, 500);
    }

    s.setData("hello");
    assert(p.transmitData(socketType, &s, withHeader, withMessageType));

    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    BytesData b;
    this_thread::sleep_for(chrono::milliseconds(20));
    receiveData(p, socketType, &b, withHeader, withMessageType, MessageType::BYTES, 0, false);
    cout << "Date: " << (int) b.getChar(0) << "." << (int) b.getChar(1) << "." << b.getShort(2) << "\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    CoordinateData c;
    c.setID(0);
    sendCoordinate(p, socketType, c, coordCap, withHeader, withMessageType, 1, 0);

    receiveCoordinate(p, socketType, c, coordCap, withHeader, withMessageType);
    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    ImageData i;
    i.setID(0);
    i.setImage(*lena);
    sendImage(p, socketType, i, imageCap, withHeader, withMessageType);

    receiveImage(p, socketType, i, 2 * imageCap, withHeader, withMessageType);
    cout << "Receiver finished normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    for (auto socketType : {SocketType::TCP, SocketType::UDP, SocketType::UDP_HEADER}) {
        for (auto withHeader: {false, true}) {
            for (auto withMessageType: {false, true}) {
                lena = new Mat(imread("../../data/Lena.png"));
                cout << "Starting test: " << socketType << ", " << withHeader << ", " << withMessageType << "\n\n";

                thread t(sender, socketType, withHeader, withMessageType);
                receiver(socketType, withHeader, withMessageType);
                t.join();

                cout << "Finishing test: " << socketType << ", " << withHeader << ", " << withMessageType << "\n\n\n\n";
                delete lena;
            }
        }
    }

    return 0;
}
