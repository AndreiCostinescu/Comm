//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 12.03.2021.
//

#include <comm/socket/Socket.h>
#include <comm/socket/utils.h>
#include <cassert>
#include <csignal>
#include <iostream>
#include <thread>

using namespace comm;
using namespace std;

void printSocketDetails(SOCKET s) {
    SocketAddress tmpPartner{}, tmpLocal{};
    SocketAddressLength tmpLength = sizeof(SocketAddress);
    getpeername(s, (sockaddr *) (&tmpPartner), &tmpLength);
    getsockname(s, (sockaddr *) (&tmpLocal), &tmpLength);
    cout << "Socket partner: " + getStringAddress(tmpPartner) + "; local addr.: " + getStringAddress(tmpLocal) + "\n";
}

Socket::Socket(SocketType protocol, SocketPartner *partner, int myPort, int sendTimeout, int recvTimeout) : Socket() {
    if (!socketsStarted) {
        startupSockets();
        socketsStarted = true;
    }
    this->initialize(protocol, partner, myPort, sendTimeout, recvTimeout);
}

Socket::Socket(SocketType protocol, const SocketPartner &partner, int myPort, int sendTimeout, int recvTimeout) :
        Socket() {
    if (!socketsStarted) {
        startupSockets();
        socketsStarted = true;
    }
    this->initialize(protocol, partner, myPort, sendTimeout, recvTimeout);
}

Socket::~Socket() {
    this->cleanup(true);
}

SOCKET Socket::getSocket() const {
    return this->socket;
}

void Socket::cleanup(bool withBuffers) {
    this->close();
    if (this->deletePartner) {
        delete this->partner;
    }
    this->partner = nullptr;
    delete this->myself;
    this->myself = nullptr;
    this->isCopy = false;
    this->initialized = false;
    if (withBuffers) {
        delete this->sendBuffer;
        delete this->recvBuffer;
        this->sendBuffer = nullptr;
        this->recvBuffer = nullptr;
    }
}

void Socket::close() {
    if (!this->isCopy) {
#if defined(_WIN32) || defined(_WIN64)
        closesocket(this->socket);
#else
        ::close(this->socket);
#endif
    }
    this->socket = INVALID_SOCKET;
}

Socket *Socket::copy() const {
    auto copy = new Socket();
    assert (copy->socket == INVALID_SOCKET);
    copy->protocol = this->protocol;
    copy->socket = this->socket;

    copy->partner = (this->partner != nullptr) ? this->partner->copy() : nullptr;
    copy->myself = (this->myself != nullptr) ? this->myself->copy() : nullptr;

    copy->sendTimeout = this->sendTimeout;
    copy->recvTimeout = this->recvTimeout;

    copy->recvAddress = this->recvAddress;

    copy->sendBuffer = (this->sendBuffer == nullptr) ? nullptr : this->sendBuffer->copy();
    copy->recvBuffer = (this->recvBuffer == nullptr) ? nullptr : this->recvBuffer->copy();

    copy->deletePartner = this->deletePartner;
    copy->isCopy = true;
    copy->initialized = true;
    return copy;
}

bool Socket::initialize() {
    this->createSocket();

    this->initMyself();

    if (this->partner != nullptr) {
        if (this->protocol == SocketType::TCP) {
            // cout << "Socket Initialization: connecting tcp socket..." << endl;
            if (::connect(this->socket, (struct sockaddr *) this->partner->getPartner(), sizeof(SocketAddress)) < 0) {
                (*cerror) << "Connection failed due to port and ip problems" << endl;
                throw runtime_error("Connection failed due to port and ip problems!");
            }
            cout << "Socket Initialization: connected tcp socket to " << this->partner->getPartnerString() << endl;
        }
    }

    Socket::_setSocketTimeouts(this->socket, this->sendTimeout, this->recvTimeout);

    printSocketDetails(this->socket);
    this->initialized = true;
    return true;
}

bool Socket::initialize(SocketType _protocol, SocketPartner *_partner, int _myPort, int _sendTimeout,
                        int _recvTimeout) {
    this->cleanup();
    this->protocol = _protocol;
    this->partner = _partner;
    if (_myPort < 0 || _myPort >= (1u << 16u)) {
        _myPort = 0;
    }
    // this->myself = new SocketPartner("127.0.0.1", _myPort, false);
    this->myself = new SocketPartner("0.0.0.0", _myPort, false);
    this->sendTimeout = _sendTimeout;
    this->recvTimeout = _recvTimeout;
    this->isCopy = false;
    this->initialized = false;
    this->deletePartner = false;
    if (this->sendBuffer == nullptr) {
        this->sendBuffer = new Buffer(CLIENT_MAX_MESSAGE_BYTES + 4);
    }
    if (this->recvBuffer == nullptr) {
        this->recvBuffer = new Buffer(CLIENT_MAX_MESSAGE_BYTES + 4);
    }
    return this->initialize();
}

