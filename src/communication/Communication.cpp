//
// Created by Andrei on 14.03.2021.
//

#include <comm/communication/Communication.h>
#include <comm/socket/utils.h>
#include <comm/utils/utils.hpp>
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace comm;
using namespace std;

Communication::Communication() : sockets(), sendBuffer(), recvBuffer(), isCopy(false), errorCode(0),
                                 sendHeader(), recvHeader() {}

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

bool Communication::transmitData(SocketType socketType, CommunicationData *data, bool withHeader, bool withMessageType,
                                 int retries, bool verbose) {
    if (data == nullptr) {
        return false;
    }
    this->errorCode = 0;

    bool serializeDone = false;
    // int dataStart = (withHeader) ? 4 : 0;
    int dataStart = 0;
    int serializationState = 0;
    while (!serializeDone) {
        if (withHeader) {
            this->sendHeader.setData(serializationState, 0, 0);
        }
        if (serializationState == 0 && withMessageType) {
            this->sendBuffer.setBufferContentSize(dataStart + 1);
            this->sendBuffer.setChar(char(data->getMessageType()), dataStart);
        } else {
            serializeDone = data->serialize(&(this->sendBuffer), dataStart, withHeader, verbose);
        }
        this->errorCode = 0;
        // TODO: change keepForNextSend = (SocketType == UDP && serializeDone);
        if (!this->send(socketType, withHeader, retries, false, verbose)) {
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
        serializationState++;
    }
    return true;
}

bool Communication::sendRaw(SocketType socketType, const char *data, int dataSize, int retries, bool verbose) {
    if (data == nullptr) {
        return false;
    }
    this->sendBuffer.setData(data, dataSize);
    this->errorCode = 0;
    if (!this->send(socketType, false, retries, false, verbose)) {
        if (this->errorCode == 0) {
            if (verbose) {
                cout << "Socket closed: Can not raw data serialized bytes..." << endl;
            }
        } else {
            printLastError();
            (*cerror) << "Can not send raw serialized bytes... error " << this->errorCode << endl;
        }
        return false;
    }
    return true;
}

bool Communication::recvMessageType(SocketType socketType, MessageType &messageType, bool withHeader,
                                    bool syphonWronglySerializedData, int retries, bool verbose) {
    bool receiveResult;
    char *dataLocalDeserializeBuffer;
    uint64_t expectedSize;
    // int dataStart = (withHeader) ? 4 : 0;
    int dataStart = 0;
    this->errorCode = 0;
    if (withHeader) {
        this->recvHeader.setData(0, 0, 0);
        this->recvHeader.setSyphonUntilFirstMessage(syphonWronglySerializedData);
    }
    this->preReceiveMessageType(dataLocalDeserializeBuffer, expectedSize, dataStart);
    receiveResult = this->doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries, false,
                                    verbose);
    return this->postReceiveMessageType(messageType, receiveResult, dataStart);
}

