//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 09.03.2021.
//

#ifndef COMM_DATA_MESSAGETYPE_H
#define COMM_DATA_MESSAGETYPE_H

#include <stdexcept>
#include <string>
#include <comm/socket/Socket.h>

namespace comm {
    enum MessageType {
        NOTHING = 0,
        STATUS,
        IMAGE,
        COORDINATE,
        BYTES,
        IMAGE_ENCODE,
    };

    MessageType stringToMessageType(const std::string &s);

    std::string messageTypeToString(const MessageType &messageType);
}

#endif //COMM_DATA_MESSAGETYPE_H