bool Socket::initialize(SocketType _protocol, const SocketPartner &_partner, int _myPort, int _sendTimeout,
                        int _recvTimeout) {
    return this->initialize(_protocol, new SocketPartner(_partner.getIP(), _partner.getPort(), _partner.getOverwrite()),
                            _myPort, _sendTimeout, _recvTimeout);
}

void Socket::setSocketTimeouts(int _sendTimeout, int _recvTimeout, bool modifySocket) {
    if (_sendTimeout > 0) {
        this->sendTimeout = _sendTimeout;
    }
    if (_recvTimeout > 0) {
        this->recvTimeout = _recvTimeout;
    }
    if (modifySocket && this->initialized) {
        Socket::_setSocketTimeouts(this->socket, this->sendTimeout, this->recvTimeout);
    }
}

void Socket::setOverwritePartner(bool overwrite) {
    if (this->isCopy) {
        (*cerror) << "Can not change the partner of a copy-socket!" << endl;
        return;
    }
    if (this->partner == nullptr) {
        this->partner = new SocketPartner();
    }
    this->partner->setOverwrite(overwrite);
}

void Socket::setPartner(const string &partnerIP, int partnerPort, bool overwrite) {
    if (this->isCopy) {
        (*cerror) << "Can not change the partner of a copy-socket!" << endl;
        return;
    }
    if (this->partner == nullptr) {
        this->partner = new SocketPartner();
    }
    this->partner->setPartner(partnerIP, partnerPort);
    this->partner->setOverwrite(overwrite);
}

void Socket::setPartner(SocketAddress _partner, bool overwrite) {
    if (this->isCopy) {
        (*cerror) << "Can not change the partner of a copy-socket!" << endl;
        return;
    }
    if (this->partner == nullptr) {
        this->partner = new SocketPartner(_partner, overwrite);
    } else {
        this->partner->setPartner(_partner);
        this->partner->setOverwrite(overwrite);
    }
}

void Socket::setPartner(SocketPartner *_partner, bool overwrite) {
    if (this->isCopy) {
        (*cerror) << "Can not change the partner of a copy-socket!" << endl;
        return;
    }
    this->partner = _partner;
    this->partner->setOverwrite(overwrite);
}

SocketPartner *&Socket::getPartner() {
    return this->partner;
}

SocketPartner *&Socket::getMyself() {
    return this->myself;
}

bool Socket::isInitialized() const {
    return this->initialized;
}

void Socket::accept(Socket *&acceptSocket, bool verbose) const {
    delete acceptSocket;
    acceptSocket = new Socket(SocketPartner(false, true));
    if (verbose) {
        cout << "Socket Initialization: accepting tcp connection... " << acceptSocket->partner->getPartnerString()
             << endl;
    }
    acceptSocket->socket = ::accept(this->socket, (struct sockaddr *) acceptSocket->getPartner()->getPartner(),
                                    &(acceptSocket->getPartner()->getPartnerSize()));
    if (verbose) {
        cout << "Found connection..." << endl;
    }
    if (acceptSocket->socket < 0) {
        (*cerror) << "ERROR on accept" << endl;
        throw runtime_error("Error on accept socket...");
    }
    acceptSocket->initMyself(false);
    if (verbose) {
        cout << "Socket Initialization: accepted tcp connection from " << acceptSocket->partner->getPartnerString()
             << endl;
    }
    printSocketDetails(acceptSocket->socket);

    Socket::_setSocketBufferSizes(acceptSocket->socket);
    acceptSocket->initialized = true;
}

bool Socket::sendBytes(const char *buffer, uint64_t bufferLength, int &errorCode, SerializationHeader *header,
                       int retries, bool keepForNextSend, bool verbose) {
    return this->_sendBytes(buffer, bufferLength, errorCode, header, retries, keepForNextSend, verbose);
}

bool Socket::sendBytes(Buffer &buffer, int &errorCode, SerializationHeader *header, int retries, bool keepForNextSend,
                       bool verbose) {
    return this->_sendBytes(buffer.getBuffer(), buffer.getBufferContentSize(), errorCode, header, retries,
                            keepForNextSend, verbose);
}

bool Socket::receiveBytes(char *&buffer, uint64_t &bufferLength, uint64_t expectedLength, int &errorCode,
                          SerializationHeader *expectedHeader, int retries, bool expectingData, bool verbose) {
    prepareBuffer(buffer, bufferLength, expectedLength);
    return this->_receiveBytes(buffer, expectedLength, errorCode, expectedHeader, retries, expectingData, verbose);
}

bool Socket::receiveBytes(Buffer &buffer, int &errorCode, SerializationHeader *expectedHeader, int retries,
                          bool expectingData, bool verbose) {
    return this->_receiveBytes(buffer.getBuffer(), buffer.getBufferContentSize(), errorCode, expectedHeader, retries,
                               expectingData, verbose);
}

