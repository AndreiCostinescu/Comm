//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 24.03.2021.
//

#ifndef COMM_DATA_DATAUTILSWITHOPENCV_H
#define COMM_DATA_DATAUTILSWITHOPENCV_H

#include <comm/data/MessageType.h>
#include <comm/data/CommunicationData.h>

namespace comm {
    CommunicationData *createCommunicationDataPtrWithOpenCV(const MessageType &messageType);
}

#endif //COMM_DATA_DATAUTILSWITHOPENCV_H
