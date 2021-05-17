//
// Created by Andrei on 15-May-21.
//

#include <comm/utils/SerializationHeader.h>
#include <comm/utils/NetworkIncludes.h>

using namespace comm;
// using namespace std;

SerializationHeader::SerializationHeader() : serializationIteration(0), sendIteration(0), sendSize(0) {}

SerializationHeader::SerializationHeader(int header) : SerializationHeader() {
    this->fromInt(header);
}

SerializationHeader::SerializationHeader(unsigned char serializationIteration, unsigned char sendIteration,
                                         unsigned short sendSize) : SerializationHeader() {
    this->fromData(serializationIteration, sendIteration, sendSize);
}

void SerializationHeader::reset() {
    this->serializationIteration = 0;
    this->sendIteration = 0;
    this->sendSize = 0;
}

void SerializationHeader::fromInt(int header, bool local) {
    if (local) {
        this->sendSize = (unsigned short) (header & 65535);  // mod 2^16
    } else {
        this->sendSize = ntohs((unsigned short) (header & 65535));  // mod 2^16
    }
    header >>= 16;
    this->sendIteration = (unsigned char) (header & 255);  // mod 2^8
    this->serializationIteration = (unsigned char) (header >> 8);  // div 2^8
}

void SerializationHeader::fromData(unsigned char _serializationIteration, unsigned char _sendIteration,
                                   unsigned short _sendSize, bool local) {
    this->serializationIteration = _serializationIteration;
    this->sendIteration = _sendIteration;
    this->sendSize = (local) ? _sendSize : ntohs(_sendSize);
}

void SerializationHeader::setBuffer(char *buffer, bool setLocal, int start) const {
    buffer[start] = (char) this->serializationIteration;
    buffer[start + 1] = (char) this->sendIteration;
    auto x = this->getSendSize(setLocal);
    memcpy(buffer + start + 2, &x, 2);
}

void SerializationHeader::setBuffer(Buffer buffer, bool setLocal, int start) const {
    buffer.setChar((char) this->serializationIteration, start);
    buffer.setChar((char) this->sendIteration, start + 1);
    buffer.setShort((short) this->getSendSize(setLocal), start + 1);
}

unsigned char SerializationHeader::getSerializationIteration() const {
    return this->serializationIteration;
}

unsigned char SerializationHeader::getSendIteration() const {
    return this->sendIteration;
}

unsigned short SerializationHeader::getSendSize(bool getLocal) const {
    if (getLocal) {
        return this->sendSize;
    } else {
        return htons(this->sendSize);
    }
}

int SerializationHeader::getInt(bool getLocal) const {
    return (this->serializationIteration << 24) + (this->sendIteration << 16) +
           ((getLocal) ? this->sendSize : htons(this->sendSize));
}