void Socket::_setSocketBufferSizes(SOCKET socket) {
    assert(setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char *) &SOCKET_BUFFER_RECV_SIZE,
                      sizeof(SOCKET_BUFFER_SEND_SIZE)) == 0);
    assert(setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char *) &SOCKET_BUFFER_SEND_SIZE,
                      sizeof(SOCKET_BUFFER_SEND_SIZE)) == 0);
}

void Socket::_setSocketTimeouts(SOCKET socket, int sendTimeout, int recvTimeout) {
    if (sendTimeout > 0) {
#if defined(_WIN32) || defined(_WIN64)
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &sendTimeout, sizeof(int));
#else
        struct timeval tv{};
        tv.tv_usec = 1000 * sendTimeout;
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *) &tv, sizeof(struct timeval));
#endif
    }
    if (recvTimeout > 0) {
#if defined(_WIN32) || defined(_WIN64)
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &recvTimeout, sizeof(int));
#else
        struct timeval tv{};
        tv.tv_usec = 1000 * recvTimeout;
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));
#endif
    }
}

Socket::Socket() : protocol(SocketType::UDP), socket(INVALID_SOCKET), partner(nullptr), myself(nullptr),
                   sendTimeout(-1), recvTimeout(-1), isCopy(false), initialized(false), deletePartner(true),
                   recvAddress(), recvAddressLength(sizeof(this->recvAddress)) {
    this->sendBuffer = new Buffer(CLIENT_MAX_MESSAGE_BYTES + 4);
    this->recvBuffer = new Buffer(CLIENT_MAX_MESSAGE_BYTES + 4);
}

Socket::Socket(const SocketPartner &partner) : Socket() {
    this->protocol = SocketType::TCP;
    this->partner = new SocketPartner(partner.getIP(), partner.getPort(), partner.getOverwrite());
}

void Socket::createSocket() {
    if (this->socket != INVALID_SOCKET) {
        this->close();
    }

    switch (this->protocol) {
        case SocketType::UDP:
        case SocketType::UDP_HEADER: {
            this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
            break;
        }
        case SocketType::TCP: {
            this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
            break;
        }
        default : {
            throw runtime_error("Unknown protocol: " + to_string(this->protocol));
        }
    }

    if (this->socket == INVALID_SOCKET) {
        (*cerror) << "Socket " << this->socket << " not created" << endl;
        throw runtime_error("Socket not created!");
    }
    Socket::_setSocketBufferSizes(this->socket);
}

void Socket::initMyself(bool withBind, bool verbose) {
    if (withBind) {
        // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
        // bind() passes file descriptor, the address structure, and the length of the address structure
        // This bind() call will bind the socket to the current IP address on port 'portNumber'
        if (verbose) {
            cout << "Socket Initialization: binding socket to local " << this->myself->getPartnerString() << "..."
                 << endl;
        }
        if (::bind(this->socket, (struct sockaddr *) this->myself->getPartner(), sizeof(SocketAddress)) < 0) {
            cout << this_thread::get_id() << ": Error " << getLastErrorString() << endl;
            (*cerror) << this_thread::get_id() << ": ERROR on binding" << endl;
            throw runtime_error("ERROR binding server socket");
        }
    }
    if (this->myself == nullptr || this->myself->getPort() == 0) {
        SocketAddress newAddress;
        SocketAddressLength newAddressLength = sizeof(SocketAddress);
        getsockname(this->socket, (sockaddr *) (&newAddress), &newAddressLength);
        if (this->myself == nullptr) {
            this->myself = new SocketPartner(newAddress);
        } else {
            this->myself->setPartner(newAddress);
        }
    }
    if (verbose) {
        cout << "Socket Initialization: bound socket to local " << this->myself->getPartnerString() << "!" << endl;
    }
}

