//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 24.03.2021.
//

#include <comm/data/dataUtilsWithOpenCV.h>
#include <comm/data/BytesData.h>
#include <comm/data/CoordinateData.h>
#include <comm/data/ImageDataWithOpenCV.h>
#include <comm/data/ImageEncodeDataWithOpenCV.h>
#include <comm/data/StatusData.h>
#include <stdexcept>

using namespace comm;
using namespace std;

CommunicationData *comm::createCommunicationDataPtrWithOpenCV(const MessageType &messageType) {
    switch (messageType) {
        case MessageType::NOTHING: {
            return nullptr;
        }
        case MessageType::COORDINATE: {
            return new CoordinateData();
        }
        case MessageType::IMAGE: {
            return new ImageDataWithOpenCV();
        }
        case MessageType::STATUS: {
            return new StatusData();
        }
        case MessageType::BYTES: {
            return new BytesData();
        }
        case MessageType::IMAGE_ENCODE: {
            return new ImageEncodeDataWithOpenCV();
        }
        default : {
            throw runtime_error("Unknown message type: " + to_string(messageType));
        }
    }
}
