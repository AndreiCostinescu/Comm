//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 01-Apr-21.
//

#ifndef COMM_DATA_DATACOLLECTION_H
#define COMM_DATA_DATACOLLECTION_H

#include <comm/data/CommunicationData.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace comm {
    class DataCollection {
    public:
        DataCollection();

        DataCollection(std::function<CommunicationData *(MessageType)> dataCreationFunction);

        virtual ~DataCollection();

        void reset();

        void set(const MessageType &messageType, CommunicationData *commData);

        CommunicationData *get(const MessageType &messageType);

    protected:
        std::map<std::string, CommunicationData *> data;
        std::vector<std::string> dataKeys;
        std::function<CommunicationData *(MessageType)> dataCreationFunction;
    };
}


#endif //COMM_DATA_DATACOLLECTION_H
