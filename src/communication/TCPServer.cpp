//
// Created by ga78cat on 14.03.2021.
//

#include <comm/communication/TCPServer.h>
#include <cassert>
#include <comm/socket/utils.h>

using namespace comm;

TCPServer::TCPServer(int port, int backlog) : port(port), backlog(backlog), socket(nullptr) {
    this->socketAcceptTimeout = {.tv_sec = SOCKET_ACCEPT_TIMEOUT_SECONDS, .tv_usec = 0};
    this->initServerSocket();
}

TCPServer::~TCPServer() {
    this->cleanup();
}

void TCPServer::cleanup() {
    delete this->socket;
    this->socket = nullptr;
}

Communication *TCPServer::acceptCommunication() {
    static fd_set readSockets;
    FD_ZERO(&readSockets);
    FD_SET(this->socket->getSocket(), &readSockets);

    int selectResult = select((int) this->socket->getSocket() + 1, &readSockets, nullptr, nullptr,
                              &(this->socketAcceptTimeout));
    if (selectResult > 0) {
        // The accept() call actually accepts an incoming connection
        auto *comm = new Communication();
        this->socket->accept(comm->getSocket(SocketType::TCP));
        return comm;
    } else if (selectResult == SOCKET_ERROR) {
        printLastError();
        this->initServerSocket();
        return nullptr;
    } else {
        // Timeout occurred!
        assert(selectResult == 0);
        return nullptr;
    }
}

void TCPServer::initServerSocket() {
    if (this->socket != nullptr) {
        this->socket->cleanup();
        this->socket->initialize(SocketType::TCP, nullptr, this->port);
    } else {
        this->socket = new Socket(SocketType::TCP, nullptr, this->port);
    }
    this->listen();
}

void TCPServer::listen() const {
    ::listen(this->socket->getSocket(), this->backlog);
}
