//
// Created by Andrei on 03.06.2021.
//

#include <cassert>
#include <comm/communication/Communication.h>
#include <comm/data/StatusData.h>
#include <cstring>
#include <iostream>
#include "json.hpp"
#include <thread>

using namespace comm;
using namespace nlohmann;
using namespace std;

bool verbose = false;
int port = 8400;

void server(SocketType udpSocketType) {
    string x = "Thread 2";
    Communication p;
    p.createSocket(udpSocketType, nullptr, port, 2000, 5000);
    cout << "Created partner communication..." << endl;
    StatusData status;

    while (true) {
        cout << x << ": In while loop, waiting for connection on " << p.getMyself(udpSocketType)->getStringAddress()
             << "..." << endl;
        assert (p.recvData(udpSocketType, &status, false, false, false, 0, verbose) || p.getErrorCode() == -1);
        if (p.getErrorCode() == -1) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }
        if (status.getData() != nullptr) {
            cout << x << ": Created partner communication with " << p.getPartnerString(udpSocketType) << endl;
            cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
            json recvData = json::parse(string(status.getData()));
            assert (recvData["request"] == "telemetry");
            assert (recvData["q"] == nullptr);
            assert (recvData["robot_state"] == 42);
            assert (recvData["O_T_EE"].size() == 16);
            status.setData("Received data!");
            assert (p.transmitData(udpSocketType, &status, false, false, -1, verbose));
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    cout << this_thread::get_id() << ": Ending server normally!" << endl;
}

void test(SocketType udpSocketType) {
    this_thread::sleep_for(std::chrono::seconds(2));

    string x = "Thread 1";
    StatusData status;
    Communication p;
    p.createSocket(udpSocketType, SocketPartner("127.0.0.1", port, false), 0, -1, 5000);
    cout << x << ": Created partner communication" << endl;

    json data;
    data["request"] = "telemetry";
    data["q"] = nullptr;
    data["robot_state"] = 42;
    data["O_T_EE"] = {1.0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 2, 3, 1};

    status.setData(data.dump());
    cout << x << ": Created status data" << endl;
    assert (p.transmitData(udpSocketType, &status, false, false, -1, verbose));
    cout << x << ": Sent status data" << endl;
    assert (p.recvData(udpSocketType, &status, false, false, false, -1, verbose));
    cout << x << ": Received status data" << endl;
    cout << x << ": Received: \"" << status.getData() << "\" from partner" << endl;
    assert (strcmp(status.getData(), "Received data!") == 0);

    cout << this_thread::get_id() << ": Ending test_1 normally!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    auto x = nlohmann::json();
    cout << x << endl;

    x = nlohmann::json::parse("{}");
    cout << x << endl;

    thread t(server, SocketType::UDP);
    test(SocketType::UDP);
    t.join();

    cout << "\n\n\n";
    this_thread::sleep_for(chrono::seconds(2));

    t = thread(server, SocketType::UDP_HEADER);
    test(SocketType::UDP_HEADER);
    t.join();

    return 0;
}