bool Socket::performSend(const char *buffer, int &localBytesSent, int &errorCode, SerializationHeader *header,
                         const int sendSize, const int sentBytes, const char sendIteration, const bool keepForNextSend,
                         const bool verbose) {
    bool withHeader = (header != nullptr || this->protocol == SocketType::UDP_HEADER);
    int startingPosition = this->sendBuffer->getBufferContentSize();
    if (startingPosition != 0 || keepForNextSend || withHeader) {
        this->sendBuffer->setBufferContentSize(startingPosition + withHeader * 4 + sendSize);
        if (withHeader) {
            if (header != nullptr) {
                this->sendBuffer->setChar((char) header->getSerializationIteration(), startingPosition);
            } else {
                this->sendBuffer->setChar(0, startingPosition);
            }
            this->sendBuffer->setChar(sendIteration, startingPosition + 1);
            this->sendBuffer->setShort((short) sendSize, startingPosition + 2);
            this->sendBuffer->setData(buffer + sentBytes, sendSize, startingPosition + 4);
        } else {
            this->sendBuffer->setData(buffer + sentBytes, sendSize, startingPosition);
        }
    } else {
        this->sendBuffer->setConstReferenceToData(buffer + sentBytes, sendSize);
    }

    if (!keepForNextSend) {
        setErrnoZero();
        if (verbose) {
            cout << "Pre send: already sentBytes = " << sentBytes << "; sending " << sendSize << "B to " << this->socket
                 << endl;
        }
        switch (this->protocol) {
            case SocketType::UDP:
            case SocketType::UDP_HEADER: {
                SocketAddress *&toAddress = this->partner->getPartner();
                assert(toAddress != nullptr);
                if (toAddress == nullptr) {
                    (*cerror) << "UDP send partner is null!" << endl;
                    errorCode = 0;
                    return false;
                }
                localBytesSent = sendto(this->socket, this->sendBuffer->getConstBuffer(),
                                        (int) this->sendBuffer->getBufferContentSize(),
                                        0, (struct sockaddr *) (toAddress), sizeof(*toAddress));
                break;
            }
            case SocketType::TCP: {
                localBytesSent = send(this->socket, this->sendBuffer->getConstBuffer(),
                                      (int) this->sendBuffer->getBufferContentSize(), 0);
                break;
            }
            default : {
                throw runtime_error("Unknown protocol: " + to_string(this->protocol));
            }
        }
        if (verbose) {
            cout << "Post send: already sent = " << sentBytes << "B; managed to send localBytesSent = "
                 << localBytesSent << endl;
            /*
            for (int i = 0; i < localBytesSent; i++) {
                cout << (int) ((unsigned char) buffer[i + sentBytes]) << ", ";
            }
            cout << endl;
            //*/
        }
        if (localBytesSent == -1) {
            cout << "Error when sending! " << getLastErrorString() << endl;
            // return true not false to let interpretSendResult print the error code!
            return true;
        }
        this->sendBuffer->setBufferContentSize(0);
        localBytesSent -= startingPosition;
        if (withHeader) {
            if (localBytesSent < 4) {
                cout << "The assertion in Socket::performSend (localBytesSent == 0 || localBytesSent == -1) will fail: "
                     << "localBytesSent = " << localBytesSent << endl;
                assert(localBytesSent == 0 || localBytesSent == -1);
            } else {
                localBytesSent -= 4;
            }
        }
    } else {
        localBytesSent = sendSize;
    }

    return true;
}

bool Socket::interpretSendResult(int &errorCode, int &localBytesSent, int &retries, char &sendIteration,
                                 const bool verbose) {
    if (localBytesSent < 0) {
        errorCode = getLastError();
        if (errorCode == SOCKET_CONNRESET || errorCode == SOCKET_SHUTDOWN || errorCode == SOCKET_PIPE) {
            // Socket closed
            errorCode = 0;
            return false;
        } else if (errorCode == SOCKET_TIMEOUT || errorCode == SOCKET_AGAIN || errorCode == SOCKET_WOULDBLOCK) {
            // send timeout
            errorCode = -1;
            localBytesSent = 0;
        } else {
            if (errorCode != 0) {
                (*cerror) << "SendBytes: " << errorCode << "; " << getLastErrorString(errorCode) << endl;
            }
            return false;
        }
    }
    // don't write as "else if" because localBytesSent gets modified above!
    if (localBytesSent == 0) {
        if (verbose) {
            cout << "Sent 0 bytes???" << endl;
        }
        if (retries == 0) {
            return false;
        }
        if (retries > 0) {
            retries--;
        }
    } else {
        sendIteration++;
    }
    return true;
}

bool Socket::_sendBytes(const char *buffer, uint64_t bufferLength, int &errorCode, SerializationHeader *header,
                        int retries, bool keepForNextSend, bool verbose) {
    assert(this->sendBuffer != nullptr);
    if (!this->isInitialized()) {
        (*cerror) << "Can not send with an uninitialized socket!" << endl;
        return false;
    }
    signal(SIGPIPE, signalHandler);
    if (verbose) {
        cout << this_thread::get_id() << ": Entering _sendBytes function with buffer length = " << bufferLength
             << " and sendPartner " << ((this->partner == nullptr) ? "any" : this->partner->getStringAddress()) << "!"
             << endl;
    }
    int sentBytes = 0, localBytesSent;
    char sendIteration = 0;
    uint64_t maxSendPerRound = CLIENT_MAX_MESSAGE_BYTES;
    while (sentBytes < bufferLength) {
        int sendSize = (int) min(maxSendPerRound, bufferLength - sentBytes);
        if (!this->performSend(buffer, localBytesSent, errorCode, header, sendSize, sentBytes, sendIteration,
                               keepForNextSend, verbose)) {
            return false;
        }

        if (!this->interpretSendResult(errorCode, localBytesSent, retries, sendIteration, verbose)) {
            return false;
        }
        sentBytes += localBytesSent;
    }
    if (verbose) {
        cout << this_thread::get_id() << ": Exiting _sendBytes function having sent = " << bufferLength << "B to "
             << ((this->partner == nullptr) ? "any" : this->partner->getStringAddress()) << "!" << endl;
    }
    return true;
}

