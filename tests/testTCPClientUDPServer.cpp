//
// Created by ga78cat on 18.03.2021.
//

#include <cassert>
#include <comm/communication/TCPServer.h>
#include <comm/data/StatusData.h>
#include <comm/socket/utils.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "string_utils.h"

using namespace comm;
using namespace std;

int tcpPort = 8000;

void tcpServer() {
    TCPServer server(tcpPort);
    // server.listen();
    Communication *partner;
    while (true) {
        partner = server.acceptCommunication();
        if (partner != nullptr) {
            cout << "Accepted connection" << endl;
            break;
        }
    }

    StatusData s;
    while (!partner->recvData(SocketType::TCP, &s, 0, false)) {}
    assert(s.getData() != nullptr);
    int udpPort = stoi(s.getData());
    cout << "Received udp port to communicate on: " << udpPort << endl;
    string udpIP = partner->getPartner(SocketType::TCP)->getIP();
    assert(udpIP == splitString(partner->getPartnerString(SocketType::TCP), ":")[0]);
    SocketPartner p(udpIP, udpPort, false);
    partner->createSocket(SocketType::UDP, p, -1);
    cout << "UDP Client initialized" << endl;
    cout << "Client: UDP sendTo: " << partner->getPartnerString(SocketType::UDP) << endl;
    cout << "Client: UDP myself: " << partner->getMyAddressString(SocketType::UDP) << endl;
    s.setData("Hello!");
    partner->transmitData(SocketType::UDP, &s, false, false, 0, false);
    cout << "Client: UDP sendTo: " << partner->getPartnerString(SocketType::UDP) << endl;
    cout << "Client: UDP myself: " << partner->getMyAddressString(SocketType::UDP) << endl;
    s.setData(partner->getMyAddressString(SocketType::UDP));
    partner->transmitData(SocketType::UDP, &s, false, false, 0, false);
    cout << "Sent my address" << endl;
    cout << "UDP Server finished normally" << endl;
}

void tcpClient() {
    this_thread::sleep_for(std::chrono::seconds(1));

    Communication p;
    p.createSocket(SocketType::TCP, SocketPartner("127.0.0.1", tcpPort, false));
    p.createSocket(SocketType::UDP, nullptr, 0);
    int udpPort = p.getMyself(SocketType::UDP)->getPort();
    cout << "Initialized client!" << endl;
    StatusData s;
    s.setData(to_string(udpPort).c_str());
    assert(p.transmitData(SocketType::TCP, &s, false, false));
    cout << "Sent udp port: " << udpPort << endl;
    this_thread::sleep_for(std::chrono::seconds(2));

    cout << endl << endl;
    cout << "Server: UDP sendTo: " << p.getPartnerString(SocketType::UDP) << endl;
    cout << "Server: UDP myself: " << p.getMyAddressString(SocketType::UDP) << endl;
    p.recvData(SocketType::UDP, &s, 0, false);
    assert(strcmp(s.getData(), "Hello!") == 0);
    cout << "Got data: " << s.getData() << "; expected: \"Hello!\"" << endl;
    cout << "Server: UDP sendTo: " << p.getPartnerString(SocketType::UDP) << endl;
    cout << "Server: UDP myself: " << p.getMyAddressString(SocketType::UDP) << endl;
    p.recvData(SocketType::UDP, &s, 0, false);
    cout << "Got data: " << s.getData() << "; expected: " << p.getPartnerString(SocketType::UDP) << endl;
    // Only compare the ports!
    assert(splitString(s.getData(), ":")[1] == splitString(p.getPartnerString(SocketType::UDP), ":")[1]);
    cout << "Receive start message" << endl;

    cout << "Test finished normally" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t(tcpServer);
    tcpClient();
    t.join();

    return 0;
}