bool Communication::recvData(SocketType socketType, CommunicationData *data, bool withHeader, bool syphonWrongSerialize,
                             bool gotMessageType, int retries, bool verbose) {
    bool receiveResult, deserializationDone = false, receivedSomething = false;
    int deserializeState = (int) gotMessageType, localRetriesThreshold = 0, localRetries = localRetriesThreshold;
    char *dataLocalDeserializeBuffer = nullptr;
    uint64_t expectedSize;
    // int dataStart = (withHeader) ? 4 : 0;
    int dataStart = 0;
    MessageType messageType = data->getMessageType();
    while (!deserializationDone && localRetries >= 0) {
        this->errorCode = 0;
        if (withHeader) {
            this->recvHeader.setData(deserializeState, 0, 0);
            this->recvHeader.setSyphonUntilFirstMessage(syphonWrongSerialize);
        }
        this->preReceiveData(dataLocalDeserializeBuffer, expectedSize, dataStart, data, withHeader);
        receiveResult = this->doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries,
                                        (deserializeState != 0), verbose);
        if (!this->postReceiveData(data, deserializeState, localRetries, receivedSomething, deserializationDone,
                                   messageType, dataStart, localRetriesThreshold, receiveResult, withHeader, verbose)) {
            return false;
        }
    }
    if (receivedSomething && !deserializationDone) {
        cout << "After loop: Could not recv " << messageTypeToString(messageType)
             << " serialized bytes... error " << this->errorCode << "; deserializeState = " << deserializeState << endl;
        (*cerror) << "After loop: Could not recv " << messageTypeToString(messageType)
                  << " serialized bytes... error " << this->errorCode << "; deserializeState = " << deserializeState
                  << endl;
        if (data != nullptr) {
            data->resetDeserializeState();
        }
        return false;
    } else if (!receivedSomething) {
        assert (this->errorCode == -1);
        if (verbose) {
            cout << "Did not receive anything although expected " << messageTypeToString(messageType)
                 << endl;
        }
        return false;
    }
    return true;
}

bool Communication::receiveData(SocketType socketType, DataCollection *data, bool withHeader, bool syphonWrongSerialize,
                                int retries, bool verbose) {
    bool deserializationDone = false, receiveResult, receivedSomething = false;
    int deserializeState = 0;
    int localRetriesThreshold = 0, localRetries = localRetriesThreshold;  // 10
    uint64_t expectedSize;
    char *dataLocalDeserializeBuffer = nullptr;
    MessageType messageType;
    CommunicationData *recvData = nullptr;
    // int dataStart = (withHeader) ? 4 : 0;
    int dataStart = 0;

    while (!deserializationDone && localRetries >= 0) {
        this->errorCode = 0;
        if (withHeader) {
            this->recvHeader.setData(deserializeState, 0, 0);
            this->recvHeader.setSyphonUntilFirstMessage(syphonWrongSerialize);
        }

        // receive setup
        if (deserializeState == 0) {
            this->preReceiveMessageType(dataLocalDeserializeBuffer, expectedSize, dataStart);
        } else {
            this->preReceiveData(dataLocalDeserializeBuffer, expectedSize, dataStart, recvData, withHeader);
            if (verbose) {
                cout << "In Communication::fullReceiveData: dataLocalDeserializeBuffer = "
                     << (int *) dataLocalDeserializeBuffer << "; expectedSize = " << expectedSize
                     << "; deserializeState = " << deserializeState << endl;
            }
        }

        // do receive
        receiveResult = this->doReceive(socketType, dataLocalDeserializeBuffer, expectedSize, withHeader, retries,
                                        (deserializeState != 0), verbose);

        // post receive
        if (deserializeState == 0) {
            if (!this->postReceiveMessageType(messageType, receiveResult, dataStart)) {
                return false;
            }

            if (messageType == MessageType::NOTHING) {
                deserializationDone = true;
                break;
            } else {
                recvData = data->get(messageType);
            }
        } else {
            if (!this->postReceiveData(recvData, deserializeState, localRetries, receivedSomething, deserializationDone,
                                       messageType, dataStart, localRetriesThreshold, receiveResult, withHeader,
                                       verbose)) {
                return false;
            }
        }

        deserializeState++;
    }

    if (receivedSomething && !deserializationDone) {
        cout << "After loop: Could not recv " << messageTypeToString(messageType)
             << " serialized bytes... error " << this->errorCode << "; deserializeState = " << deserializeState << endl;
        (*cerror) << "After loop: Could not recv " << messageTypeToString(messageType)
                  << " serialized bytes... error " << this->errorCode << "; deserializeState = " << deserializeState
                  << endl;
        if (recvData != nullptr) {
            recvData->resetDeserializeState();
        }
        return false;
    } else if (!receivedSomething) {
        assert (this->errorCode == -1);
        if (verbose) {
            cout << "Did not receive anything although expected " << messageTypeToString(messageType)
                 << endl;
        }
        return false;
    }
    return true;
}

