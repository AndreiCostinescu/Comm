//
// Created by Andrei on 01-Apr-21.
//

#include <comm/data/DataCollection.h>
#include <comm/data/dataUtils.h>
#include <comm/utils/utils.hpp>
#include <stdexcept>

using namespace comm;
using namespace std;

DataCollection::DataCollection() : data() {}

DataCollection::~DataCollection() {
    for (const auto &_data : this->data) {
        delete _data.second;
    }
    this->data.clear();
}

CommunicationData *DataCollection::get(const MessageType &messageType) {
    CommunicationData *commData;
    if (!mapGetIfContains(this->data, messageTypeToString(messageType), commData)) {
        commData = createCommunicationDataPtr(messageType);
        this->data[messageTypeToString(messageType)] = commData;
    }
    return commData;
}
