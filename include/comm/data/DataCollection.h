//
// Created by Andrei on 01-Apr-21.
//

#ifndef PINAKOTHEKDRAWING_DATACOLLECTION_H
#define PINAKOTHEKDRAWING_DATACOLLECTION_H

#include <comm/data/CommunicationData.h>
#include <map>
#include <string>

namespace comm {
    class DataCollection {
    public:
        DataCollection();

        ~DataCollection();

        CommunicationData *get(const MessageType &messageType);

    private:
        std::map<std::string, CommunicationData *> data;
    };
}


#endif //PINAKOTHEKDRAWING_DATACOLLECTION_H
