//
// Created by Andrei on 24.03.2021.
//

#ifndef COMM_DATA_DATAUTILS_H
#define COMM_DATA_DATAUTILS_H

#include <comm/data/MessageType.h>
#include <comm/data/CommunicationData.h>

namespace comm {
    CommunicationData *createCommunicationDataPtr(const MessageType &messageType);
}

#endif //COMM_DATA_DATAUTILS_H
