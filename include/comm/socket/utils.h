//
// Created by ga78cat on 16.03.2021.
//

#ifndef PINAKOTHEKDRAWING_UTILS_H
#define PINAKOTHEKDRAWING_UTILS_H

#include <comm/utils/NetworkIncludes.h>
#include <string>

namespace comm {
    extern const int SOCKET_ACCEPT_TIMEOUT_SECONDS;

    extern const unsigned long long int CLIENT_MAX_MESSAGE_BYTES;

    extern const int SOCKET_BUFFER_RECV_SIZE;

    extern const int SOCKET_BUFFER_SEND_SIZE;

    void signalHandler(int signal);

    void startupSockets();

    std::string getAddressIP(SocketAddress address);

    int getAddressPort(SocketAddress address);

    std::string getStringAddress(SocketAddress address);

    void prepareBuffer(char *&buffer, int desiredLength);

    void prepareBuffer(char *&buffer, int &bufferLength, int desiredLength);

    void prepareBuffer(char *&buffer, unsigned long long int &bufferLength, unsigned long long int desiredLength);

    int getLastError();

    const char *getLastErrorString();

    const char *getLastErrorString(int errorCode);

    void printLastError();

    void setErrnoZero();

    bool checkErrno(int errorCode, const std::string &where);

    bool checkErrno(const std::string &where);

    bool comparePartners(const SocketAddress &p1, const SocketAddress &p2);
}

#endif //PINAKOTHEKDRAWING_UTILS_H
