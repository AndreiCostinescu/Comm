//
// Created by ga78cat on 16.03.2021.
//

#include <comm/socket/utils.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>

using namespace comm;
using namespace std;

int comm::bigEndian = -1;

const int comm::SOCKET_ACCEPT_TIMEOUT_SECONDS = 1;

const unsigned long long int comm::CLIENT_MAX_MESSAGE_BYTES = 65500;

const int comm::SOCKET_BUFFER_RECV_SIZE = 4 * 1024 * 1024;

const int comm::SOCKET_BUFFER_SEND_SIZE = 4 * 1024 * 1024;

void comm::signalHandler(int signal) {
    // cout << "Caught signal " << signal << endl;
    // exit(2 * signal);
}

void comm::startupSockets() {
    if (!socketsStarted) {
#if defined(_WIN32) || defined(_WIN64)
        WSADATA WSAData;
        WSAStartup(MAKEWORD(2, 0), &WSAData);
#endif
        socketsStarted = true;
    }
}

string comm::getAddressIP(SocketAddress address) {
    return inet_ntoa(address.sin_addr);
}

int comm::getAddressPort(SocketAddress address) {
    return ntohs(address.sin_port);
}

string comm::getStringAddress(SocketAddress address) {
    stringstream s;
    s << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port);
    return s.str();
}

void setBigEndian() {
    auto p = (short) 0xfeff;
    auto *c = (uint8_t *) &p;
    auto x = short(256 * (*c) + (*(c + 1)));
    // cout << "x1 = " << x1 << " and x2 = " << x2 << " : " << (x1 == x2) << endl;
    bigEndian = (p == x);
}

bool comm::isBigEndian() {
    if (bigEndian < 0) {
        setBigEndian();
    }
    return bigEndian == 1;
}

bool comm::isLittleEndian() {
    return !isBigEndian();
}

void comm::shortToNetworkBytes(char *buffer, int start, short value) {
    short x = htons(value);
    memcpy(buffer + start, &x, sizeof(short));
}

short comm::networkBytesToShort(const char *buffer, int start) {
    auto a = static_cast<short>(static_cast<unsigned char>(buffer[start]) << 8 |
                                static_cast<unsigned char>(buffer[start + 1]));
    return a;
}

void comm::intToNetworkBytes(char *buffer, int start, int value) {
    int x = htonl(value);
    memcpy(buffer + start, &x, sizeof(int));
}

int comm::networkBytesToInt(const char *buffer, int start) {
    auto a = static_cast<int>(static_cast<unsigned char>(buffer[start]) << 24 |
                              static_cast<unsigned char>(buffer[start + 1]) << 16 |
                              static_cast<unsigned char>(buffer[start + 2]) << 8 |
                              static_cast<unsigned char>(buffer[start + 3]));
    return a;
}

void comm::longLongToNetworkBytes(char *buffer, int start, long long value) {
    // Network Bytes = Big Endian (0xfeff is stored as 0xfe and 0xff)
    //              Little Endian (0xfeff is stored as 0xff and 0xfe)
    char *c = (char *) &value, inc = 1, size = sizeof(long long) - 1;
    if (isLittleEndian()) {
        c = c + size;
        inc = -1;
    }
    for (int i = 0; i <= size; i++, c += inc) {
        buffer[start + i] = *c;
    }
}

long long comm::networkBytesToLongLong(const char *buffer, int start) {
    long long a = 0;
    char inc = 0, size = sizeof(long long);
    for (int i = size - 1; i >= 0; i--, inc += 8) {
        a |= ((long long) ((unsigned char) buffer[start + i])) << inc;
    }
    return a;
}

void comm::doubleToNetworkBytes(char *buffer, int start, double value) {
    memcpy(buffer + start, &value, sizeof(double));
}

double comm::networkBytesToDouble(const char *buffer, int start) {
    double a;
    memcpy(&a, buffer + start, sizeof(double));
    return a;
    /*
    auto a = static_cast<double>(static_cast<unsigned char>(buffer[start]) << 56 |
                                 static_cast<unsigned char>(buffer[start + 1]) << 48 |
                                 static_cast<unsigned char>(buffer[start + 2]) << 40 |
                                 static_cast<unsigned char>(buffer[start + 3]) << 32 |
                                 static_cast<unsigned char>(buffer[start + 4]) << 24 |
                                 static_cast<unsigned char>(buffer[start + 5]) << 16 |
                                 static_cast<unsigned char>(buffer[start + 6]) << 8 |
                                 static_cast<unsigned char>(buffer[start + 7]));
    return a;
    //*/
}

void comm::floatToNetworkBytes(char *buffer, int start, float value) {
    memcpy(buffer + start, &value, sizeof(float));
}

float comm::networkBytesToFloat(const char *buffer, int start) {
    float a;
    memcpy(&a, buffer + start, sizeof(float));
    return a;
}

void comm::prepareBuffer(char *&buffer, int desiredLength) {
    delete[] buffer;
    // cout << "Initialize new buffer!" << endl;
    buffer = new char[desiredLength];
    // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
    memset(buffer, 0, desiredLength * sizeof(*buffer));
}

void comm::prepareBuffer(char *&buffer, int &bufferLength, int desiredLength) {
    if (bufferLength < desiredLength) {
        delete[] buffer;
        // cout << "Initialize new buffer!" << endl;
        buffer = new char[desiredLength];
        bufferLength = desiredLength;
        // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
        memset(buffer, 0, bufferLength * sizeof(*buffer));
    }
}

void comm::prepareBuffer(char *&buffer, unsigned long long int &bufferLength, unsigned long long int desiredLength) {
    if (bufferLength < desiredLength) {
        delete[] buffer;
        // cout << "Initialize new buffer!" << endl;
        buffer = new char[desiredLength];
        bufferLength = desiredLength;
        // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
        memset(buffer, 0, bufferLength * sizeof(*buffer));
    }
}

int comm::getLastError() {
#if defined(_WIN32) || defined(_WIN64)
    return errno;
    // return WSAGetLastError();
#else
    return errno;
#endif
}

const char *comm::getLastErrorString() {
    return strerror(getLastError());
}

const char *comm::getLastErrorString(int errorCode) {
    return strerror(errorCode);
}

void comm::setErrnoZero() {
#if defined(_WIN32) || defined(_WIN64)
    errno = 0;
    WSASetLastError(0);
#else
    errno = 0;
#endif
}

void comm::printLastError() {
    int lastError = getLastError();
    cout << "lastError = " << lastError << "; " << getLastErrorString(lastError) << endl;
}

bool comm::checkErrno(int errorCode, const string &where) {
    if (errorCode != 0) {
        cout << "Error check in " << where << " failed: " << errorCode << "; " << getLastErrorString(errorCode) << endl;
        return false;
    }
    return true;
}

bool comm::checkErrno(const string &where) {
    cout << "errno check..." << endl;
    return checkErrno(getLastError(), where);
}

bool comm::comparePartners(const SocketAddress &p1, const SocketAddress &p2) {
    return (inet_ntoa(p1.sin_addr) == inet_ntoa(p2.sin_addr)) && (ntohs(p1.sin_port) && ntohs(p2.sin_port));
}
