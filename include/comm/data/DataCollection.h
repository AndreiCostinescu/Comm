//
// Created by Andrei on 01-Apr-21.
//

#ifndef PINAKOTHEKDRAWING_DATACOLLECTION_H
#define PINAKOTHEKDRAWING_DATACOLLECTION_H

#include <comm/data/CommunicationData.h>
#include <map>
#include <string>
#include <vector>

namespace comm {
    class DataCollection {
    public:
        DataCollection();

        ~DataCollection();

        void reset();

        void set(const MessageType &messageType, CommunicationData *commData);

        CommunicationData *get(const MessageType &messageType);

    private:
        std::map<std::string, CommunicationData *> data;
        std::vector<std::string> dataKeys;
    };
}


#endif //PINAKOTHEKDRAWING_DATACOLLECTION_H
