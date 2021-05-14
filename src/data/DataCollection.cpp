//
// Created by Andrei on 01-Apr-21.
//

#include <comm/data/DataCollection.h>
#include <comm/data/dataUtils.h>
#include <comm/utils/utils.hpp>
#include <stdexcept>

using namespace comm;
using namespace std;

DataCollection::DataCollection() : data(), dataKeys() {}

DataCollection::~DataCollection() {
    cout << "In DataCollection destructor: " << endl;
    this->reset();
}

void DataCollection::reset() {
    for (const auto &_data : this->data) {
        cout << "\tDelete " << _data.first << ": " << _data.second << endl;
        delete _data.second;
    }
    this->data.clear();
    this->dataKeys.clear();
}

void DataCollection::set(const MessageType &messageType, CommunicationData *commData) {
    string stringMessageType = messageTypeToString(messageType);
    if (!mapContains(this->data, stringMessageType)) {
        this->dataKeys.push_back(stringMessageType);
    }
    this->data[stringMessageType] = commData;
}

CommunicationData *DataCollection::get(const MessageType &messageType) {
    CommunicationData *commData;
    string stringMessageType = messageTypeToString(messageType);
    if (!mapGetIfContains(this->data, stringMessageType, commData)) {
        for (const auto &dataKey : this->dataKeys) {
            if (dataKey == stringMessageType) {
                (*cerror) << "The data key assertion will fail for key = " << stringMessageType << endl;
            }
            assert (dataKey != stringMessageType);
        }
        commData = createCommunicationDataPtr(messageType);
        this->data[stringMessageType] = commData;
        this->dataKeys.push_back(stringMessageType);
    }
    return commData;
}
