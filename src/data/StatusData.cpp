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

StatusData::StatusData() : data(nullptr), dataLength(0), dataSize(0), dataType(0) {}

StatusData::StatusData(int size) : StatusData() {
    prepareBuffer(this->data, this->dataLength, size);
}

StatusData::StatusData(const string &command) : StatusData() {
    this->setCommand(command);
}

MessageType StatusData::getMessageType() {
    return MessageType(this->getDataType());
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
    this->dataType = MessageType::NOTHING;
}

void StatusData::setCommand(const std::string &command) {
    const char *commandData;
    if (command == "_reset") {
        this->reset();
        return;
    } else if (command == "ping") {
        commandData = PING_MESSAGE;
        this->dataSize = PING_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "quit") {
        commandData = QUIT_MESSAGE;
        this->dataSize = QUIT_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "start") {
        commandData = START_MESSAGE;
        this->dataSize = START_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "stop") {
        commandData = STOP_MESSAGE;
        this->dataSize = STOP_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "wait") {
        commandData = WAIT_MESSAGE;
        this->dataSize = WAIT_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "accept") {
        commandData = ACCEPT_MESSAGE;
        this->dataSize = ACCEPT_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "ready") {
        commandData = READY_MESSAGE;
        this->dataSize = READY_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "control") {
        commandData = CONTROL_MESSAGE;
        this->dataSize = CONTROL_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "upload") {
        commandData = UPLOAD_MESSAGE;
        this->dataSize = UPLOAD_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "select") {
        commandData = SELECT_MESSAGE;
        this->dataSize = SELECT_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (command == "reject") {
        commandData = REJECT_MESSAGE;
        this->dataSize = REJECT_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    }
    prepareBuffer(this->data, this->dataLength, this->dataSize);
    memcpy(this->data, commandData, this->dataSize);
    assert(strcmp(commandData, this->data) == 0);
}

void StatusData::setStatus(const std::string &status) {
    const char *statusData;
    if (status == "_reset") {
        this->reset();
        return;
    } else if (status == "idle") {
        statusData = IDLE_MESSAGE;
        this->dataSize = IDLE_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (status == "active") {
        statusData = ACTIVE_MESSAGE;
        this->dataSize = ACTIVE_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    } else if (status == "done") {
        statusData = DONE_MESSAGE;
        this->dataSize = DONE_MESSAGE_LENGTH;
        this->dataType = MessageType::STATUS;
    }
    prepareBuffer(this->data, this->dataLength, this->dataSize);
    memcpy(this->data, statusData, this->dataSize);
    assert(strcmp(statusData, this->data) == 0);
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
    this->dataType = StatusData::statusDataType;
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

char StatusData::getDataType() const {
    return this->dataType;
}

const char StatusData::statusDataType = MessageType::STATUS;
