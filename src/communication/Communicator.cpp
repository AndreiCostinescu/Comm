//
// Created by Andrei on 24.03.2021.
//

#include <cassert>
#include <comm/communication/Communicator.h>
#include <comm/socket/utils.h>
#include <comm/data/dataUtils.h>
#include <comm/data/messages.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <stdexcept>

using namespace comm;
using namespace std;

bool Communicator::send(Communication *comm, SocketType socketType, CommunicationData *data, bool withHeader,
                        bool withMessageType, int retries, bool verbose) {
    if (data == nullptr) {
        return true;
    }
    if (!comm->transmitData(socketType, data, withHeader, withMessageType, retries, verbose)) {
        if (comm->getErrorCode() > 0) {
            (*cerror) << "When sending data... error: " << getLastErrorString(comm->getErrorCode()) << endl;
        }
        return false;
    }
    return true;
}

bool Communicator::send(Communication *comm, SocketType socketType, const char *data, int dataSize, int retries,
                        bool verbose) {
    return comm->sendRaw(socketType, data, dataSize, retries, verbose);
}

bool Communicator::syphon(Communication *comm, SocketType socketType, MessageType &messageType, CommunicationData *data,
                          bool withHeader, const bool *quitFlag, int retries, bool verbose, int syphonRetries) {
    return Communicator::_syphon(comm, socketType, messageType, data, withHeader,
                                 [quitFlag]() { return quitFlag == nullptr || !(*quitFlag); }, retries, verbose,
                                 syphonRetries);
}

bool Communicator::syphon(Communication *comm, SocketType socketType, MessageType &messageType, CommunicationData *data,
                          bool withHeader, const bool &quitFlag, int retries, bool verbose, int syphonRetries) {
    return Communicator::_syphon(comm, socketType, messageType, data, withHeader, [quitFlag]() { return !quitFlag; },
                                 retries, verbose, syphonRetries);
}

bool Communicator::syphon(Communication *comm, SocketType socketType, MessageType &messageType, CommunicationData *data,
                          bool withHeader, const atomic<bool> &quitFlag, int retries, bool verbose, int syphonRetries) {
    return Communicator::_syphon(comm, socketType, messageType, data, withHeader, [&quitFlag]() { return !quitFlag; },
                                 retries, verbose, syphonRetries);
}

bool Communicator::listen(Communication *comm, SocketType socketType, MessageType &messageType,
                          DataCollection &_dataCollection, bool withHeader, const bool *quitFlag, int retries,
                          bool verbose) {
    return Communicator::_listen(comm, socketType, messageType, _dataCollection, withHeader,
                                 [quitFlag]() { return quitFlag == nullptr || !(*quitFlag); }, retries, verbose);
}

bool Communicator::listen(Communication *comm, SocketType socketType, MessageType &messageType,
                          DataCollection &_dataCollection, bool withHeader, const bool &quitFlag, int retries,
                          bool verbose) {
    return Communicator::_listen(comm, socketType, messageType, _dataCollection, withHeader,
                                 [quitFlag]() { return !quitFlag; }, retries, verbose);
}

bool Communicator::listen(Communication *comm, SocketType socketType, MessageType &messageType,
                          DataCollection &_dataCollection, bool withHeader, const atomic<bool> &quitFlag, int retries,
                          bool verbose) {
    return Communicator::_listen(comm, socketType, messageType, _dataCollection, withHeader,
                                 [&quitFlag]() { return !quitFlag; }, retries, verbose);
}

bool Communicator::listenFor(Communication *comm, SocketType socketType, CommunicationData *data, bool withHeader,
                             const bool *quitFlag, bool *timeoutResult, int countIgnoreOther, int countOther,
                             int retries, bool verbose) {
    return Communicator::_listenFor(comm, socketType, data, withHeader,
                                    [quitFlag]() { return quitFlag == nullptr || !(*quitFlag); },
                                    timeoutResult, countIgnoreOther, countOther, retries, verbose);
}