bool Socket::checkCorrectReceivePartner(bool &overwritePartner, const int receiveIteration) {
    if (this->protocol == SocketType::TCP) {
        return true;
    }

    assert (overwritePartner || this->partner != nullptr);
    if (overwritePartner) {
        assert (receiveIteration == 0);
        // if overwritePartner is desired, choose first partner and do not overwrite until recv is finished!
        if (this->partner == nullptr) {
            this->partner = new SocketPartner(this->recvAddress);
        } else {
            this->partner->setPartner(this->recvAddress);
        }
        overwritePartner = false;
    } else if (!comparePartners(this->recvAddress, *(this->partner->getPartner()))) {
        // do not consider received data from other partners than the saved partner
        return false;
    }
    return true;
}

bool Socket::performReceive(char *buffer, int &localReceivedBytes, bool &overwritePartner, bool &recvFromCorrectPartner,
                            SerializationHeader *expectedHeader, int receiveSize, const uint64_t receivedBytes,
                            const int receiveIteration, const bool expectingData, const bool verbose) {
    setErrnoZero();
    recvFromCorrectPartner = true;  // needed because there are return paths before recomputing the value :)
    if (verbose) {
        cout << "Pre receive... already recvBytes = " << receivedBytes << endl;
    }

    bool localVerbose = verbose;
    bool withHeader = (expectedHeader != nullptr || this->protocol == SocketType::UDP_HEADER);
    int dataStart = (withHeader) ? 4 : 0;
    int receiveAmount = 0;
    receiveSize += dataStart;

    if (localVerbose && receiveSize > 100 && receiveSize != 65500 + dataStart) {
        cout << "WEIRD RECEIVE_SIZE = " << receiveSize << endl;
    }

    localReceivedBytes = 0;
    recvFromCorrectPartner = true;
    if (withHeader && receiveIteration == 0 && !this->recvBuffer->empty()) {
        cout << "USING THE LOCAL RECEIVED BYTES VALUE FROM THE PREVIOUS ITERATION!" << endl;
        receiveAmount = (int) this->recvBuffer->getBufferContentSize();
        localReceivedBytes += receiveAmount;
        assert (receiveAmount > 0);
    }
    // cout << "LocalReceivedBytes before switch: " << localReceivedBytes << endl;
    int serializationIteration = -1, sendIteration = -1;
    switch (this->protocol) {
        case SocketType::UDP:
        case SocketType::UDP_HEADER: {
            if (localReceivedBytes > 0) {
                // udp respects the boundaries of messages -> what we previously received was a (full) package/message!
                receiveSize = 0;
                if (withHeader && localReceivedBytes >= 4) {
                    serializationIteration = (int) ((unsigned char) this->recvBuffer->getChar(0));
                    sendIteration = (int) ((unsigned char) this->recvBuffer->getChar(1));
                }
                break;
            }
            assert (this->recvBuffer->empty());
            assert (this->recvBuffer->getBuffer() != nullptr);

            bool doneReceive = false, startSyphon = false, serializationError = false;
            int localSerializationIteration = -1, localSendIteration = -1, localReceiveAmount;
            do {
                // receive
                this->recvAddressLength = sizeof(this->recvAddress);
                receiveAmount = recvfrom(this->socket, this->recvBuffer->getBuffer(), this->recvBuffer->getBufferSize(),
                                         0, (struct sockaddr *) (&this->recvAddress), &this->recvAddressLength);
                if (receiveAmount <= 0) {
                    int errorCode = getLastError();
                    if (!(errorCode == 0 || errorCode == SOCKET_TIMEOUT ||
                          errorCode == SOCKET_AGAIN || errorCode == SOCKET_WOULDBLOCK)) {
                        cout << "BREAK BECAUSE OF ERROR (UDP): receiveAmount = " << receiveAmount << "; "
                             << getLastError() << "; " << getLastErrorString() << endl;
                    }
                    localReceivedBytes = receiveAmount;
                    this->recvBuffer->setBufferContentSize(0);
                    return true;
                }
                recvFromCorrectPartner = this->checkCorrectReceivePartner(overwritePartner, receiveIteration);
                this->recvBuffer->setBufferContentSize(receiveAmount);
                if (!startSyphon && withHeader && receiveAmount >= 4) {
                    serializationIteration = (int) ((unsigned char) this->recvBuffer->getChar(0));
                    sendIteration = (int) ((unsigned char) this->recvBuffer->getChar(1));
                }

                if (startSyphon) {
                    assert (withHeader && expectedHeader != nullptr);
                    localSerializationIteration = (int) ((unsigned char) this->recvBuffer->getChar(0));
                    localSendIteration = (int) ((unsigned char) this->recvBuffer->getChar(1));
                    cout << "localSerializationIteration: " << localSerializationIteration << endl;
                    cout << "localSendIteration: " << localSendIteration << endl;
                    doneReceive = (recvFromCorrectPartner && localSerializationIteration == 0 &&
                                   localSerializationIteration == localSendIteration);
                } else if (recvFromCorrectPartner && expectedHeader != nullptr &&
                           expectedHeader->getSyphonUntilFirstMessage() &&
                           (serializationIteration != expectedHeader->getSerializationIteration() ||
                            sendIteration != receiveIteration)) {
                    cout << "serializationIteration: " << serializationIteration << ", expected "
                         << (int) expectedHeader->getSerializationIteration() << endl;
                    cout << "sendIteration: " << sendIteration << ", expected "
                         << receiveIteration << endl;
                    doneReceive = (serializationIteration == 0 && serializationIteration == sendIteration);
                    startSyphon = !doneReceive;
                } else {
                    doneReceive = true;
                }
            } while (!doneReceive);

            localReceivedBytes += receiveAmount;
            receiveSize = 0;
            break;
        }
        case SocketType::TCP: {
            // tcp does not respect the boundaries of messages -> read until we get the receiveSize!
            receiveSize -= localReceivedBytes;
            int localRetries = 10, iterationCounter = 0;
            while (receiveSize > 0 && localRetries > 0) {
                // cout << "Waiting to receive " << receiveSize << " bytes" << endl;
                receiveAmount = recv(this->socket, this->recvBuffer->getBuffer() + localReceivedBytes, receiveSize, 0);
                if (localVerbose && receiveAmount > 0 && receiveAmount != receiveSize) {
                    cout << "Waiting to receive " << receiveSize << " bytes, received " << receiveAmount << endl;
                }
                // Determine if timeout or error!
                if (receiveAmount <= 0) {
                    int errorCode = getLastError();
                    if (receiveAmount < 0 && (errorCode == 0 || errorCode == SOCKET_TIMEOUT ||
                                              errorCode == SOCKET_AGAIN || errorCode == SOCKET_WOULDBLOCK)) {
                        // timeout
                        if (!expectingData && iterationCounter == 0) {
                            // honestly received nothing
                            localReceivedBytes = receiveAmount;
                            return true;
                        }
                        if (localVerbose) {
                            cout << "TIMEOUT IN TCP RECEIVE!!!" << endl;
                        }
                        localRetries--;
                    } else {
                        cout << "BREAK BECAUSE OF (TCP) ERROR: receive amount = " << receiveAmount << "; errorCode: "
                             << errorCode << "; " << getLastErrorString(errorCode) << endl;
                        localReceivedBytes = receiveAmount;
                        return true;
                    }
                } else {
                    localReceivedBytes += receiveAmount;
                    receiveSize -= receiveAmount;
                    this->recvBuffer->setBufferContentSize(localReceivedBytes);
                }
                if (withHeader && expectedHeader != nullptr && localReceivedBytes > 0) {
                    if (expectedHeader->getSerializationIteration() != this->recvBuffer->getChar(0)) {
                        cout << "TCP Serialization wrong: expected "
                             << (int) (unsigned char) expectedHeader->getSerializationIteration() << " vs. received "
                             << (int) (unsigned char) this->recvBuffer->getChar(0) << endl;
                        localVerbose = true;
                    }
                }
                iterationCounter++;
            }
            if (localRetries == 0) {
                cout << "Couldn't receive data from TCP (timeout threshold reached); There will be an error..." << endl;
                // latency; network issues...
                localReceivedBytes = -3;
                return false;
            }
            if (withHeader && localReceivedBytes > 4) {
                serializationIteration = (int) ((unsigned char) this->recvBuffer->getChar(0));
                sendIteration = (int) ((unsigned char) this->recvBuffer->getChar(1));
            }
            break;
        }
        default : {
            throw runtime_error("Unknown protocol: " + to_string(this->protocol));
        }
    }
    int remainingBufferBytes = -receiveSize;
    assert ((receiveSize <= 0 || remainingBufferBytes >= 0));
    assert ((remainingBufferBytes == 0 || this->protocol == SocketType::TCP));

    if (!recvFromCorrectPartner || receiveAmount <= 0) {
        if (receiveAmount <= 0) {
            localReceivedBytes = receiveAmount;
        }
        this->recvBuffer->setBufferContentSize(0);
        return true;
    }
    assert (remainingBufferBytes >= 0);
    assert (localReceivedBytes > 0);

    if (withHeader) {
        if (localReceivedBytes < 4) {
            cout << "Wrong protocol for this socket!!!" << endl;
            localReceivedBytes = -1;
            return false;
        }
        this->recvBuffer->setBufferContentSize(localReceivedBytes);
        localReceivedBytes -= 4;
        if (localVerbose) {
            cout << "Should have received " << (unsigned short) this->recvBuffer->getShort(2)
                 << ", received " << localReceivedBytes << endl;
            cout << "First bytes: ";
            for (int i = 0; i < min(localReceivedBytes, 20) + 4; i++) {
                cout << (int) ((unsigned char) this->recvBuffer->getChar(i)) << ", ";
            }
            if (localReceivedBytes > 20) {
                cout << "...";
            }
            cout << endl;
        }

        bool syphonedWronglySerializedData = (expectedHeader != nullptr &&
                                              expectedHeader->getSyphonUntilFirstMessage());
        // check serialization iteration
        if (expectedHeader != nullptr && serializationIteration != expectedHeader->getSerializationIteration()) {
            // received different serialization state... check if the partner is the same
            // interrupt receive! and don't reset the recvBuffer to keep data for next recv!
            cout << "Serialization Iteration check failed: got " << (int) serializationIteration
                 << "; localReceivedBytes from receive call = " << localReceivedBytes + 4 << "; receiveAmount = "
                 << receiveAmount << endl;
            cout << "Trailing bytes: ";
            for (int i = 0; i < min(localReceivedBytes, 20) + 4; i++) {
                cout << (int) ((unsigned char) this->recvBuffer->getChar(i)) << ", ";
            }
            if (localReceivedBytes > 20) {
                cout << "...";
            }
            cout << endl;
            localReceivedBytes = -2;
            setErrnoZero();  // <- to ensure that this will be interpreted as a timeout :)
            // keep buffer contents if the wrong data has been syphoned or if it's the start of another message!
            if (!syphonedWronglySerializedData && serializationIteration != 0) {
                this->recvBuffer->setBufferContentSize(0);
            }
            return true;
        }

        // check send iteration
        if (receiveIteration != sendIteration) {
            cout << "Send Iteration check failed: got " << sendIteration << ", expected " << receiveIteration << endl;
            // keep buffer contents if the wrong data has been syphoned or if it's the start of another message!
            if (!syphonedWronglySerializedData && (serializationIteration != 0 || sendIteration != 0)) {
                this->recvBuffer->setBufferContentSize(0);
            }
            if (receiveIteration == 0) {
                assert (sendIteration > 0);
                // wrong data, ignore, pretend like nothing was received (like recv from wrong partner)
                recvFromCorrectPartner = false;
            } else {
                // (receiveIteration != 0 || sendIteration == 0) - interrupt receive!
                localReceivedBytes = -2;
                setErrnoZero();  // <- to ensure that this will be interpreted as a timeout :)
            }
            return true;
        }
    }
    if (recvFromCorrectPartner) {
        if (localVerbose) {
            cout << "Received data: ";
            const char *receivedBufferData = this->recvBuffer->getBuffer() + dataStart;
            for (int i = 0; i < min(localReceivedBytes, 50); i++) {
                cout << (int) ((unsigned char) receivedBufferData[i]) << " ";
            }
            cout << endl;
        }
        memcpy(buffer + receivedBytes, this->recvBuffer->getBuffer() + dataStart, localReceivedBytes);
        if (remainingBufferBytes > 0) {
            cout << "HELP: COPYING TCP DATA!" << endl;
            // can happen in TCP if we read more than expected...
            memcpy(this->recvBuffer->getBuffer(), this->recvBuffer->getBuffer() + localReceivedBytes,
                   remainingBufferBytes);
        }
    }
    if (remainingBufferBytes < 0) {
        cout << "ERROR, the next call will throw a bad alloc!!!" << endl;
    }
    this->recvBuffer->setBufferContentSize(remainingBufferBytes);

    if (localVerbose) {
        cout << "Post receive... managed to receive localReceivedBytes = " << localReceivedBytes
             << " in addition to the received " << receivedBytes << "!" << endl;
        /*
        for (int i = 0; i < localReceivedBytes; i++) {
            cout << (int) ((unsigned char) buffer[i + receivedBytes]) << ", ";
        }
        cout << endl;
        //*/
    }
    return true;
}

