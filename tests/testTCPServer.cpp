//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 15.03.2021.
//

#include <cassert>
#include <comm/communication/TCPServer.h>
#include <comm/data/StatusData.h>
#include <cstring>
#include <iostream>
#include <thread>

using namespace comm;
using namespace std;

int port = 8400;

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
            assert (p->recvData(SocketType::TCP, &status, false, false, false, -1, true));
            cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
            assert (strcmp(status.getData(), "Ce faci?") == 0);
            status.setData("Bine!");
            assert (p->transmitData(SocketType::TCP, &status, false, false, -1, true));
            break;
        }
    }

    delete p;
}

void test_1() {
    this_thread::sleep_for(std::chrono::seconds(7));

    string x = "Thread 1";
    StatusData status;
    Communication p;
    p.createSocket(SocketType::TCP, SocketPartner("127.0.0.1", port, false));
    cout << x << ": Created partner communication" << endl;
    status.setData("Ce faci?");
    cout << x << ": Created status data" << endl;
    assert(p.transmitData(SocketType::TCP, &status, false, false, -1, true));
    cout << x << ": Sent status data" << endl;
    assert(p.recvData(SocketType::TCP, &status, false, false, false, -1, true));
    cout << x << ": Received status data" << endl;
    cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
    assert (strcmp(status.getData(), "Bine!") == 0);
}

int main() {
    cout << "Hello World!" << endl;

    thread t(server_1);
    test_1();
    t.join();

    return 0;
}

