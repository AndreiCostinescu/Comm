//
// Created by Andrei on 16.03.2021.
//

#include <comm/data/CommunicationData.h>

using namespace comm;

CommunicationData::CommunicationData() : serializeState(0), deserializeState(0) {}

CommunicationData::~CommunicationData() = default;

void CommunicationData::resetSerializeState() {
    this->serializeState = 0;
}

char *CommunicationData::getDeserializeBuffer() {
    return nullptr;
}

void CommunicationData::resetDeserializeState() {
    this->deserializeState = 0;
}
