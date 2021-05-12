//
// Created by ga78cat on 14.03.2021.
//

#include <comm/communication/Communication.h>
#include <comm/socket/utils.h>
#include <comm/utils/utils.hpp>
#include <cassert>
#include <iostream>

using namespace comm;
using namespace std;

Communication::Communication() : sockets(), sendBuffer(), recvBuffer(), isCopy(false), errorCode(0), dataCollection() {}

Communication::~Communication() {
    this->_cleanupData();
    this->isCopy = false;
}

void Communication::cleanup() {
    this->_cleanup();
    this->isCopy = false;
}

Communication *Communication::copy() const {
    auto *copy = new Communication();
    this->copyData(copy);
    return copy;
}

void Communication::createSocket(SocketType socketType, SocketPartner *partner, int myPort, int sendTimeout,
                                 int recvTimeout) {
    Socket *&socket = this->getSocket(socketType);
    delete socket;
    socket = new Socket(socketType, partner, myPort, sendTimeout, recvTimeout);
}

void Communication::createSocket(SocketType socketType, const SocketPartner &partner, int myPort, int sendTimeout,
                                 int recvTimeout) {
    Socket *&socket = this->getSocket(socketType);
    delete socket;
    socket = new Socket(socketType, partner, myPort, sendTimeout, recvTimeout);
}

void Communication::setSocketTimeouts(int _sendTimeout, int _recvTimeout) {
    for (auto &socketData : this->sockets) {
        this->setSocketTimeouts(socketData.first, _sendTimeout, _recvTimeout);
    }
}

void Communication::setSocketTimeouts(SocketType type, int _sendTimeout, int _recvTimeout) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return;
    }
    socket->setSocketTimeouts(_sendTimeout, _recvTimeout);
}

bool Communication::sendMessageType(SocketType type, const MessageType *messageType, int retries, bool verbose) {
    if (messageType == nullptr) {
        return false;
    }
    this->errorCode = 0;
    this->sendBuffer.setBufferContentSize(1);
    this->sendBuffer.setChar(char(*messageType), 0);
    if (verbose) {
        cout << "Before sending message type..." << endl;
    }
    if (!this->send(type, retries, verbose)) {
        if (this->errorCode == 0) {
            if (verbose) {
                cout << "Socket closed: Can not send messageType... " << messageTypeToString(*messageType) << endl;
            }
        } else {
            (*cerror) << "Can not send messageType... " << messageTypeToString(*messageType) << endl;
            if (verbose) {
                cout << "Can not send messageType... " << messageTypeToString(*messageType) << endl;
            }
        }
        return false;
    }
    return true;
}

bool Communication::sendData(SocketType type, CommunicationData *data, bool withMessageType, int retries,
                             bool verbose) {
    if (data == nullptr) {
        return false;
    }
    this->errorCode = 0;

    if (withMessageType) {
        if (verbose) {
            cout << "Before sending message type..." << endl;
        }
        auto x = data->getMessageType();
        if (verbose) {
            cout << "Message type = " << messageTypeToString(x) << endl;
        }
        if (!this->sendMessageType(type, &x, retries, verbose)) {
            if (this->errorCode == 0) {
                if (verbose) {
                    cout << "Socket closed: Can not send data message type bytes..." << endl;
                }
            } else {
                printLastError();
                (*cerror) << "Can not send data message type bytes... error " << this->errorCode << endl;
            }
            return false;
        }
        if (verbose) {
            cout << "Sent message type!!!" << endl;
        }
    }

    bool serializeDone = false;
    while (!serializeDone) {
        serializeDone = data->serialize(&(this->sendBuffer), verbose);
        this->errorCode = 0;
        if (!this->send(type, retries, verbose)) {
            if (this->errorCode == 0) {
                if (verbose) {
                    cout << "Socket closed: Can not send data serialized bytes..." << endl;
                }
            } else {
                printLastError();
                (*cerror) << "Can not send data serialized bytes... error " << this->errorCode << endl;
            }
            data->resetSerializeState();
            return false;
        }
    }
    return true;
}

