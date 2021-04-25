//
// Created by Andrei on 25-Apr-21.
//

#include <comm/data/BytesData.h>
#include <cassert>

using namespace comm;
using namespace std;

const int BytesData::headerSize = sizeof(int);

BytesData::BytesData(int size) : data(size), expectedDataSize(0) {}

BytesData::~BytesData() {
    this->data.reset();
}

MessageType BytesData::getMessageType() {
    return MessageType::BYTES;
}

bool BytesData::serialize(Buffer *buffer, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            // header
            buffer->setBufferContentSize(BytesData::headerSize);
            buffer->setInt((int) this->data.getBufferContentSize(), 0);
            if (verbose) {
                char *dataBuffer = buffer->getBuffer();
                cout << "buffer int content: " << (int) dataBuffer[0] << " " << (int) dataBuffer[1] << " "
                     << (int) dataBuffer[2] << " " << (int) dataBuffer[3] << endl;
            }
            this->serializeState = 1;
            return false;
        }
        case 1: {
            // data
            buffer->setReferenceToData(this->data.getBuffer(), this->data.getBufferContentSize());
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

int BytesData::getExpectedDataSize() const {
    switch (this->deserializeState) {
        case 0: {
            return BytesData::headerSize;
        }
        case 1: {
            return this->expectedDataSize;
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

bool BytesData::deserialize(Buffer *buffer, int start, bool verbose) {
    switch (this->deserializeState) {
        case 0: {
            this->expectedDataSize = buffer->getInt(start);
            this->deserializeState = 1;
            return false;
        }
        case 1: {
            assert (buffer->getBufferContentSize() == this->expectedDataSize);
            this->data.setData(buffer->getBuffer(), this->expectedDataSize);
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

void BytesData::reset() {
    this->data.reset();
    this->expectedDataSize = 0;
}

void BytesData::setData(char *_data, unsigned int dataSize, int start) {
    this->data.setData((const char *) _data, dataSize, start);
}

void BytesData::setData(const char *_data, unsigned int dataSize, int start) {
    this->data.setData(_data, dataSize, start);
}

void BytesData::setReferenceToData(char *_data, unsigned int dataSize) {
    this->data.setReferenceToData(_data, dataSize);
}

void BytesData::setChar(char _data, int position) {
    this->data.setChar(_data, position);
}

void BytesData::setShort(short _data, int position) {
    this->data.setShort(_data, position);
}

void BytesData::setInt(int _data, int position) {
    this->data.setInt(_data, position);
}

void BytesData::setLongLong(long long _data, int position) {
    this->data.setLongLong(_data, position);
}

void BytesData::setFloat(float _data, int position) {
    this->data.setFloat(_data, position);
}

void BytesData::setDouble(double _data, int position) {
    this->data.setDouble(_data, position);
}

char BytesData::getChar(int position) {
    return this->data.getChar(position);
}

short BytesData::getShort(int position) {
    return this->data.getShort(position);
}

int BytesData::getInt(int position) {
    return this->data.getInt(position);
}

long long BytesData::getLongLong(int position) {
    return this->data.getLongLong(position);
}

float BytesData::getFloat(int position) {
    return this->data.getFloat(position);
}

double BytesData::getDouble(int position) {
    return this->data.getDouble(position);
}

bool BytesData::empty() const {
    return this->data.empty();
}

char *BytesData::getBuffer() {
    return this->data.getBuffer();
}

unsigned long long int BytesData::getBufferSize() const {
    unsigned long long int size = this->data.getBufferContentSize();
    assert (this->deserializeState != 1 || size == this->expectedDataSize);
    return size;
}
