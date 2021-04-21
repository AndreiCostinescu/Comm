//
// Created by ga78cat on 16.03.2021.
//

#ifndef PINAKOTHEKDRAWING_UTILS_H
#define PINAKOTHEKDRAWING_UTILS_H

#include <comm/socket/NetworkIncludes.h>
#include <string>

namespace comm {
    extern int bigEndian;

    extern const int SOCKET_ACCEPT_TIMEOUT_SECONDS;

    extern const unsigned long long int CLIENT_MAX_MESSAGE_BYTES;

    extern const int SOCKET_BUFFER_RECV_SIZE;

    extern const int SOCKET_BUFFER_SEND_SIZE;

    void signalHandler(int signal);

    void startupSockets();

    std::string getAddressIP(SocketAddress address);

    int getAddressPort(SocketAddress address);

    std::string getStringAddress(SocketAddress address);

    bool isBigEndian();

    bool isLittleEndian();

    void shortToNetworkBytes(char *buffer, int start, short value);

    short networkBytesToShort(const char *buffer, int start);

    void intToNetworkBytes(char *buffer, int start, int value);

    int networkBytesToInt(const char *buffer, int start);

    void longLongToNetworkBytes(char *buffer, int start, long long value);

    long long networkBytesToLongLong(const char *buffer, int start);

    void doubleToNetworkBytes(char *buffer, int start, double value);

    double networkBytesToDouble(const char *buffer, int start);

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