bool Communication::receiveRaw(SocketType socketType, char *&data, bool &receivedData, int retries, bool verbose) {
    receivedData = false;
    if (data == nullptr) {
        return false;
    }

    this->recvBuffer.setReferenceToData(data, 0);
    this->errorCode = 0;
    if (!this->recv(socketType, false, retries, false, verbose)) {
        if (this->errorCode == -1) {
            if (verbose) {
                cout << "Received nothing..." << endl;
            }
        } else {
            if (verbose && this->errorCode == 0) {
                cout << "Socket closed: Can not receive raw serialized bytes..." << endl;
            }
            if (this->errorCode != 0) {
                printLastError();
                (*cerror) << "Can not receive raw serialized bytes... error " << this->errorCode << endl;
            }
            return false;
        }
    }
    receivedData = this->errorCode >= 0;
    return true;
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

void Communication::setSocketTimeouts(SocketType socketType, int _sendTimeout, int _recvTimeout) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return;
    }
    socket->setSocketTimeouts(_sendTimeout, _recvTimeout);
}

void Communication::setSocket(SocketType socketType, Socket *socket) {
    if (mapGet(this->sockets, socketType)) {
        this->sockets[socketType]->cleanup();
        delete this->sockets[socketType];
    }
    this->sockets[socketType] = socket;
}

void Communication::setPartner(SocketType socketType, SocketAddress _partner, bool overwrite) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return;
    }
    socket->setPartner(_partner, overwrite);
}

void Communication::setPartner(SocketType socketType, const string &partnerIP, int partnerPort, bool overwrite) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return;
    }
    socket->setPartner(partnerIP, partnerPort, overwrite);
}

void Communication::setOverwritePartner(SocketType socketType, bool overwrite) {
    Socket *&socket = this->getSocket(socketType);
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

SocketPartner *Communication::getMyself(SocketType socketType) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr || !socket->isInitialized()) {
        (*cerror) << socketTypeToString(socketType) << " socket is not initialized! Can't getMyself..." << endl;
        return nullptr;
    }
    return socket->getMyself();
}

string Communication::getMyAddressString(SocketType socketType) {
    SocketPartner *myself = this->getMyself(socketType);
    assert (myself != nullptr);
    return myself->getPartnerString();
}

SocketPartner *&Communication::getPartner(SocketType socketType) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr || !socket->isInitialized()) {
        (*cerror) << socketTypeToString(socketType) << " socket is not initialized! Can't getPartner..." << endl;
        throw runtime_error(socketTypeToString(socketType) + " socket is not initialized! Can't getPartner...");
    }
    return socket->getPartner();
}

string Communication::getPartnerString(SocketType socketType) {
    SocketPartner *&partner = this->getPartner(socketType);
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

bool Communication::send(SocketType socketType, bool withHeader, int retries, bool keepForNextSend, bool verbose) {
    return this->send(socketType, this->sendBuffer.getConstBuffer(), this->sendBuffer.getBufferContentSize(),
                      (withHeader) ? &this->sendHeader : nullptr, retries, keepForNextSend, verbose);
}

bool Communication::send(SocketType socketType, const char *buffer, uint64_t contentSize,
                         SerializationHeader *header, int retries, bool keepForNextSend, bool verbose) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return false;
    }
    return socket->sendBytes(buffer, contentSize, this->errorCode, header, retries, keepForNextSend, verbose);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

void Communication::preReceiveMessageType(char *&dataLocalDeserializeBuffer, uint64_t &expectedSize,
                                          const int dataStart) {
    dataLocalDeserializeBuffer = nullptr;
    expectedSize = dataStart + 1;
}

