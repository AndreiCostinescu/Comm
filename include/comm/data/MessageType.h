//
// Created by ga78cat on 09.03.2021.
//

#ifndef PINAKOTHEKDRAWING_MESSAGETYPE_H
#define PINAKOTHEKDRAWING_MESSAGETYPE_H

#include <stdexcept>
#include <string>
#include <comm/socket/Socket.h>

namespace comm {
    enum MessageType {
        NOTHING = 0,
        STATUS,
        IMAGE,
        COORDINATE,
    };

    MessageType stringToMessageType(const std::string &s);

    std::string messageTypeToString(const MessageType &messageType);
}

#endif //PINAKOTHEKDRAWING_MESSAGETYPE_H
