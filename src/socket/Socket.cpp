//
// Created by ga78cat on 12.03.2021.
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

    copy->partner = this->partner->copy();
    copy->myself = this->myself->copy();

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
            cout << "Socket Initialization: connecting tcp socket..." << endl;
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
    cout << "Socket Initialization: accepting tcp connection... " << acceptSocket->partner->getPartnerString() << endl;
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
    cout << "Socket Initialization: accepted tcp connection from " << acceptSocket->partner->getPartnerString() << endl;
    printSocketDetails(acceptSocket->socket);

    Socket::_setSocketBufferSizes(acceptSocket->socket);
    acceptSocket->initialized = true;
}

bool Socket::sendBytes(const char *buffer, unsigned long long int bufferLength, int &errorCode,
                       SerializationHeader *header, int retries, bool verbose) {
    return this->_sendBytes(buffer, bufferLength, errorCode, header, retries, verbose);
}

bool Socket::sendBytes(Buffer &buffer, int &errorCode, SerializationHeader *header, int retries, bool verbose) {
    return this->_sendBytes(buffer.getBuffer(), buffer.getBufferContentSize(), errorCode, header, retries, verbose);
}

bool Socket::receiveBytes(char *&buffer, unsigned long long int &bufferLength, unsigned long long int expectedLength,
                          int &errorCode, int retries, bool verbose) {
    prepareBuffer(buffer, bufferLength, expectedLength);
    return this->_receiveBytes(buffer, expectedLength, errorCode, retries, verbose);
}

