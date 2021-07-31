//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 16.03.2021.
//

#include <comm/socket/utils.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>

using namespace comm;
using namespace std;

const int comm::SOCKET_ACCEPT_TIMEOUT_SECONDS = 1;

const uint64_t comm::CLIENT_MAX_MESSAGE_BYTES = 65500;

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

void comm::prepareBuffer(char *&buffer, uint64_t &bufferLength, uint64_t desiredLength) {
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
