//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 23.03.2021.
//

#include <comm/communication/Communication.h>
#include <comm/communication/TCPServer.h>
#include <comm/data/ImageData.h>
#include <comm/data/StatusData.h>
#include <comm/socket/utils.h>
#include <iostream>
#include <thread>

using namespace comm;
using namespace cv;
using namespace std;

int port = 8400;
bool verbose = false;

void sender(SocketType udpSocketType) {
    this_thread::sleep_for(std::chrono::seconds(1));

    Communication p;
    p.createSocket(SocketType::TCP, SocketPartner("127.0.0.1", port, false), -1, 2000, 500);
    cout << "Initialized sender tcp client!" << endl;
    p.createSocket(udpSocketType, nullptr, 0, 2000, 500);
    int udpPort = p.getMyself(udpSocketType)->getPort();
    cout << "Initialized sender udp server!" << endl;
    StatusData s;
    s.setData(to_string(udpPort).c_str());
    assert(p.transmitData(SocketType::TCP, &s, false, false));
    assert(p.recvData(udpSocketType, &s, false, false, false));
    assert(strcmp(s.getData(), "hello") == 0);

    Mat image = cv::imread("../../data/Lena.png");
    ImageData i(image, 1);
    if (!p.transmitData(udpSocketType, &i, false, false, 0, true)) {
        cout << "Error: " << p.getErrorCode() << "; " << p.getErrorString() << "; " << getLastErrorString() << endl;
    } else {
        cout << "Sent image data " << endl;
    }

    cout << "Sender finished normally" << endl;
}

void receiver(SocketType udpSocketType) {
    TCPServer server(port);
    // server.listen();
    Communication *comm;
    while (true) {
        comm = server.acceptCommunication();
        if (comm != nullptr) {
            cout << "Accepted connection" << endl;
            break;
        }
    }
    comm->setSocketTimeouts(SocketType::TCP, 2000, 500);

    StatusData s;
    while (!comm->recvData(SocketType::TCP, &s, false, false, false, 0, false)) {}
    assert(s.getData() != nullptr);
    SocketPartner p(comm->getPartner(SocketType::TCP)->getIP(), stoi(s.getData()), false);
    cout << "Connecting to udp: " << p.getPartnerString() << endl;
    comm->createSocket(udpSocketType, p, -1, 2000, 500);

    s.setData("hello");
    assert(comm->transmitData(udpSocketType, &s, false, false));

    this_thread::sleep_for(chrono::seconds(2));
    cout << "\n\n";

    ImageData i;
    if (!comm->recvData(udpSocketType, &i, false, false, false, 10, true)) {
        cout << "Error: " << comm->getErrorCode() << "; " << comm->getErrorString() << "; " << getLastErrorString()
             << endl;
    } else if (i.isImageDeserialized()) {
        if (!i.getImage().empty()) {
            imshow("Recv Image", i.getImage());
            cv::waitKey();
        } else {
            cout << "Error: received image is empty!" << endl;
        }
    } else {
        cout << "Error: deserializing image..." << endl;
    }

    cout << "Receiver finished normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t(sender, SocketType::UDP_HEADER);
    receiver(SocketType::UDP_HEADER);
    t.join();

    return 0;
}
