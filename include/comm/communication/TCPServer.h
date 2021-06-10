//
// Created by ga78cat on 14.03.2021.
//

#ifndef COMM_COMMUNICATION_TCPSERVER_H
#define COMM_COMMUNICATION_TCPSERVER_H

#include <comm/communication/Communication.h>
#include <comm/socket/Socket.h>
#include <comm/utils/NetworkIncludes.h>

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

#endif //COMM_COMMUNICATION_TCPSERVER_H
