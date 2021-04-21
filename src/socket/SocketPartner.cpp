//
// Created by Andrei on 31-Mar-21.
//

#include <comm/socket/SocketPartner.h>
#include <comm/socket/utils.h>
#include <cstring>

using namespace comm;
using namespace std;

SocketPartner::SocketPartner(bool overwritePartner, bool initializePartner) :
        partner(initializePartner ? new SocketAddress() : nullptr), overwrite(overwritePartner), isCopy(false),
        addressSize(sizeof(SocketAddress)) {}

SocketPartner::SocketPartner(const string &ip, int port, bool overwritePartner) :
        SocketPartner(overwritePartner, true) {
    this->setPartner(ip, port);
}

SocketPartner::SocketPartner(SocketAddress *partner, bool overwritePartner) : SocketPartner(overwritePartner) {
    this->partner = partner;
}

SocketPartner::SocketPartner(const SocketAddress &partner, bool overwritePartner) : SocketPartner(overwritePartner) {
    this->setPartner(partner);
}

SocketPartner::~SocketPartner() {
    this->cleanup();
}

SocketPartner *SocketPartner::copy() const {
    auto *copy = new SocketPartner(this->partner, this->overwrite);
    copy->isCopy = true;
    return copy;
}

bool SocketPartner::isInitialized() const {
    return this->partner != nullptr;
}

void SocketPartner::setOverwrite(bool _overwrite) {
    this->overwrite = _overwrite;
}

void SocketPartner::setPartner(const SocketAddress &_partner) {
    if (!this->isCopy) {
        delete this->partner;
    } else {
        this->isCopy = false;
    }
    this->partner = new SocketAddress;
    memset(this->partner, 0, sizeof(SocketAddress));
    this->partner->sin_family = _partner.sin_family;
    this->partner->sin_port = _partner.sin_port;
    this->partner->sin_addr = _partner.sin_addr;
}

void SocketPartner::setPartner(const std::string &partnerIP, int partnerPort) {
    if (!this->isCopy) {
        delete this->partner;
    } else {
        this->isCopy = false;
    }
    this->partner = new SocketAddress;
    memset(this->partner, 0, sizeof(SocketAddress));
    this->partner->sin_family = AF_INET;
    this->partner->sin_port = htons(partnerPort);
    this->partner->sin_addr.s_addr = partnerIP.empty() ? INADDR_ANY : inet_addr(partnerIP.c_str());
}

bool SocketPartner::getOverwrite() const {
    return this->overwrite;
}

SocketAddress *&SocketPartner::getPartner() {
    return this->partner;
}

SocketAddressLength &SocketPartner::getPartnerSize() {
    return this->addressSize;
}

std::string SocketPartner::getPartnerString() const {
    return this->getStringAddress();
}

std::string SocketPartner::getIP() const {
    if (this->partner == nullptr) {
        return "";
    }
    return inet_ntoa(this->partner->sin_addr);
}

int SocketPartner::getPort() const {
    if (this->partner == nullptr) {
        return 0;
    }
    return ntohs(this->partner->sin_port);
}

std::string SocketPartner::getStringAddress() const {
    return this->getIP() + ":" + to_string(this->getPort());
}

void SocketPartner::cleanup() {
    if (!this->isCopy) {
        delete this->partner;
    } else {
        this->isCopy = false;
    }
    this->partner = nullptr;
}
