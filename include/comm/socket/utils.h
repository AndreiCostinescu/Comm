//
// Created by ga78cat on 16.03.2021.
//

#ifndef COMM_SOCKET_UTILS_H
#define COMM_SOCKET_UTILS_H

#include <comm/utils/NetworkIncludes.h>
#include <string>

namespace comm {
    extern const int SOCKET_ACCEPT_TIMEOUT_SECONDS;

    extern const uint64_t CLIENT_MAX_MESSAGE_BYTES;

    extern const int SOCKET_BUFFER_RECV_SIZE;

    extern const int SOCKET_BUFFER_SEND_SIZE;

    void signalHandler(int signal);

    void startupSockets();

    std::string getAddressIP(SocketAddress address);

    int getAddressPort(SocketAddress address);

    std::string getStringAddress(SocketAddress address);

    void prepareBuffer(char *&buffer, int desiredLength);

    void prepareBuffer(char *&buffer, int &bufferLength, int desiredLength);

    void prepareBuffer(char *&buffer, uint64_t &bufferLength, uint64_t desiredLength);

    int getLastError();

    const char *getLastErrorString();

    const char *getLastErrorString(int errorCode);

    void printLastError();

    void setErrnoZero();

    bool checkErrno(int errorCode, const std::string &where);

    bool checkErrno(const std::string &where);

    bool comparePartners(const SocketAddress &p1, const SocketAddress &p2);
}

#endif //COMM_SOCKET_UTILS_H