bool Socket::interpretReceiveResult(int &errorCode, int &localReceivedBytes, bool &recvFirstMessage,
                                    const bool recvFromCorrectPartner, const bool verbose) {
    if (localReceivedBytes > 0) {
        if (recvFromCorrectPartner) {
            if (verbose && !recvFirstMessage) {
                cout << "Received first data!" << endl;
            }
            recvFirstMessage = true;
        } else {
            // simulate receiving nothing!
            localReceivedBytes = -1;
            errorCode = getLastError();
        }
        if (!checkErrno(errorCode, "Socket::_receiveBytes - socket recv sth")) {
            assert (false);
        }
    } else if (localReceivedBytes == 0) {
        // socket closed
        if (!checkErrno(errorCode, "Socket::_receiveBytes - socket closed")) {
            assert (false);
        }
        return false;
    } else {
        if (localReceivedBytes > -1 || localReceivedBytes < -3) {
            cout << "The next assertion will fail: localReceivedBytes = " << localReceivedBytes << endl;
        }
        assert (localReceivedBytes <= -1 && localReceivedBytes >= -3);
        if (localReceivedBytes == -2 || localReceivedBytes == -3) {
            errorCode = localReceivedBytes;
            return false;
        }
        // only set the errorCode here, because when there is an error, localReceivedBytes is set to -1!
        errorCode = getLastError();
        assert (errorCode >= 0);
        if (verbose) {
            cout << "ReceiveBytes: " << errorCode << "; " << getLastErrorString(errorCode) << endl;
        }

        if (errorCode == SOCKET_CONNRESET || errorCode == SOCKET_SHUTDOWN || errorCode == SOCKET_PIPE) {
            // Socket closed
            errorCode = 0;
            return false;
        } else if (errorCode == SOCKET_TIMEOUT || errorCode == SOCKET_AGAIN || errorCode == SOCKET_WOULDBLOCK) {
            // recv timeout
            errorCode = 0;
            localReceivedBytes = -1;
        } else if (errorCode != 0) {
            (*cerror) << "ReceiveBytes: " << errorCode << "; " << getLastErrorString(errorCode) << endl;
            return false;
        }
    }
    return true;
}

