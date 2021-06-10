//
// Created by Andrei on 31-Mar-21.
//

#ifndef COMM_SOCKET_SOCKETPARTNER_H
#define COMM_SOCKET_SOCKETPARTNER_H

#include <comm/utils/NetworkIncludes.h>
#include <string>

namespace comm {
    class SocketPartner {
    public:
        explicit SocketPartner(bool overwritePartner = false, bool initializePartner = false);

        explicit SocketPartner(const std::string &ip, int port, bool overwritePartner);

        explicit SocketPartner(const SocketAddress &partner, bool overwritePartner = false);

        explicit SocketPartner(SocketAddress *partner, bool overwritePartner = false);

        virtual ~SocketPartner();

        [[nodiscard]] SocketPartner *copy() const;

        [[nodiscard]] bool isInitialized() const;

        void setOverwrite(bool _overwrite);

        void setPartner(const SocketAddress &_partner);

        void setPartner(const std::string &partnerIP, int partnerPort);

        [[nodiscard]] bool getOverwrite() const;

        SocketAddress *&getPartner();

        SocketAddressLength &getPartnerSize();

        [[nodiscard]] std::string getPartnerString() const;

        [[nodiscard]] std::string getIP() const;

        [[nodiscard]] int getPort() const;

        [[nodiscard]] std::string getStringAddress() const;

        void cleanup();

    private:
        SocketAddress *partner;
        SocketAddressLength addressSize;
        bool overwrite, isCopy;
    };
}

#endif //COMM_SOCKET_SOCKETPARTNER_H
