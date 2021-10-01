//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 16.03.2021.
//

#ifndef COMM_DATA_COMMUNICATIONDATA_H
#define COMM_DATA_COMMUNICATIONDATA_H

#include <comm/data/MessageType.h>
#include <comm/utils/Buffer.h>

namespace comm {
    class CommunicationData {
    public:
        CommunicationData();

        virtual ~CommunicationData();

        virtual MessageType getMessageType() = 0;

        void resetSerializeState();

        virtual bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) = 0;

        [[nodiscard]] virtual int getExpectedDataSize() const = 0;

        virtual char *getDeserializeBuffer();

        void resetDeserializeState();

        virtual bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) = 0;

    protected:
        int serializeState, deserializeState;
    };
}

#endif //COMM_DATA_COMMUNICATIONDATA_H
