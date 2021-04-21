//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_DATAUTILS_H
#define PINAKOTHEKDRAWING_DATAUTILS_H

#include <comm/data/MessageType.h>
#include <comm/data/CommunicationData.h>

namespace comm {
    CommunicationData *createCommunicationDataPtr(const MessageType &messageType);
}

#endif //PINAKOTHEKDRAWING_DATAUTILS_H
