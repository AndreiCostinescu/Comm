//
// Created by ga78cat on 24.03.2021.
//

#include <cassert>
#include <comm/communication/Communicator.h>
#include <comm/socket/utils.h>
#include <comm/data/dataUtils.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <stdexcept>

using namespace comm;
using namespace std;

Communicator::Communicator() : quit(false), state(COMMUNICATOR_IDLE) {}

Communicator::~Communicator() = default;

void Communicator::main() {
    while (!this->quit) {
        this->_preMain();
        try {
            this->_main();
        } catch (exception &e) {
            cout << "Exception caught: " << e.what() << endl;
        }
        this->_postMain();
    }
}

void Communicator::stop() {
    this->quit = true;
}

bool Communicator::isReceiveErrorOk(int errorCode, MessageType *messageType, bool nothingOk) {
    if (errorCode < 0) {
        // nothing received
        if (messageType != nullptr) {
            *messageType = MessageType::NOTHING;
        }
        return nothingOk;
    } else if (errorCode == 0) {
        cout << "Socket closed." << endl;
    } else {
        (*cerror) << "Communicator::isReceiveErrorOk ";
        printLastError();
    }
    return false;
}

bool Communicator::send(Communication *comm, SocketType type, CommunicationData *data, int retries, bool verbose) {
    if (data == nullptr) {
        return true;
    }
    if (!comm->sendData(type, data, true, retries, verbose)) {
        if (comm->getErrorCode() > 0) {
            (*cerror) << "When sending data... error: " << getLastErrorString(comm->getErrorCode()) << endl;
        }
        return false;
    }
    return true;
}

bool Communicator::send(Communication *comm, SocketType type, MessageType *messageType, CommunicationData *data,
                        int retries, bool verbose) {
    if (messageType == nullptr || data != nullptr) {
        return Communicator::send(comm, type, data, retries, verbose);
    }
    if (!comm->sendMessageType(type, messageType, retries, verbose)) {
        if (comm->getErrorCode() > 0) {
            (*cerror) << "When sending data... error: " << getLastErrorString(comm->getErrorCode()) << endl;
        }
        return false;
    }
    return true;
}

bool Communicator::syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                          int retries, bool verbose, int syphonRetries) {
    switch (messageType) {
        case MessageType::NOTHING: {
            return true;
        }
        case MessageType::COORDINATE:
        case MessageType::IMAGE:
        case MessageType::STATUS: {
            int localSyphonRetries = (syphonRetries < 1) ? 1 : syphonRetries;
            while (!this->quit && localSyphonRetries > 0) {
                if (!comm->recvData(type, data, retries, verbose)) {
                    if (comm->getErrorCode() == -1) {
                        localSyphonRetries--;
                        continue;
                    }
                    // it shouldn't be possible to receive nothing if we already have received the header!!!
                    // set nothingOk to false!
                    return Communicator::isReceiveErrorOk(comm->getErrorCode(), &messageType, false);
                }
                return true;
            }
            return false;
        }
        default: {
            throw runtime_error("Unknown message type: " + to_string(messageType));
        }
    }
}

bool Communicator::listen(Communication *comm, SocketType type, MessageType &messageType,
                          DataCollection &_dataCollection) {
    if (!comm->recvMessageType(type, &messageType)) {
        if (Communicator::isReceiveErrorOk(comm->getErrorCode(), &messageType, true)) {
            return true;
        }
        (*cerror) << "Error when receiving message type... setting \"quit\"" << endl;
        messageType = MessageType::STATUS;
        auto *status = (StatusData *) _dataCollection.get(messageType);
        status->setCommand("quit");
        return true;
    }

    CommunicationData *data = _dataCollection.get(messageType);
    MessageType receivedMessageType = messageType;
    if (!this->syphon(comm, type, messageType, data)) {
        (*cerror) << "Error when syphoning data " << messageTypeToString(receivedMessageType)
                  << "... setting \"quit\"" << endl;
        messageType = MessageType::STATUS;
        auto *status = (StatusData *) _dataCollection.get(messageType);
        status->setCommand("quit");
    }
    return true;
}

bool Communicator::listenFor(Communication *comm, SocketType type, CommunicationData *data, int countIgnoreOther,
                             int countIgnore) {
    assert(data != nullptr);
    MessageType messageType;
    while (!this->quit) {
        if (!comm->recvMessageType(type, &messageType)) {
            if (Communicator::isReceiveErrorOk(comm->getErrorCode(), &messageType, true)) {
                continue;
            }
            return false;
        }

        if (messageType != data->getMessageType()) {
            if (messageType != MessageType::NOTHING) {
                (*cerror) << "Wrong messageType... expected " << messageTypeToString(data->getMessageType()) << "; got "
                          << messageTypeToString(messageType) << endl;
            }
            CommunicationData *syphonData = createCommunicationDataPtr(messageType);
            if (!this->syphon(comm, type, messageType, syphonData)) {
                delete syphonData;
                return false;
            }
            delete syphonData;
            // it can happen that messageType becomes NOTHING because we don't receive any data in this->syphon!
            if (messageType != MessageType::NOTHING && countIgnoreOther > 0) {
                countIgnoreOther--;
            }
            if (countIgnore > 0) {
                countIgnore--;
            }
            if (countIgnoreOther == 0 || countIgnore == 0) {
                return false;
            }
        } else {
            break;
        }
    }
    if (this->quit) {
        return false;
    }
    while (!this->quit) {
        if (!this->syphon(comm, type, messageType, data)) {
            if (comm->getErrorCode() < 0) {
                continue;
            }
            return false;
        }
        break;
    }
    return !this->quit;
}

void Communicator::_preMain() {}

void Communicator::_main() {}

void Communicator::_postMain() {}
