//
// Created by ga78cat on 16.03.2021.
//

#include <cassert>
#include <comm/communication/Communication.h>
#include <comm/data/StatusData.h>
#include <cstring>
#include <iostream>
#include <thread>

using namespace comm;
using namespace std;

bool verbose = false;
int port = 8400;

void server_1(SocketType udpSocketType) {
    string x = "Thread 2";
    Communication p;
    p.createSocket(udpSocketType, nullptr, port, 2000, 5000);
    cout << "Created partner communication..." << endl;
    StatusData status;

    while (true) {
        cout << x << ": In while loop, waiting for connection on " << p.getMyself(udpSocketType)->getStringAddress()
             << "..." << endl;
        assert (p.recvData(udpSocketType, &status, 0, verbose) || p.getErrorCode() == -1);
        if (p.getErrorCode() == -1) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }
        if (status.getData() != nullptr) {
            cout << x << ": Created partner communication with " << p.getPartnerString(udpSocketType) << endl;
            cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
            assert (strcmp(status.getData(), "Ce faci?") == 0);
            status.setData("Bine!");
            assert (p.sendData(udpSocketType, &status, false, -1, verbose));
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    cout << this_thread::get_id() << ": Ending server normally!" << endl;
}

void test_1(SocketType udpSocketType) {
    this_thread::sleep_for(std::chrono::seconds(2));

    string x = "Thread 1";
    StatusData status;
    Communication p;
    p.createSocket(udpSocketType, SocketPartner("127.0.0.1", port, false), 0, -1, 5000);
    cout << x << ": Created partner communication" << endl;
    status.setData("Ce faci?");
    cout << x << ": Created status data" << endl;
    assert (p.sendData(udpSocketType, &status, false, -1, verbose));
    cout << x << ": Sent status data" << endl;
    assert (p.recvData(udpSocketType, &status, -1, verbose));
    cout << x << ": Received status data" << endl;
    cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
    assert (strcmp(status.getData(), "Bine!") == 0);

    cout << this_thread::get_id() << ": Ending test_1 normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t(server_1, SocketType::UDP);
    test_1(SocketType::UDP);
    t.join();

    cout << "\n\n\n";
    this_thread::sleep_for(chrono::seconds(2));

    t = thread(server_1, SocketType::UDP_HEADER);
    test_1(SocketType::UDP_HEADER);
    t.join();

    return 0;
}

