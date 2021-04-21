//
// Created by Andrei on 15-Apr-21.
//

#include <comm/socket/SocketType.h>
#include <stdexcept>

using namespace comm;
using namespace std;

SocketType comm::stringToSocketType(const string &s) {
    if (s == "UDP") {
        return SocketType::UDP;
    } else if (s == "UDP_HEADER") {
        return SocketType::UDP_HEADER;
    } else if (s == "TCP") {
        return SocketType::TCP;
    } else {
        throw runtime_error("Unknown SocketType " + s);
    }
}

string comm::socketTypeToString(SocketType socketType) {
    switch (socketType) {
        case UDP: {
            return "UDP";
        }
        case UDP_HEADER: {
            return "UDP_HEADER";
        }
        case TCP: {
            return "TCP";
        }
        default : {
            throw runtime_error("Unknown SocketType " + to_string(socketType));
        }
    }
}
