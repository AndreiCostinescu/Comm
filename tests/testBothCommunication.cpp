//
// Created by ga78cat on 16.03.2021.
//

#include <cassert>
#include <cstring>
#include <comm/communication/TCPServer.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <thread>
#include "string_utils.h"

using namespace comm;
using namespace std;

int port = 8401;

void server_1() {
    string x = "Thread 2";
    Communication *p;
    StatusData status;
    TCPServer t(port);
    cout << x << ": Created tcp server" << endl;

    // t.listen();
    cout << x << ": listening for connections..." << endl;

    while (true) {
        cout << x << ": In while loop, waiting for connection..." << endl;
        p = t.acceptCommunication();
        if (p != nullptr) {
            cout << x << ": Created partner communication with " << p->getPartnerString(SocketType::TCP) << endl;
            p->setSocketTimeouts(SocketType::TCP, 2000, 50);
            break;
        }
    }

    cout << x << ": Starting receive tcp status data..." << endl;
    assert(p->recvData(SocketType::TCP, &status, false, false, -1, true));
    cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;

    status.setData("start");
    cout << x << ": Starting start status data..." << endl;
    assert(p->transmitData(SocketType::TCP, &status, false, false, -1, true));

    this_thread::sleep_for(std::chrono::seconds(3));

    p->createSocket(SocketType::UDP, nullptr, port, 2000, 50);
    const char *message;
    while (true) {
        assert(p->recvData(SocketType::UDP, &status, false, false) || p->getErrorCode() == -1);
        if (p->getErrorCode() == -1) {
            continue;
        }
        message = status.getData();
        if (message != nullptr) {
            cout << x << ": Received on UDP \"" << message << "\"" << endl;
            auto split = splitString(message, " ");
            if (split.size() == 2 && stoi(split[1]) >= 96) {
                break;
            }
        }
    }

    status.setData("stop");
    assert(p->transmitData(SocketType::TCP, &status, false, false));

    delete p;

    cout << x << ": server finished normally!" << endl;
}

void test_1() {
    this_thread::sleep_for(std::chrono::seconds(2));

    string x = "Thread 1";
    StatusData tcpStatus, udpStatus;
    SocketPartner partner("127.0.0.1", port, false);
    Communication p;

    p.createSocket(SocketType::TCP, partner, -1, 2000, 50);
    cout << x << ": Created TCP partner communication" << endl;
    p.createSocket(SocketType::UDP, partner, -1, 2000, 50);
    cout << x << ": Created TCP & UDP partner communication" << endl;

    tcpStatus.setData("Robot 1");
    cout << x << ": Created tcpStatus data" << endl;
    assert(p.transmitData(SocketType::TCP, &tcpStatus, false, false, -1, true));
    cout << x << ": Sent tcpStatus data" << endl;
    assert(p.recvData(SocketType::TCP, &tcpStatus, false, false, -1, true));
    cout << x << ": Received tcpStatus data \"" << tcpStatus.getData() << "\" from partner" << endl;
    assert(strcmp(tcpStatus.getData(), "start") == 0);

    this_thread::sleep_for(std::chrono::seconds(3));

    int id = 0;
    const char *message;
    while (true) {
        udpStatus.setData("Message " + to_string(id++));
        // cout << x << ": set data for udp transmission" << endl;
        assert(p.transmitData(SocketType::UDP, &udpStatus, false, false));
        cout << x << ": sent udp data: " << udpStatus.getData() << endl;
        assert(p.recvData(SocketType::TCP, &tcpStatus, false, false, 0) || p.getErrorCode() == -1);
        // cout << x << ": after tcp listen/recv" << endl;
        if (p.getErrorCode() == -1) {
            continue;
        }
        message = tcpStatus.getData();
        if (message != nullptr) {
            cout << x << ": received \"" << message << "\" from server" << endl;
            if (strcmp(message, "stop") == 0) {
                break;
            }
        }
    }

    cout << x << ": test_1 finished normally!" << endl;
}

void server_2() {
    string x = "Thread 2";
    Communication *p;
    StatusData status;
    TCPServer t(port);
    cout << x << ": Created tcp server" << endl;

    // t.listen();
    cout << x << ": listening for connections..." << endl;

    while (true) {
        cout << x << ": In while loop, waiting for connection..." << endl;
        p = t.acceptCommunication();
        if (p != nullptr) {
            cout << x << ": Created partner communication with " << p->getPartnerString(SocketType::TCP) << endl;
            p->setSocketTimeouts(SocketType::TCP, 2000, 50);
            break;
        }
    }

    status.setData("start");
    cout << x << ": Starting start status data..." << endl;
    assert(p->transmitData(SocketType::TCP, &status, false, false, -1, true));

    cout << x << ": Starting receive tcp status data..." << endl;
    assert(p->recvData(SocketType::TCP, &status, false, false, -1, true));
    cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;

    delete p;

    cout << x << ": server finished normally!" << endl;
}

void test_2() {
    this_thread::sleep_for(std::chrono::seconds(2));

    string x = "Thread 1";
    StatusData tcpStatus, udpStatus;
    SocketPartner partner("127.0.0.1", port, false);
    Communication p;

    p.createSocket(SocketType::TCP, partner, -1, 2000, 50);
    cout << x << ": Created TCP partner communication" << endl;

    assert(p.recvData(SocketType::TCP, &tcpStatus, false, false, -1, true));
    cout << x << ": Received tcpStatus data \"" << tcpStatus.getData() << "\" from partner" << endl;

    tcpStatus.setData("Robot 1");
    cout << x << ": Created tcpStatus data" << endl;
    assert(p.transmitData(SocketType::TCP, &tcpStatus, false, false, -1, true));
    cout << x << ": Sent tcpStatus data" << endl;

    cout << x << ": test_2 finished normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t(server_2);
    test_2();
    t.join();

    this_thread::sleep_for(chrono::seconds(2));

    t = thread(server_1);
    test_1();
    t.join();

    return 0;
}