void Socket::setRetries(int &retries, bool &retry, const int maxRetries, const bool recvFromCorrectPartner,
                        const int localReceivedBytes, const bool verbose) {
    // retry receiving if received data from someone else or (if received nothing and there are retries left)
    retry = (!recvFromCorrectPartner) || ((localReceivedBytes == -1) && (retries != 0));
    if (verbose) {
        cout << "Retry = " << retry << "; retries = " << retries << endl;
    }
    if (retry && retries > 0) {
        if (recvFromCorrectPartner) {
            retries--;
        } else {
            retries = maxRetries;
        }
    }
}

bool Socket::_receiveBytes(char *buffer, uint64_t expectedLength, int &errorCode, SerializationHeader *expectedHeader,
                           int retries, bool expectingData, bool verbose) {
    assert(this->recvBuffer != nullptr);
    if (!this->isInitialized()) {
        (*cerror) << "Can not receive with an uninitialized socket!" << endl;
        return false;
    }
    signal(SIGPIPE, signalHandler);
    if (verbose) {
        cout << this_thread::get_id() << ": Entering _receiveBytes function with expected length = " << expectedLength
             << " and receivePartner = " << ((this->partner == nullptr) ? "any" : this->partner->getStringAddress())
             << "!" << endl;
    }
    int localReceivedBytes, receiveSize, maxRetries = retries;
    uint64_t receivedBytes = 0, maxPossibleReceiveBytes = CLIENT_MAX_MESSAGE_BYTES;
    errorCode = 0;
    bool recvFirstMessage = false, retry, recvFromCorrectPartner = false;
    bool overwritePartner = (this->partner == nullptr || !this->partner->isInitialized() ||
                             this->partner->getOverwrite());
    int receiveIteration = 0;
    while (expectedLength == 0 || receivedBytes < expectedLength) {
        if (expectedLength == 0) {
            receiveSize = (int) maxPossibleReceiveBytes;
        } else {
            receiveSize = (int) min(expectedLength - receivedBytes, maxPossibleReceiveBytes);
        }
        // wait to receive data from socket (and retry when timeout occurs)
        do {
            if (!checkErrno(errorCode, "Socket::_receiveBytes - begin do-while")) {
                assert (false);
            }
            if (!this->performReceive(buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner,
                                      expectedHeader, receiveSize, receivedBytes, receiveIteration, expectingData,
                                      verbose)) {
                return false;
            }
            if (!checkErrno(errorCode, "Socket::_receiveBytes - after actual receive")) {
                assert (false);
            }

            if (!this->interpretReceiveResult(errorCode, localReceivedBytes, recvFirstMessage, recvFromCorrectPartner,
                                              verbose)) {
                return false;
            }
            if (!checkErrno(errorCode, "Socket::_receiveBytes - after process localReceivedBytes")) {
                assert (false);
            }
            this->setRetries(retries, retry, maxRetries, recvFromCorrectPartner, localReceivedBytes, verbose);
        } while (!recvFirstMessage && retry);  // <- continue retry receiving until the first message has been received
        if (verbose && localReceivedBytes >= 0) {
            cout << "Total received bytes so far: " << receivedBytes + localReceivedBytes << endl;
        }

        assert(localReceivedBytes == -1 || localReceivedBytes > 0);
        if (localReceivedBytes == -1) {
            // set timeout error code
            errorCode = -1;
            return false;
        }
        receivedBytes += localReceivedBytes;
        receiveIteration += recvFirstMessage;
        if (expectedLength == 0) {
            break;
        }
    }
    if (!(expectedLength == 0 || receivedBytes == expectedLength)) {
        cout << "Next assertion will fail: expectedLength = " << expectedLength << " and receivedBytes = "
             << receivedBytes << endl;
        if (this->protocol == SocketType::UDP) {
            cout << "Maybe packets arrived in the wrong order on UDP?" << endl;
        }
    }
    assert(expectedLength == 0 || receivedBytes == expectedLength);
    if (verbose) {
        cout << this_thread::get_id() << ": Exiting _receiveBytes function having received = " << receivedBytes
             << "B from " << ((this->partner == nullptr) ? "any" : this->partner->getStringAddress()) << "!" << endl;
    }
    return true;
}
