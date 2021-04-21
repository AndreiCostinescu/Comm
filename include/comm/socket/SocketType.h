//
// Created by Andrei on 15-Apr-21.
//

#ifndef PINAKOTHEKDRAWING_SOCKETTYPE_H
#define PINAKOTHEKDRAWING_SOCKETTYPE_H

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

#endif //PINAKOTHEKDRAWING_SOCKETTYPE_H
