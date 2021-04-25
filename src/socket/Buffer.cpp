//
// Created by ga78cat on 24.03.2021.
//

#include <comm/socket/Buffer.h>
#include <cassert>
#include <comm/socket/utils.h>
#include <cstring>
#include <iostream>

using namespace comm;
using namespace std;

Buffer::Buffer(unsigned long long int bufferSize) :
        buffer(nullptr), bufferSize(0), bufferContentSize(0), referenceBuffer(nullptr), useReferenceBuffer(false) {
    this->prepareBuffer(bufferSize);
}

Buffer::~Buffer() {
    this->reset();
}

void Buffer::reset() {
    delete[] this->buffer;
    this->buffer = nullptr;
    this->bufferSize = 0;
    this->bufferContentSize = 0;
    this->referenceBuffer = nullptr;
    this->useReferenceBuffer = false;
}

Buffer *Buffer::copy() const {
    auto *copy = new Buffer(this->bufferSize);

    assert (copy->bufferSize == this->bufferSize);
    memcpy(copy->buffer, this->buffer, this->bufferSize);

    copy->bufferContentSize = this->bufferContentSize;

    copy->referenceBuffer = this->referenceBuffer;
    copy->useReferenceBuffer = this->useReferenceBuffer;

    return copy;
}

void Buffer::setData(char *data, unsigned int dataSize, int start) {
    this->setData((const char *) data, dataSize, start);
}

void Buffer::setData(const char *data, unsigned int dataSize, int start) {
    this->setBufferContentSize(dataSize + start);
    memcpy(this->buffer + start, data, dataSize);
    this->useReferenceBuffer = false;
}

void Buffer::setReferenceToData(char *data, unsigned int dataSize) {
    this->referenceBuffer = data;
    this->bufferContentSize = dataSize;
    this->useReferenceBuffer = true;
}

void Buffer::setChar(char data, int position) {
    this->checkBufferContentSize(position + sizeof(char), true);
    this->buffer[position] = data;
    this->useReferenceBuffer = false;
}

void Buffer::setShort(short data, int position) {
    this->checkBufferContentSize(position + sizeof(short), true);
    shortToNetworkBytes(this->buffer, position, data);
    this->useReferenceBuffer = false;
}

void Buffer::setInt(int data, int position) {
    this->checkBufferContentSize(position + sizeof(int), true);
    intToNetworkBytes(this->buffer, position, data);
    this->useReferenceBuffer = false;
}

void Buffer::setLongLong(long long data, int position) {
    this->checkBufferContentSize(position + sizeof(long long), true);
    longLongToNetworkBytes(this->buffer, position, data);
    this->useReferenceBuffer = false;
}

void Buffer::setFloat(float data, int position) {
    this->checkBufferContentSize(position + sizeof(float), true);
    floatToNetworkBytes(this->buffer, position, data);
    this->useReferenceBuffer = false;
}

void Buffer::setDouble(double data, int position) {
    this->checkBufferContentSize(position + sizeof(double), true);
    doubleToNetworkBytes(this->buffer, position, data);
    this->useReferenceBuffer = false;
}

char Buffer::getChar(int position) {
    this->checkBufferContentSize(position + sizeof(char), false);
    return this->buffer[position];
}

short Buffer::getShort(int position) {
    this->checkBufferContentSize(position + sizeof(short), false);
    return networkBytesToShort(this->buffer, position);
}

int Buffer::getInt(int position) {
    this->checkBufferContentSize(position + sizeof(int), false);
    return networkBytesToInt(this->buffer, position);
}

long long Buffer::getLongLong(int position) {
    this->checkBufferContentSize(position + sizeof(long long), false);
    return networkBytesToLongLong(this->buffer, position);
}

float Buffer::getFloat(int position) {
    this->checkBufferContentSize(position + sizeof(float), false);
    return networkBytesToFloat(this->buffer, position);
}

double Buffer::getDouble(int position) {
    this->checkBufferContentSize(position + sizeof(double), false);
    return networkBytesToDouble(this->buffer, position);
}

bool Buffer::empty() const {
    return this->bufferContentSize == 0;
}

char *Buffer::getBuffer() {
    char *result = (this->useReferenceBuffer) ? this->referenceBuffer : this->buffer;
    this->useReferenceBuffer = false;
    return result;
}

unsigned long long int Buffer::getBufferSize() const {
    return this->bufferSize;
}

unsigned long long int Buffer::getBufferContentSize() const {
    return this->bufferContentSize;
}

void Buffer::setBufferContentSize(unsigned long long int _bufferContentSize) {
    this->prepareBuffer(_bufferContentSize);
    this->bufferContentSize = _bufferContentSize;
}

void Buffer::prepareBuffer(unsigned long long int desiredSize) {
    if (this->bufferSize < desiredSize) {
        char *oldBuffer = this->buffer;
        unsigned long long int oldSize = this->bufferSize;

        // cout << "Initialize new buffer!" << endl;
        this->buffer = new char[desiredSize];
        this->bufferSize = desiredSize;
        // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;

        if (oldSize > 0) {
            memcpy(this->buffer, oldBuffer, oldSize * sizeof(*(this->buffer)));
        }
        memset(this->buffer + oldSize, 0, (this->bufferSize - oldSize) * sizeof(*(this->buffer)));

        delete[] oldBuffer;
    }
}

void Buffer::checkBufferContentSize(unsigned long long size, bool modifySize) {
    if (size > this->bufferContentSize) {
        if (modifySize) {
            this->prepareBuffer(size);
            this->bufferContentSize = size;
        } else {
            (*cerror) << "Size of " << size << " is greater than the current buffer content size " << this->bufferContentSize
                 << "! The next assertion will fail..." << endl;
        }
    }
    if (size > this->bufferContentSize) {
        cout << "Requested size = " << size << " vs. this->bufferContentSize = " << this->bufferContentSize << endl;
        assert (!modifySize);
    }
    assert (size <= this->bufferContentSize);
}