bool Communication::sendRaw(SocketType type, const char *data, int dataSize, int retries, bool verbose) {
    if (data == nullptr) {
        return false;
    }
    this->sendBuffer.setData(data, dataSize);
    this->errorCode = 0;
    if (!this->send(type, retries, verbose)) {
        if (this->errorCode == 0) {
            if (verbose) {
                cout << "Socket closed: Can not send data serialized bytes..." << endl;
            }
        } else {
            printLastError();
            (*cerror) << "Can not send data serialized bytes... error " << this->errorCode << endl;
        }
        return false;
    }
    return true;
}

bool Communication::recvMessageType(SocketType type, MessageType *messageType, int retries, bool verbose) {
    this->errorCode = 0;
    this->recvBuffer.setBufferContentSize(1);

    bool result = this->recv(type, retries, verbose);
    if (messageType == nullptr) {
        return result;
    }

    if (!result) {
        // why < 1 (why is also 0, when the socket closes, ok???)
        if (this->errorCode < 1) {
            *messageType = MessageType::NOTHING;
            return true;
        }
        return false;
    }
    *messageType = MessageType(int(this->recvBuffer.getChar(0)));
    return true;
}

bool Communication::recvData(SocketType type, CommunicationData *data, int retries, bool verbose) {
    assert (data != nullptr);
    this->errorCode = 0;
    int localRetriesThreshold = 0, localRetries = localRetriesThreshold, deserializeState = 0;  // 10
    unsigned long long int expectedSize;
    bool deserializeDone = false, receiveResult, receivedSomething = false;
    char *dataLocalDeserializeBuffer = nullptr;
    while (!deserializeDone && localRetries >= 0) {
        if (verbose) {
            cout << "Communication::recvData: LocalRetries = " << localRetries << " data->getMessageType() "
                 << messageTypeToString(data->getMessageType()) << "; deserializeState = " << deserializeState << endl;
        }
        this->errorCode = 0;

        dataLocalDeserializeBuffer = data->getDeserializeBuffer();
        expectedSize = data->getExpectedDataSize();
        if (verbose) {
            cout << "In Communication::recvData: dataLocalDeserializeBuffer = " << (int *) dataLocalDeserializeBuffer
                 << "; expectedSize = " << expectedSize << "; deserializeState = " << deserializeState << endl;
        }
        if (dataLocalDeserializeBuffer != nullptr) {
            receiveResult = this->recv(type, dataLocalDeserializeBuffer, expectedSize, expectedSize, retries, verbose);
            if (verbose) {
                cout << "ReceiveResult = " << receiveResult << endl;
            }
        } else {
            this->recvBuffer.setBufferContentSize(expectedSize);
            receiveResult = this->recv(type, retries, verbose);
        }
        if (!receiveResult) {
            if (this->errorCode >= 0) {
                printLastError();
                (*cerror) << "Stop loop: Can not recv data serialized bytes... error " << this->errorCode
                          << "; deserializeState = " << deserializeState << endl;
                data->resetDeserializeState();
                return false;
            } else if (this->errorCode == -2) {
                (*cerror) << "Stop loop: Only part of the data has been received before new message started... "
                          << "; deserializeState = " << deserializeState << endl;
                data->resetDeserializeState();
                return false;
            }
        }
        assert (receiveResult || this->errorCode == -1);
        // if we received something...
        if (this->errorCode != -1) {
            if (verbose) {
                cout << "Received something! data->getMessageType() " << messageTypeToString(data->getMessageType())
                     << endl;
            }
            receivedSomething = true;
            deserializeDone = data->deserialize(&this->recvBuffer, 0, verbose);
            deserializeState++;
            localRetries = localRetriesThreshold;
        } else {
            if (verbose) {
                cout << "Received nothing, decrease local retries!" << endl;
            }
            localRetries--;
        }
    }
    if (receivedSomething && !deserializeDone) {
        cout << "After loop: Could not recv data serialized bytes... error " << this->errorCode
             << "; deserializeState = " << deserializeState << endl;
        (*cerror) << "After loop: Could not recv data serialized bytes... error " << this->errorCode
                  << "; deserializeState = " << deserializeState << endl;
        data->resetDeserializeState();
        return false;
    } else if (!receivedSomething) {
        assert (this->errorCode == -1);
        if (verbose) {
            cout << "Did not receive anything although expected " << messageTypeToString(data->getMessageType())
                 << endl;
        }
        return false;
    }
    return true;
}

