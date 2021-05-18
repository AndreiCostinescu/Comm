//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATOR_H
#define PINAKOTHEKDRAWING_COMMUNICATOR_H

#include <atomic>
#include <comm/communication/Communication.h>
#include <comm/data/DataCollection.h>
#include <functional>
#include <map>
#include <string>

namespace comm {
    class Communicator {
    public:
        static bool send(Communication *comm, SocketType socketType, CommunicationData *data, bool withHeader,
                         bool withMessageType = true, int retries = 0, bool verbose = false);

        static bool send(Communication *comm, SocketType socketType, const char *data, int dataSize, int retries = 0,
                         bool verbose = false);

        static bool syphon(Communication *comm, SocketType socketType, MessageType &messageType,
                           CommunicationData *data, const bool *quitFlag, int retries = 0, bool verbose = false,
                           int syphonRetries = 10);

        static bool syphon(Communication *comm, SocketType socketType, MessageType &messageType,
                           CommunicationData *data, bool &quitFlag, int retries = 0, bool verbose = false,
                           int syphonRetries = 10);

        static bool syphon(Communication *comm, SocketType socketType, MessageType &messageType,
                           CommunicationData *data, std::atomic<bool> &quitFlag, int retries = 0, bool verbose = false,
                           int syphonRetries = 10);

        static bool listen(Communication *comm, SocketType socketType, MessageType &messageType,
                           DataCollection &_dataCollection, const bool *quitFlag, int retries = 0,
                           bool verbose = false);

        static bool listen(Communication *comm, SocketType socketType, MessageType &messageType,
                           DataCollection &_dataCollection, bool &quitFlag, int retries = 0, bool verbose = false);

        static bool listen(Communication *comm, SocketType socketType, MessageType &messageType,
                           DataCollection &_dataCollection, std::atomic<bool> &quitFlag, int retries = 0,
                           bool verbose = false);

        static bool listenFor(Communication *comm, SocketType socketType, CommunicationData *data, const bool *quitFlag,
                              bool *timeoutResult = nullptr, int countIgnoreOther = -1, int countOther = -1,
                              int retries = 0, bool verbose = false);

        static bool listenFor(Communication *comm, SocketType socketType, CommunicationData *data, bool &quitFlag,
                              bool *timeoutResult = nullptr, int countIgnoreOther = -1, int countOther = -1,
                              int retries = 0, bool verbose = false);

        static bool listenFor(Communication *comm, SocketType socketType, CommunicationData *data,
                              std::atomic<bool> &quitFlag, bool *timeoutResult = nullptr, int countIgnoreOther = -1,
                              int countOther = -1, int retries = 0, bool verbose = false);

    private:
        static bool isReceiveErrorOk(int errorCode, MessageType *messageType, bool nothingOk);

        static bool _syphon(Communication *comm, SocketType socketType, MessageType &messageType,
                            CommunicationData *data, const std::function<bool()> &notQuit, int retries = 0,
                            bool verbose = false, int syphonRetries = 10);

        static bool _listen(Communication *comm, SocketType socketType, MessageType &messageType,
                            DataCollection &_dataCollection, const std::function<bool()> &notQuit, int retries = 0,
                            bool verbose = false);

        static bool _listenFor(Communication *comm, SocketType socketType, CommunicationData *data,
                               const std::function<bool()> &notQuit, bool *timeoutResult = nullptr,
                               int countIgnoreOther = -1, int countOther = -1, int retries = 0, bool verbose = false);
    };
}

#endif //PINAKOTHEKDRAWING_COMMUNICATOR_H
