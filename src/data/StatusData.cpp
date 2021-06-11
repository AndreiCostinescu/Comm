//
// Created by ga78cat on 11.03.2021.
//

#include <comm/data/StatusData.h>
#include <cassert>
#include <comm/socket/utils.h>
#include <cstring>
#include <comm/data/messages.h>
#include <comm/data/MessageType.h>
#include <iostream>

using namespace comm;
using namespace std;

const int StatusData::headerSize = sizeof(int);

StatusData::StatusData() : data(nullptr), dataLength(0), dataSize(0) {}

StatusData::StatusData(int size) : StatusData() {
    prepareBuffer(this->data, this->dataLength, size);
}

StatusData::StatusData(const char *data) : StatusData() {
    this->setData(data);
}

MessageType StatusData::getMessageType() {
    return MessageType::STATUS;
}

bool StatusData::serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(start + StatusData::headerSize);
            buffer->setInt(this->dataSize, start);
            if (verbose) {
                const char *dataBuffer = buffer->getBuffer();
                cout << "buffer int content: " << (int) dataBuffer[start] << " " << (int) dataBuffer[start + 1] << " "
                     << (int) dataBuffer[start + 2] << " " << (int) dataBuffer[start + 3] << endl;
            }
            this->serializeState = 1;
            return false;
        }
        case 1: {
            if (forceCopy) {
                buffer->setData(this->data, this->dataSize, start);
            } else {
                if (start != 0) {
                    throw runtime_error("Can not set a reference to data not starting at the first position!");
                }
                buffer->setConstReferenceToData(this->data, this->dataSize);
            }
            this->serializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible serialize state... " << this->serializeState << endl;
            this->serializeState = 0;
            return false;
        }
    }
}

int StatusData::getExpectedDataSize() const {
    switch (this->deserializeState) {
        case 0: {
            return StatusData::headerSize;
        }
        case 1: {
            return this->dataSize;
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

bool StatusData::deserialize(Buffer *buffer, int start, bool, bool verbose) {
    switch (this->deserializeState) {
        case 0: {
            this->dataSize = buffer->getInt(start);
            this->deserializeState = 1;
            return false;
        }
        case 1: {
            assert ((buffer->getBufferContentSize() - start) == this->dataSize);
            this->setData(buffer->getBuffer() + start, this->dataSize);
            this->deserializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible deserialize state... " << this->deserializeState << endl;
            this->resetDeserializeState();
            return false;
        }
    }
}

void StatusData::reset() {
    this->dataSize = -1;
}

void StatusData::setData(char *_data) {
    this->setData((const char *) _data, (int) strlen(_data) + 1);
}

void StatusData::setData(char *_data, int _dataSize) {
    this->setData((const char *) _data, _dataSize);
}

void StatusData::setData(const char *_data) {
    this->setData(_data, (int) strlen(_data) + 1);
}

void StatusData::setData(const char *_data, int _dataSize) {
    if (_data == nullptr) {
        this->dataSize = -1;
        return;
    }
    prepareBuffer(this->data, this->dataLength, _dataSize);
    memcpy(this->data, _data, _dataSize);
    this->dataSize = _dataSize;
}

void StatusData::setData(const string &_data) {
    this->setData(_data.c_str(), (int) _data.size() + 1);
}

void StatusData::setData(const string &_data, int _dataSize) {
    this->setData(_data.c_str(), _dataSize);
}

char *StatusData::getData() const {
    if (this->dataSize < 0) {
        return nullptr;
    }
    return this->data;
}

int StatusData::getDataSize() const {
    return this->dataSize;
}