bool Socket::receiveBytes(Buffer &buffer, int &errorCode, int retries, bool verbose) {
    return this->_receiveBytes(buffer.getBuffer(), buffer.getBufferContentSize(), errorCode, retries, verbose);
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
                   recvAddress(), recvAddressLength(sizeof(this->recvAddress)), recvBuffer(nullptr) {
    this->sendBuffer = new Buffer(CLIENT_MAX_MESSAGE_BYTES + 4);
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
        case UDP:
        case UDP_HEADER: {
            this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
            break;
        }
        case TCP: {
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

void Socket::initMyself(bool withBind) {
    if (withBind) {
        // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
        // bind() passes file descriptor, the address structure, and the length of the address structure
        // This bind() call will bind the socket to the current IP address on port 'portNumber'
        cout << "Socket Initialization: binding socket to local " << this->myself->getPartnerString() << "..." << endl;
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
    cout << "Socket Initialization: bound socket to local " << this->myself->getPartnerString() << "!" << endl;
}

bool Socket::performSend(const char *buffer, int &localBytesSent, int &errorCode, SerializationHeader *header,
                         const int sendSize, const int sentBytes, const char sendIteration, const bool verbose) {
    if (header != nullptr || this->protocol == UDP_HEADER) {
        if (header != nullptr) {
            this->sendBuffer->setChar((char) header->getSerializationIteration(), 0);
        } else {
            this->sendBuffer->setChar((char) (sendIteration != 0), 0);
        }
        this->sendBuffer->setChar(sendIteration, 1);
        this->sendBuffer->setShort((short) sendSize, 2);
        this->sendBuffer->setData(buffer + sentBytes, sendSize, 4);
    } else {
        this->sendBuffer->setConstReferenceToData(buffer + sentBytes, sendSize);
    }
    setErrnoZero();
    switch (this->protocol) {
        case UDP:
        case UDP_HEADER: {
            SocketAddress *&toAddress = this->partner->getPartner();
            assert(toAddress != nullptr);
            if (toAddress == nullptr) {
                (*cerror) << "UDP send partner is null!" << endl;
                errorCode = 0;
                return false;
            }
            if (verbose) {
                cout << "Pre sendto: already sentBytes = " << sentBytes << endl;
            }
            localBytesSent = sendto(this->socket, this->sendBuffer->getConstBuffer(),
                                    (int) this->sendBuffer->getBufferContentSize(),
                                    0, (struct sockaddr *) (toAddress), sizeof(*toAddress));
            if (verbose) {
                cout << "Post sendto: managed to send (UDP) localBytesSent = " << localBytesSent << endl;
                /*
                for (int i = 0; i < localBytesSent; i++) {
                    cout << (int) buffer[i + sentBytes] << ", ";
                }
                cout << endl;
                //*/
            }
            break;
        }
        case TCP: {
            if (verbose) {
                cout << "Pre send: sending " << sendSize << "B to " << this->socket << endl;
            }
            localBytesSent = send(this->socket, this->sendBuffer->getConstBuffer(),
                                  (int) this->sendBuffer->getBufferContentSize(), 0);
            if (verbose) {
                cout << "Post send: managed to send (TCP) localBytesSent = " << localBytesSent << endl;
            }
            break;
        }
        default : {
            throw runtime_error("Unknown protocol: " + to_string(this->protocol));
        }
    }
    if ((header != nullptr || this->protocol == UDP_HEADER) && localBytesSent >= 4) {
        localBytesSent -= 4;
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

bool Socket::_sendBytes(const char *buffer, unsigned long long int bufferLength, int &errorCode,
                        SerializationHeader *header, int retries, bool verbose) {
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
    unsigned long long int maxSendPerRound = CLIENT_MAX_MESSAGE_BYTES;
    while (sentBytes < bufferLength) {
        int sendSize = (int) min(maxSendPerRound, bufferLength - sentBytes);
        if (!this->performSend(buffer, localBytesSent, errorCode, header, sendSize, sentBytes, sendIteration,
                               verbose)) {
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

bool Socket::checkCorrectReceivePartner(bool &overwritePartner, const bool recvFirstMessage) {
    assert (overwritePartner || this->partner != nullptr);
    if (overwritePartner) {
        assert (!recvFirstMessage);
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
                            const int receiveSize, const unsigned long long int receivedBytes,
                            const bool recvFirstMessage, const bool verbose) {
    setErrnoZero();
    recvFromCorrectPartner = true;
    switch (this->protocol) {
        case UDP: {
            if (verbose) {
                cout << "Pre recvfrom... already recvBytes = " << receivedBytes << endl;
            }
            this->recvAddressLength = sizeof(this->recvAddress);
            localReceivedBytes = recvfrom(this->socket, buffer + receivedBytes, receiveSize, 0,
                                          (struct sockaddr *) (&this->recvAddress), &this->recvAddressLength);
            if (localReceivedBytes >= 0) {
                recvFromCorrectPartner = this->checkCorrectReceivePartner(overwritePartner, recvFirstMessage);
            }
            if (verbose) {
                cout << "Post recvfrom... managed to receive localReceivedBytes = " << localReceivedBytes
                     << endl;
                /*
                for (int i = 0; i < localReceivedBytes; i++) {
                    cout << (int) buffer[i + receivedBytes] << ", ";
                }
                cout << endl;
                //*/
            }
            break;
        }
        case UDP_HEADER: {
            if (verbose) {
                cout << "Pre recvfrom... already recvBytes = " << receivedBytes << ", expecting " << 4 + receiveSize
                     << endl;
            }

            if (!recvFirstMessage && !this->recvBuffer->empty()) {
                localReceivedBytes = (int) this->recvBuffer->getBufferContentSize();
            } else {
                assert (this->recvBuffer->empty());
                assert (this->recvBuffer->getBuffer() != nullptr);
                this->recvAddressLength = sizeof(this->recvAddress);
                localReceivedBytes = recvfrom(this->socket, this->recvBuffer->getBuffer(), 4 + receiveSize, 0,
                                              (struct sockaddr *) (&this->recvAddress), &this->recvAddressLength);
            }

            if (localReceivedBytes >= 0) {
                if (localReceivedBytes < 4) {
                    cout << "Wrong protocol for this socket!!!" << endl;
                    localReceivedBytes = -1;
                    return false;
                }
                this->recvBuffer->setBufferContentSize(localReceivedBytes);
                localReceivedBytes -= 4;
                if (verbose) {
                    cout << "Should have received " << (unsigned short) this->recvBuffer->getShort(2)
                         << ", received " << localReceivedBytes << endl;
                }
            }
            if (verbose) {
                printLastError();
                cout << "Post recvfrom... localReceivedBytes = " << localReceivedBytes << "; overwritePartner = "
                     << overwritePartner << endl;
            }

            if (localReceivedBytes >= 0) {
                if (!recvFirstMessage) {
                    if (this->recvBuffer->getChar(0) != 0) {
                        // wrong data, ignore, pretend like nothing was received (like recv from wrong partner)
                        recvFromCorrectPartner = false;
                    } else {
                        recvFromCorrectPartner = this->checkCorrectReceivePartner(overwritePartner,
                                                                                  recvFirstMessage);
                        assert (recvFromCorrectPartner);
                        memcpy(buffer + receivedBytes, this->recvBuffer->getBuffer() + 4,
                               localReceivedBytes * sizeof(char));
                    }
                    // make recvBuffer empty
                    this->recvBuffer->setBufferContentSize(0);
                } else {
                    if (this->recvBuffer->getChar(0) == 0) {
                        // received start of another message...
                        // check if the partner is the same
                        recvFromCorrectPartner = this->checkCorrectReceivePartner(overwritePartner,
                                                                                  recvFirstMessage);
                        if (recvFromCorrectPartner) {
                            // if the partner is ok, interrupt receive! and don't reset the recvBuffer
                            localReceivedBytes = -2;
                            setErrnoZero();  // <- to ensure that this will be interpreted as a timeout :)
                        } else {
                            // if the partner is not ok, ignore message
                            this->recvBuffer->setBufferContentSize(0);
                        }
                    } else {
                        memcpy(buffer + receivedBytes, this->recvBuffer->getBuffer() + 4,
                               localReceivedBytes * sizeof(char));
                        this->recvBuffer->setBufferContentSize(0);
                    }
                }
            }

            /*
            // in previous if-statement, localReceivedBytes can get changed!
            if (localReceivedBytes >= 0) {
                recvFromCorrectPartner = this->checkCorrectReceivePartner(overwritePartner, recvFirstMessage);
            }
            //*/

            if (verbose) {
                cout << "Post recvfrom... managed to receive localReceivedBytes = " << localReceivedBytes
                     << endl;
                /*
                for (int i = 0; i < localReceivedBytes; i++) {
                    cout << (int) buffer[i + receivedBytes] << ", ";
                }
                cout << endl;
                //*/
            }
            break;
        }
        case TCP: {
            localReceivedBytes = recv(this->socket, buffer + receivedBytes, receiveSize, 0);
            break;
        }
        default : {
            throw runtime_error("Unknown protocol: " + to_string(this->protocol));
        }
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
        if (localReceivedBytes != -1 && localReceivedBytes != -2) {
            cout << "The next assertion will fail: localReceivedBytes = " << localReceivedBytes << endl;
        }
        assert (localReceivedBytes == -1 || localReceivedBytes == -2);
        if (localReceivedBytes == -2) {
            errorCode = -2;
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

bool Socket::_receiveBytes(char *buffer, unsigned long long int expectedLength, int &errorCode, int retries,
                           bool verbose) {
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
    unsigned long long int receivedBytes = 0, maxPossibleReceiveBytes = CLIENT_MAX_MESSAGE_BYTES;
    errorCode = 0;
    bool recvFirstMessage = false, retry, recvFromCorrectPartner = false;
    bool overwritePartner =
            (this->partner == nullptr) || (!this->partner->isInitialized()) || (this->partner->getOverwrite());
    if (this->protocol == SocketType::UDP_HEADER && this->recvBuffer == nullptr) {
        this->recvBuffer = new Buffer(maxPossibleReceiveBytes + 4);
    }
    while (receivedBytes < expectedLength) {
        receiveSize = (int) min(expectedLength - receivedBytes, maxPossibleReceiveBytes);
        // wait to receive data from socket (and retry when timeout occurs)
        do {
            if (!checkErrno(errorCode, "Socket::_receiveBytes - begin do-while")) {
                assert (false);
            }
            if (!this->performReceive(buffer, localReceivedBytes, overwritePartner, recvFromCorrectPartner, receiveSize,
                                      receivedBytes, recvFirstMessage, verbose)) {
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
    }
    assert(receivedBytes == expectedLength);
    if (verbose) {
        cout << this_thread::get_id() << ": Exiting _receiveBytes function having received = " << expectedLength
             << "B to " << ((this->partner == nullptr) ? "any" : this->partner->getStringAddress()) << "!" << endl;
    }
    return true;
}
