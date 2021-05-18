//
// Created by ga78cat on 14.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATION_H
#define PINAKOTHEKDRAWING_COMMUNICATION_H

#include <comm/data/CommunicationData.h>
#include <comm/data/DataCollection.h>
#include <comm/data/MessageType.h>
#include <comm/socket/Socket.h>
#include <comm/utils/Buffer.h>
#include <comm/utils/SerializationHeader.h>

namespace comm {
    class Communication {
    public:
        explicit Communication();

        virtual ~Communication();

        virtual void cleanup();

        [[nodiscard]] virtual Communication *copy() const;

        bool transmitData(SocketType type, CommunicationData *data, bool withHeader, bool withMessageType = true,
                          int retries = 0, bool verbose = false);

        bool sendRaw(SocketType type, const char *data, int dataSize, int retries = 0, bool verbose = false);

        bool recvMessageType(SocketType socketType, MessageType &messageType, bool withHeader, int retries = 0,
                             bool verbose = false);

        bool recvData(SocketType socketType, CommunicationData *data, bool withHeader, bool gotMessageType = true,
                      int retries = 0, bool verbose = false);

        bool receiveData(SocketType type, DataCollection *data, bool withHeader, bool withMessageType = true,
                         int retries = 0, bool verbose = false);

        void createSocket(SocketType socketType, SocketPartner *partner = nullptr, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void createSocket(SocketType socketType, const SocketPartner &partner, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void setSocketTimeouts(int sendTimeout = -1, int recvTimeout = -1);

        void setSocketTimeouts(SocketType type, int sendTimeout = -1, int recvTimeout = -1);

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

        virtual bool send(SocketType type, bool withHeader, int retries, bool verbose);

        virtual bool send(SocketType type, const char *buffer, unsigned long long int contentSize,
                          SerializationHeader *header, int retries, bool verbose);

        void preReceiveMessageType(char *&dataLocalDeserializeBuffer, unsigned long long int &expectedSize,
                                   int dataStart);

        void preReceiveData(char *&dataLocalDeserializeBuffer, unsigned long long int &expectedSize, int dataStart,
                            CommunicationData *recvData, bool withHeader);

        bool doReceive(SocketType socketType, char *&dataLocalDeserializeBuffer, unsigned long long int &expectedSize,
                       int retries, bool verbose);

        bool postReceiveMessageType(MessageType &messageType, bool receiveResult, int dataStart);

        bool postReceiveData(CommunicationData *&recvData, int &deserializeState, int &localRetries,
                             bool &receivedSomething, bool &deserializationDone, MessageType messageType, int dataStart,
                             int localRetriesThreshold, bool receiveResult, bool withHeader, bool verbose);

        virtual bool recv(SocketType type, int retries, bool verbose);

        virtual bool recv(SocketType type, char *&buffer, unsigned long long int &bufferSize,
                          unsigned long long int expectedBytes, int retries, bool verbose);

        std::map<SocketType, Socket *> sockets;
        Buffer sendBuffer, recvBuffer;
        bool isCopy;
        int errorCode;
        SerializationHeader sendHeader, recvHeader;

    private:
        void _cleanupData();
    };

    class BroadcastCommunication : public Communication {

    };
}

#endif //PINAKOTHEKDRAWING_COMMUNICATION_H
