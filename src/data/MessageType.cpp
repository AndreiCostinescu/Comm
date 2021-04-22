//
// Created by ga78cat on 09.03.2021.
//

#include <comm/data/MessageType.h>
#include <comm/socket/utils.h>
#include <iostream>

using namespace comm;
using namespace std;

MessageType comm::stringToMessageType(const string &s) {
    if (s == "nothing") {
        return MessageType::NOTHING;
    } else if (s == "status") {
        return MessageType::STATUS;
    } else if (s == "image") {
        return MessageType::IMAGE;
    } else if (s == "coordinate") {
        return MessageType::COORDINATE;
    }
    throw runtime_error("Can not convert " + s + " to MessageType enum");
}

string comm::messageTypeToString(const MessageType &messageType) {
    switch (messageType) {
        case MessageType::NOTHING: {
            return "nothing";
        }
        case MessageType::STATUS: {
            return "status";
        }
        case MessageType::IMAGE: {
            return "image";
        }
        case MessageType::COORDINATE: {
            return "coordinate";
        }
        default: {
            throw runtime_error("Undefined MessageType: " + to_string(int(messageType)));
        }
    }
}