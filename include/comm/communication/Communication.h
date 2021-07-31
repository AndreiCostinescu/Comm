//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 14.03.2021.
//

#ifndef COMM_COMMUNICATION_COMMUNICATION_H
#define COMM_COMMUNICATION_COMMUNICATION_H

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

        bool transmitData(SocketType socketType, CommunicationData *data, bool withHeader, bool withMessageType = true,
                          int retries = 0, bool verbose = false);

        bool sendRaw(SocketType socketType, const char *data, int dataSize, int retries = 0, bool verbose = false);

        bool recvMessageType(SocketType socketType, MessageType &messageType, bool withHeader,
                             bool syphonWronglySerializedData, int retries = 0, bool verbose = false);

        bool recvData(SocketType socketType, CommunicationData *data, bool withHeader, bool syphonWrongSerialize,
                      bool gotMessageType = true, int retries = 0, bool verbose = false);

        bool receiveData(SocketType socketType, DataCollection *data, bool withHeader, bool syphonWrongSerialize,
                         int retries = 0, bool verbose = false);

        bool receiveRaw(SocketType socketType, char *&data, bool &receivedData, int retries = 0, bool verbose = false);

        void createSocket(SocketType socketType, SocketPartner *partner = nullptr, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void createSocket(SocketType socketType, const SocketPartner &partner, int myPort = 0, int sendTimeout = -1,
                          int recvTimeout = -1);

        void setSocketTimeouts(int sendTimeout = -1, int recvTimeout = -1);

        void setSocketTimeouts(SocketType socketType, int sendTimeout = -1, int recvTimeout = -1);

        void setSocket(SocketType socketType, Socket *socket);

        void setPartner(SocketType socketType, SocketAddress _partner, bool overwrite);

        void setPartner(SocketType socketType, const std::string &partnerIP, int partnerPort, bool overwrite);

        void setOverwritePartner(SocketType socketType, bool overwrite);

        Socket *&getSocket(SocketType socketType);

        SocketPartner *getMyself(SocketType socketType);

        std::string getMyAddressString(SocketType socketType);

        SocketPartner *&getPartner(SocketType socketType);

        std::string getPartnerString(SocketType socketType);

        [[nodiscard]] int getErrorCode() const;

        [[nodiscard]] const char *getErrorString() const;

    protected:
        virtual void copyData(Communication *copy) const;

        virtual void _cleanup();

        virtual bool send(SocketType socketType, bool withHeader, int retries, bool keepForNextSend, bool verbose);

        virtual bool send(SocketType socketType, const char *buffer, uint64_t contentSize,
                          SerializationHeader *header, int retries, bool keepForNextSend, bool verbose);

        void preReceiveMessageType(char *&dataLocalDeserializeBuffer, uint64_t &expectedSize,
                                   int dataStart);

        void preReceiveData(char *&dataLocalDeserializeBuffer, uint64_t &expectedSize, int dataStart,
                            CommunicationData *recvData, bool withHeader);

        bool doReceive(SocketType socketType, char *&dataLocalDeserializeBuffer, uint64_t &expectedSize,
                       bool withHeader, int retries, bool expectingData, bool verbose);

        bool postReceiveMessageType(MessageType &messageType, bool receiveResult, int dataStart);

        bool postReceiveData(CommunicationData *&recvData, int &deserializeState, int &localRetries,
                             bool &receivedSomething, bool &deserializationDone, MessageType messageType, int dataStart,
                             int localRetriesThreshold, bool receiveResult, bool withHeader, bool verbose);

        virtual bool recv(SocketType socketType, bool withHeader, int retries, bool expectingData, bool verbose);

        virtual bool recv(SocketType socketType, char *&buffer, uint64_t &bufferSize, uint64_t expectedBytes,
                          SerializationHeader *expectedHeader, int retries, bool expectingData, bool verbose);

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

#endif //COMM_COMMUNICATION_COMMUNICATION_H
