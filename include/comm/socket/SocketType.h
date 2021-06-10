//
// Created by Andrei on 15-Apr-21.
//

#ifndef COMM_SOCKET_SOCKETTYPE_H
#define COMM_SOCKET_SOCKETTYPE_H

#include<string>

namespace comm {
    enum SocketType {
        UDP,
        UDP_HEADER,
        TCP
    };

    SocketType stringToSocketType(const std::string &s);

    std::string socketTypeToString(SocketType socketType);
}

#endif //COMM_SOCKET_SOCKETTYPE_H