void Communication::preReceiveData(char *&dataLocalDeserializeBuffer, uint64_t &expectedSize,
                                   const int dataStart, CommunicationData *const recvData, bool withHeader) {
    dataLocalDeserializeBuffer = recvData->getDeserializeBuffer();
    // don't to the if before, because some CommunicationData depend on calling getDeserializeBuffer!
    if (withHeader) {
        dataLocalDeserializeBuffer = nullptr;
    }
    expectedSize = dataStart + recvData->getExpectedDataSize();
}

#pragma clang diagnostic pop

bool Communication::doReceive(SocketType socketType, char *&dataLocalDeserializeBuffer, uint64_t &expectedSize,
                              bool withHeader, int retries, bool expectingData, bool verbose) {
    if (dataLocalDeserializeBuffer != nullptr) {
        assert (!withHeader);
        return this->recv(socketType, dataLocalDeserializeBuffer, expectedSize, expectedSize, nullptr, retries,
                          expectingData, verbose);
    } else {
        this->recvBuffer.setBufferContentSize(expectedSize);
        return this->recv(socketType, withHeader, retries, expectingData, verbose);
    }
}

bool Communication::postReceiveMessageType(MessageType &messageType, const bool receiveResult, int dataStart) {
    if (!receiveResult) {
        if (this->getErrorCode() < 0) {
            messageType = MessageType::NOTHING;
        } else {
            return false;
        }
    } else {
        messageType = MessageType(int(this->recvBuffer.getChar(dataStart)));
    }
    return true;
}

bool Communication::postReceiveData(CommunicationData *&recvData, int &deserializeState, int &localRetries,
                                    bool &receivedSomething, bool &deserializationDone, MessageType messageType,
                                    const int dataStart, const int localRetriesThreshold, const bool receiveResult,
                                    const bool withHeader, const bool verbose) {
    if (!receiveResult) {
        if (this->getErrorCode() >= 0) {
            printLastError();
            (*cerror) << "Stop loop: Can not recv data serialized bytes... error " << this->getErrorCode()
                      << "; deserializeState = " << deserializeState << endl;
            recvData->resetDeserializeState();
            return false;
        } else if (this->getErrorCode() == -2) {
            (*cerror) << "Stop loop: Only part of the data (" << messageTypeToString(messageType)
                      << ") has been received before new message started...; deserializeState = "
                      << deserializeState << endl;
            recvData->resetDeserializeState();
            return false;
        } else if (this->getErrorCode() == -3) {
            (*cerror) << "Stop loop: NETWORK ERRORS when receiving " << messageTypeToString(messageType)
                      << "... Maybe there was a send error...; deserializeState = " << deserializeState << endl;
            recvData->resetDeserializeState();
            return false;
        }
    }

    assert (receiveResult || this->getErrorCode() == -1);
    // if we received something...
    if (this->getErrorCode() != -1) {
        if (verbose) {
            cout << "Received something! data->getMessageType() " << messageTypeToString(messageType) << endl;
        }
        receivedSomething = true;
        deserializationDone = recvData->deserialize(&(this->recvBuffer), dataStart, withHeader, verbose);
        deserializeState++;
        localRetries = localRetriesThreshold;
    } else {
        if (verbose) {
            cout << "Received nothing, decrease local retries!" << endl;
        }
        localRetries--;
    }
    return true;
}

bool Communication::recv(SocketType socketType, bool withHeader, int retries, bool expectingData, bool verbose) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return false;
    }
    return socket->receiveBytes(this->recvBuffer, this->errorCode, (withHeader) ? &(this->recvHeader) : nullptr,
                                retries, expectingData, verbose);
}

bool Communication::recv(SocketType socketType, char *&buffer, uint64_t &bufferSize,
                         uint64_t expectedBytes, SerializationHeader *expectedHeader, int retries,
                         bool expectingData, bool verbose) {
    Socket *&socket = this->getSocket(socketType);
    if (socket == nullptr) {
        return false;
    }
    return socket->receiveBytes(buffer, bufferSize, expectedBytes, this->errorCode, expectedHeader, retries,
                                expectingData, verbose);
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
