//
// Created by ga78cat on 14.03.2021.
//

#ifndef PINAKOTHEKDRAWING_TCPSERVER_H
#define PINAKOTHEKDRAWING_TCPSERVER_H

#include <comm/communication/Communication.h>
#include <comm/socket/NetworkIncludes.h>
#include <comm/socket/Socket.h>

namespace comm {
    class TCPServer {
    public:
        explicit TCPServer(int port, int backlog = 5);

        virtual ~TCPServer();

        void cleanup();

        Communication *acceptCommunication();

    private:
        void initServerSocket();

        void listen() const;

        int port, backlog;
        Socket *socket;
        struct timeval socketAcceptTimeout{};
    };
}

#endif //PINAKOTHEKDRAWING_TCPSERVER_H
