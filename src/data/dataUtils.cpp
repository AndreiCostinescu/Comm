//
// Created by ga78cat on 24.03.2021.
//

#include <comm/data/dataUtils.h>
#include <comm/data/CoordinateData.h>
#include <comm/data/ImageData.h>
#include <comm/data/StatusData.h>
#include <stdexcept>

using namespace comm;
using namespace std;

CommunicationData *comm::createCommunicationDataPtr(const MessageType &messageType) {
    switch (messageType) {
        case MessageType::NOTHING: {
            return nullptr;
        }
        case MessageType::COORDINATE: {
            return new CoordinateData();
        }
        case MessageType::IMAGE: {
            return new ImageData();
        }
        case MessageType::STATUS: {
            return new StatusData();
        }
        default : {
            throw runtime_error("Unknown message type: " + to_string(messageType));
        }
    }
}