void Communication::setSocket(SocketType socketType, Socket *socket) {
    if (mapGet(this->sockets, socketType)) {
        this->sockets[socketType]->cleanup();
        delete this->sockets[socketType];
    }
    this->sockets[socketType] = socket;
}

void Communication::setPartner(SocketType type, SocketAddress _partner, bool overwrite) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return;
    }
    socket->setPartner(_partner, overwrite);
}

void Communication::setPartner(SocketType type, const string &partnerIP, int partnerPort, bool overwrite) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return;
    }
    socket->setPartner(partnerIP, partnerPort, overwrite);
}

void Communication::setOverwritePartner(SocketType type, bool overwrite) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return;
    }
    socket->setOverwritePartner(overwrite);
}

Socket *&Communication::getSocket(SocketType socketType) {
    if (!mapContains(this->sockets, socketType)) {
        this->sockets[socketType] = nullptr;
    }
    return this->sockets[socketType];
}

SocketPartner *Communication::getMyself(SocketType type) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr || !socket->isInitialized()) {
        (*cerror) << socketTypeToString(type) << " socket is not initialized! Can't getMyself..." << endl;
        return nullptr;
    }
    return socket->getMyself();
}

string Communication::getMyAddressString(SocketType type) {
    SocketPartner *myself = this->getMyself(type);
    assert (myself != nullptr);
    return myself->getPartnerString();
}

SocketPartner *&Communication::getPartner(SocketType type) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr || !socket->isInitialized()) {
        (*cerror) << socketTypeToString(type) << " socket is not initialized! Can't getPartner..." << endl;
        throw runtime_error(socketTypeToString(type) + " socket is not initialized! Can't getPartner...");
    }
    return socket->getPartner();
}

string Communication::getPartnerString(SocketType type) {
    SocketPartner *&partner = this->getPartner(type);
    if (partner == nullptr) {
        return "???";
    }
    return partner->getPartnerString();
}

int Communication::getErrorCode() const {
    return this->errorCode;
}

const char *Communication::getErrorString() const {
    return getLastErrorString(this->errorCode);
}

void Communication::copyData(Communication *copy) const {
    copy->isCopy = true;
    for (auto &socketData : this->sockets) {
        const Socket *const &socket = socketData.second;
        if (socket != nullptr) {
            copy->sockets[socketData.first] = socket->copy();
        }
    }
}

void Communication::_cleanup() {
    this->_cleanupData();
}

bool Communication::send(SocketType type, int retries, bool verbose) {
    return this->send(type, this->sendBuffer.getBuffer(), this->sendBuffer.getBufferContentSize(), retries, verbose);
}

bool Communication::send(SocketType type, char *buffer, unsigned long long int contentSize, int retries, bool verbose) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return false;
    }
    return socket->sendBytes(buffer, contentSize, this->errorCode, retries, verbose);
}

bool Communication::recv(SocketType type, int retries, bool verbose) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return false;
    }
    return socket->receiveBytes(this->recvBuffer, this->errorCode, retries, verbose);
}

bool Communication::recv(SocketType type, char *&buffer, unsigned long long int &bufferSize,
                         unsigned long long int expectedBytes, int retries, bool verbose) {
    Socket *&socket = this->getSocket(type);
    if (socket == nullptr) {
        return false;
    }
    return socket->receiveBytes(buffer, bufferSize, expectedBytes, this->errorCode, retries, verbose);
}

void Communication::_cleanupData() {
    for (auto &socketData : this->sockets) {
        Socket *&socket = socketData.second;
        if (socket != nullptr) {
            socket->cleanup();
            if (!this->isCopy) {
                delete socket;
            }
            socket = nullptr;
        }
    }
    this->sockets.clear();
}
