//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 28.05.21.
//

#include <comm/communication/Communication.h>
#include <iostream>
#include <thread>

using namespace comm;
using namespace std;

int port = 8400;
bool quit = false;

void echo() {
    Communication c;
    c.createSocket(SocketType::UDP, nullptr, port);

    char *data = new char[65500];
    bool receivedData;
    while (!quit) {
        if (!c.receiveRaw(SocketType::UDP, data, receivedData)) {
            cout << "Error when receiving..." << endl;
        }
        if (receivedData) {
            cout << "UDP ECHO: " << data << endl;
        }
    }
    delete[] data;
    cout << "Finishing UDP RAW ECHO!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    thread t = thread(echo);
    Communication c;
    c.createSocket(SocketType::UDP, SocketPartner("127.0.0.1", port, false), 0, 2000);

    string x;
    char data[65500] = {0};
    while (!quit) {
        getline(cin, x);
        memcpy(data, x.c_str(), x.length());
        data[x.length()] = 0;
        if (!c.sendRaw(SocketType::UDP, x.c_str(), (int) x.length() + 1, 0, false)) {
            quit = true;
        }
        if (x == "q") {
            quit = true;
        }
    }
    t.join();

    return 0;
}