//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATOR_H
#define PINAKOTHEKDRAWING_COMMUNICATOR_H

#include <atomic>
#include <comm/communication/Communication.h>
#include <comm/data/CommunicatorState.h>
#include <comm/data/DataCollection.h>
#include <functional>
#include <map>
#include <string>

namespace comm {
    class Communicator {
    public:
        Communicator();

        virtual ~Communicator();

        virtual void main();

        virtual void stop();

        static bool send(Communication *comm, SocketType type, CommunicationData *data, int retries = 0,
                         bool verbose = false);

        static bool send(Communication *comm, SocketType type, MessageType *messageType, CommunicationData *data,
                         int retries = 0, bool verbose = false);

        static bool syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                           bool &quitFlag, int retries = 0, bool verbose = false, int syphonRetries = 10);

        static bool syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                           std::atomic<bool> &quitFlag, int retries = 0, bool verbose = false, int syphonRetries = 10);

        static bool listen(Communication *comm, SocketType type, MessageType &messageType,
                           DataCollection &_dataCollection, bool &quitFlag);

        static bool listen(Communication *comm, SocketType type, MessageType &messageType,
                           DataCollection &_dataCollection, std::atomic<bool> &quitFlag);

        static bool listenFor(Communication *comm, SocketType type, CommunicationData *data, bool &quitFlag,
                              bool *timeoutResult = nullptr, int countIgnoreOther = -1, int countOther = -1);

        static bool listenFor(Communication *comm, SocketType type, CommunicationData *data,
                              std::atomic<bool> &quitFlag, bool *timeoutResult = nullptr, int countIgnoreOther = -1,
                              int countOther = -1);

    protected:
        bool syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                    int retries = 0, bool verbose = false, int syphonRetries = 10);

        bool listen(Communication *comm, SocketType type, MessageType &messageType, DataCollection &_dataCollection);

        bool listenFor(Communication *comm, SocketType type, CommunicationData *data, bool *timeoutResult = nullptr,
                       int countIgnoreOther = -1, int countOther = -1);

        virtual void _preMain();

        virtual void _main();

        virtual void _postMain();

        std::atomic<bool> quit;
        CommunicatorState state;
        DataCollection dataCollection;

    private:
        static bool isReceiveErrorOk(int errorCode, MessageType *messageType, bool nothingOk);

        static bool _syphon(Communication *comm, SocketType type, MessageType &messageType, CommunicationData *data,
                            const std::function<bool()> &notQuit, int retries = 0, bool verbose = false,
                            int syphonRetries = 10);

        static bool _listen(Communication *comm, SocketType type, MessageType &messageType,
                            DataCollection &_dataCollection, const std::function<bool()> &notQuit);

        static bool _listenFor(Communication *comm, SocketType type, CommunicationData *data,
                               const std::function<bool()> &notQuit, bool *timeoutResult = nullptr,
                               int countIgnoreOther = -1, int countOther = -1);
    };
}

#endif //PINAKOTHEKDRAWING_COMMUNICATOR_H