bool Communicator::listenFor(Communication *comm, SocketType socketType, CommunicationData *data, bool withHeader,
                             const bool &quitFlag, bool *timeoutResult, int countIgnoreOther, int countOther,
                             int retries, bool verbose) {
    return Communicator::_listenFor(comm, socketType, data, withHeader, [quitFlag]() { return !quitFlag; },
                                    timeoutResult, countIgnoreOther, countOther, retries, verbose);
}

bool Communicator::listenFor(Communication *comm, SocketType socketType, CommunicationData *data, bool withHeader,
                             const atomic<bool> &quitFlag, bool *timeoutResult, int countIgnoreOther, int countOther,
                             int retries, bool verbose) {
    return Communicator::_listenFor(comm, socketType, data, withHeader, [&quitFlag]() { return !quitFlag; },
                                    timeoutResult, countIgnoreOther, countOther, retries, verbose);
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

bool Communicator::_syphon(Communication *comm, SocketType socketType, MessageType &messageType,
                           CommunicationData *data, bool withHeader, const function<bool()> &notQuit, int retries,
                           bool verbose, int syphonRetries) {
    switch (messageType) {
        case MessageType::NOTHING: {
            return true;
        }
        case MessageType::STATUS:
        case MessageType::IMAGE:
        case MessageType::COORDINATE:
        case MessageType::BYTES:
        case MessageType::IMAGE_ENCODE: {
            int localSyphonRetries = (syphonRetries < 1) ? 1 : syphonRetries;
            while (notQuit() && localSyphonRetries > 0) {
                if (!comm->recvData(socketType, data, withHeader, true, true, retries, verbose)) {
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

bool Communicator::_listen(Communication *comm, SocketType socketType, MessageType &messageType,
                           DataCollection &_dataCollection, bool withHeader, const function<bool()> &notQuit,
                           int retries, bool verbose) {
    // cout << "Entering _listen function..." << endl;
    if (!comm->recvMessageType(socketType, messageType, withHeader, true, retries, verbose)) {
        if (Communicator::isReceiveErrorOk(comm->getErrorCode(), &messageType, true)) {
            // cout << "Received NOTHING" << endl;
            return true;
        }
        (*cerror) << "Error when receiving message type... setting \"quit\"" << endl;
        messageType = MessageType::STATUS;
        auto *status = (StatusData *) _dataCollection.get(messageType);
        status->setData(QUIT_MESSAGE);
        // cout << "Received NOTHING, set quit!" << endl;
        return true;
    }
    // cout << "Received messageType: " << messageTypeToString(messageType) << endl;

    CommunicationData *data = _dataCollection.get(messageType);
    MessageType receivedMessageType = messageType;
    if (!Communicator::_syphon(comm, socketType, messageType, data, withHeader, notQuit, retries, verbose)) {
        if (notQuit()) {
            (*cerror) << "Error when syphoning data " << messageTypeToString(receivedMessageType)
                      << "... setting \"quit\"" << endl;
        }
        messageType = MessageType::STATUS;
        auto *status = (StatusData *) _dataCollection.get(messageType);
        status->setData(QUIT_MESSAGE);
    }
    return true;
}

bool Communicator::_listenFor(Communication *comm, SocketType socketType, CommunicationData *data,
                              bool withHeader, const function<bool()> &notQuit, bool *timeoutResult,
                              int countIgnoreOther, int countIgnore, int retries, bool verbose) {
    assert(data != nullptr);
    MessageType messageType;
    while (notQuit()) {
        if (!comm->recvMessageType(socketType, messageType, withHeader, true, retries, verbose)) {
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
            if (!Communicator::_syphon(comm, socketType, messageType, syphonData, withHeader, notQuit, retries,
                                       verbose)) {
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
                if (timeoutResult != nullptr) {
                    *timeoutResult = true;
                }
                return false;
            }
        } else {
            break;
        }
    }
    if (!notQuit()) {
        return false;
    }
    while (notQuit()) {
        if (!Communicator::_syphon(comm, socketType, messageType, data, withHeader, notQuit, retries, verbose)) {
            if (comm->getErrorCode() < 0) {
                continue;
            }
            return false;
        }
        break;
    }
    return notQuit();
}
