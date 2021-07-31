//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 12.03.2021.
//

#ifndef COMM_SOCKET_SOCKET_H
#define COMM_SOCKET_SOCKET_H

#include <comm/socket/SocketPartner.h>
#include <comm/socket/SocketType.h>
#include <comm/utils/Buffer.h>
#include <comm/utils/NetworkIncludes.h>
#include <comm/utils/SerializationHeader.h>
#include <string>

namespace comm {
    class Socket {
    public:
        explicit Socket(SocketType protocol, SocketPartner *partner = nullptr, int myPort = 0, int sendTimeout = -1,
                        int recvTimeout = -1);

        explicit Socket(SocketType protocol, const SocketPartner &partner, int myPort = 0, int sendTimeout = -1,
                        int recvTimeout = -1);

        ~Socket();

        [[nodiscard]] SOCKET getSocket() const;

        void cleanup(bool withBuffers = false);

        void close();

        [[nodiscard]] Socket *copy() const;

        bool initialize();

        bool initialize(SocketType _protocol, SocketPartner *_partner = nullptr, int _myPort = 0, int _sendTimeout = -1,
                        int _recvTimeout = -1);

        bool initialize(SocketType _protocol, const SocketPartner &_partner, int _myPort = 0, int _sendTimeout = -1,
                        int _recvTimeout = -1);

        void setSocketTimeouts(int sendTimeout = -1, int recvTimeout = -1, bool modifySocket = true);

        void setOverwritePartner(bool overwrite);

        void setPartner(const std::string &partnerIP, int partnerPort, bool overwrite);

        void setPartner(SocketAddress _partner, bool overwrite);

        void setPartner(SocketPartner *_partner, bool overwrite);

        SocketPartner *&getPartner();

        [[nodiscard]] bool isInitialized() const;

        [[nodiscard]] SocketPartner *&getMyself();

        void accept(Socket *&acceptSocket, bool verbose = false) const;

        bool sendBytes(const char *buffer, uint64_t bufferLength, int &errorCode, SerializationHeader *header,
                       int retries = 0, bool keepForNextSend = false, bool verbose = false);

        bool sendBytes(Buffer &buffer, int &errorCode, SerializationHeader *header, int retries = 0,
                       bool keepForNextSend = false, bool verbose = false);

        bool receiveBytes(char *&buffer, uint64_t &bufferLength, uint64_t expectedLength,
                          int &errorCode, SerializationHeader *expectedHeader, int retries = 0,
                          bool expectingData = false, bool verbose = false);

        bool receiveBytes(Buffer &buffer, int &errorCode, SerializationHeader *expectedHeader, int retries = 0,
                          bool expectingData = false, bool verbose = false);

    private:
        static void _setSocketBufferSizes(SOCKET socket);

        static void _setSocketTimeouts(SOCKET socket, int sendTimeout = -1, int recvTimeout = -1);

        explicit Socket();

        explicit Socket(const SocketPartner &partner);

        void createSocket();

        void initMyself(bool withBind = true, bool verbose = false);

        bool performSend(const char *buffer, int &localBytesSent, int &errorCode, SerializationHeader *header,
                         int sendSize, int sentBytes, char sendIteration, bool keepForNextSend, bool verbose = false);

        static bool interpretSendResult(int &errorCode, int &localBytesSent, int &retries, char &sendIteration,
                                        bool verbose = false);

        bool _sendBytes(const char *buffer, uint64_t bufferLength, int &errorCode,
                        SerializationHeader *header, int retries = 0, bool keepForNextSend = false,
                        bool verbose = false);

        bool checkCorrectReceivePartner(bool &overwritePartner, int receiveIteration);

        bool performReceive(char *buffer, int &localReceivedBytes, bool &overwritePartner, bool &recvFromCorrectPartner,
                            SerializationHeader *expectedHeader, int receiveSize, uint64_t receivedBytes,
                            int receiveIteration, bool expectingData, bool verbose = false);

        static bool interpretReceiveResult(int &errorCode, int &localReceivedBytes, bool &recvFirstMessage,
                                           bool recvFromCorrectPartner, bool verbose = false);

        static void setRetries(int &retries, bool &retry, int maxRetries, bool recvFromCorrectPartner,
                               int localReceivedBytes, bool verbose = false);

        bool _receiveBytes(char *buffer, uint64_t expectedLength, int &errorCode,
                           SerializationHeader *expectedHeader, int retries = 0, bool expectingData = false,
                           bool verbose = false);

        SOCKET socket;
        SocketType protocol;
        SocketPartner *partner, *myself;
        bool isCopy, initialized, deletePartner;
        int recvTimeout, sendTimeout;

        // for UDP protocol
        SocketAddress recvAddress;
        SocketAddressLength recvAddressLength;

        // for UDP_HEADER protocol
        Buffer *sendBuffer, *recvBuffer;
    };
}

#endif //COMM_SOCKET_SOCKET_H
