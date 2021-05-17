//
// Created by Andrei on 15-May-21.
//

#include <comm/utils/SerializationHeader.h>
#include <comm/utils/NetworkIncludes.h>

using namespace comm;
// using namespace std;

SerializationHeader::SerializationHeader(bool create) : localBuffer(nullptr), passedBuffer(nullptr), created(create) {
    if (this->created) {
        this->localBuffer = new char[4];
    }
}

SerializationHeader::SerializationHeader(char *buffer) : SerializationHeader(false) {
    this->localBuffer = buffer;
}

SerializationHeader::SerializationHeader(Buffer &buffer) : SerializationHeader(false) {
    this->passedBuffer = &buffer;
}

SerializationHeader::SerializationHeader(int header) : SerializationHeader() {
    this->setInt(header);
}

SerializationHeader::SerializationHeader(unsigned char serializationIteration, unsigned char sendIteration,
                                         unsigned short sendSize) : SerializationHeader() {
    this->setData(serializationIteration, sendIteration, sendSize);
}

SerializationHeader::~SerializationHeader() {
    if (this->created) {
        delete[] this->localBuffer;
    }
}

void SerializationHeader::reset() {
    this->setInt(0);
}

void SerializationHeader::setInt(int header, bool local) {
    this->setSendSize((unsigned short) (header & 65535), local);  // mod 2^16
    header >>= 16;  // div 2^16
    this->setSendIteration((unsigned char) (header & 255));  // mod 2^8
    this->setSerializationIteration((unsigned char) (header >> 8));  // div 2^8
}

void SerializationHeader::setData(unsigned char _serializationIteration, unsigned char _sendIteration,
                                  unsigned short _sendSize, bool local) {
    this->setSerializationIteration(_serializationIteration);
    this->setSendIteration(_sendIteration);
    this->setSendSize(_sendSize, local);
}

void SerializationHeader::setSerializationIteration(unsigned char _serializationIteration) {
    if (this->passedBuffer != nullptr) {
        this->localBuffer[0] = (char) _serializationIteration;
    } else {
        this->passedBuffer->setChar((char) _serializationIteration, 0);
    }
}

void SerializationHeader::setSendIteration(unsigned char _sendIteration) {
    if (this->passedBuffer != nullptr) {
        this->localBuffer[1] = (char) _sendIteration;
    } else {
        this->passedBuffer->setChar((char) _sendIteration, 1);
    }
}

void SerializationHeader::setSendSize(unsigned short _sendSize, bool setLocal) {
    auto sendSize = (short) ((setLocal) ? _sendSize : ntohs(_sendSize));
    if (this->passedBuffer != nullptr) {
        memcpy(this->localBuffer + 2, &sendSize, 2);
    } else {
        this->passedBuffer->setShort(sendSize, 2);
    }
}

void SerializationHeader::setBuffer(char *buffer, bool setLocal, int start) const {
    buffer[start] = (char) this->getSerializationIteration();
    buffer[start + 1] = (char) this->getSendIteration();
    auto x = this->getSendSize(setLocal);
    memcpy(buffer + start + 2, &x, 2);
}

void SerializationHeader::setBuffer(Buffer &buffer, bool setLocal, int start) const {
    buffer.setInt(this->getInt(setLocal), start);
}

unsigned char SerializationHeader::getSerializationIteration() const {
    if (this->passedBuffer != nullptr) {
        return this->localBuffer[0];
    } else {
        return this->passedBuffer->getChar(0);
    }
}

unsigned char SerializationHeader::getSendIteration() const {
    if (this->passedBuffer != nullptr) {
        return this->localBuffer[1];
    } else {
        return this->passedBuffer->getChar(1);
    }
}

unsigned short SerializationHeader::getSendSize(bool getLocal) const {
    unsigned short sendSize;
    if (this->passedBuffer != nullptr) {
        memcpy(&sendSize, this->localBuffer + 2, 2);
    } else {
        sendSize = this->passedBuffer->getShort(2);
    }
    if (getLocal) {
        return sendSize;
    } else {
        return htons(sendSize);
    }
}

int SerializationHeader::getInt(bool getLocal) const {
    return (this->getSerializationIteration() << 24) + (this->getSendIteration() << 16) + this->getSendSize(getLocal);
}
