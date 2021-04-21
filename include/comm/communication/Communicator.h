//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATOR_H
#define PINAKOTHEKDRAWING_COMMUNICATOR_H

#include <atomic>
#include <comm/communication/Communication.h>
#include <comm/data/CommunicatorState.h>
#include <comm/data/DataCollection.h>
#include <map>
#include <string>

namespace comm {
    class Communicator {
    public:
        Communicator();

        virtual ~Communicator();

        virtual void main();

        virtual void stop();

    protected:
        static bool isReceiveErrorOk(int errorCode, MessageType *messageType, bool nothingOk);

        static bool send(Communication *comm, SocketType type, CommunicationData *data, int retries = 0,
                         bool verbose = false);

        static bool send(Communication *comm, SocketType type, MessageType *messageType, CommunicationData *data,
                         int retries = 0, bool verbose = false);

        bool syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                    int retries = 0, bool verbose = false, int syphonRetries = 10);

        bool listen(Communication *comm, SocketType type, MessageType &messageType, DataCollection &_dataCollection);

        bool listenFor(Communication *comm, SocketType type, CommunicationData *data, bool onlyFirstMessage = false);

        virtual void _preMain();

        virtual void _main();

        virtual void _postMain();

        std::atomic<bool> quit;
        CommunicatorState state;
        DataCollection dataCollection;
    };
}

#endif //PINAKOTHEKDRAWING_COMMUNICATOR_H
