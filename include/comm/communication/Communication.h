//
// Created by ga78cat on 14.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATION_H
#define PINAKOTHEKDRAWING_COMMUNICATION_H

#include <comm/data/CommunicationData.h>
#include <comm/data/DataCollection.h>
#include <comm/data/MessageType.h>
#include <comm/socket/Buffer.h>
#include <comm/socket/Socket.h>

namespace comm {
    class Communication {
    public:
        explicit Communication();

        virtual ~Communication();

        virtual void cleanup();

        [[nodiscard]] virtual Communication *copy() const;

        void createSocket(SocketType socketType, SocketPartner *partner = nullptr, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void createSocket(SocketType socketType, const SocketPartner &partner, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void setSocketTimeouts(int sendTimeout = -1, int recvTimeout = -1);

        void setSocketTimeouts(SocketType type, int sendTimeout = -1, int recvTimeout = -1);

        bool sendMessageType(SocketType type, const MessageType *messageType, int retries = 0, bool verbose = false);

        bool sendData(SocketType type, CommunicationData *data, bool withMessageType, int retries = 0,
                      bool verbose = false);

        bool recvMessageType(SocketType type, MessageType *messageType, int retries = 0, bool verbose = false);

        bool recvData(SocketType type, CommunicationData *data, int retries = 0, bool verbose = false);

        void setSocket(SocketType type, Socket *socket);

        void setPartner(SocketType type, SocketAddress _partner, bool overwrite);

        void setPartner(SocketType type, const std::string &partnerIP, int partnerPort, bool overwrite);

        void setOverwritePartner(SocketType type, bool overwrite);

        Socket *&getSocket(SocketType socketType);

        SocketPartner *getMyself(SocketType type);

        std::string getMyAddressString(SocketType type);

        SocketPartner *&getPartner(SocketType type);

        std::string getPartnerString(SocketType type);

        [[nodiscard]] int getErrorCode() const;

        [[nodiscard]] const char *getErrorString() const;

    protected:
        virtual void copyData(Communication *copy) const;

        virtual void _cleanup();

        virtual bool send(SocketType type, int retries, bool verbose);

        virtual bool send(SocketType type, char *buffer, unsigned long long int contentSize, int retries, bool verbose);

        virtual bool recv(SocketType type, int retries, bool verbose);

        virtual bool recv(SocketType type, char *&buffer, unsigned long long int &bufferSize,
                          unsigned long long int expectedBytes, int retries, bool verbose);

        std::map<SocketType, Socket *> sockets;
        Buffer sendBuffer, recvBuffer;
        bool isCopy;
        int errorCode;
        DataCollection dataCollection;

    private:
        void _cleanupData();
    };

    class BroadcastCommunication : public Communication {

    };
}

#endif //PINAKOTHEKDRAWING_COMMUNICATION_H
