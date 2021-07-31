//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 11.03.2021.
//

#ifndef COMM_DATA_STATUSDATA_H
#define COMM_DATA_STATUSDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/utils/Buffer.h>
#include <string>

namespace comm {
    class StatusData : public CommunicationData {
    public:
        static const int headerSize;

        StatusData();

        explicit StatusData(int size);

        explicit StatusData(const std::string &command);

        explicit StatusData(const char *data);

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void reset();

        void setData(char *_data);

        void setData(char *_data, int _dataSize);

        void setData(const char *_data);

        void setData(const char *_data, int _dataSize);

        void setData(const std::string& _data);

        void setData(const std::string& _data, int _dataSize);

        [[nodiscard]] char *getData() const;

        [[nodiscard]] int getDataSize() const;

    private:
        char *data;
        int dataSize, dataLength;
    };
}

#endif //COMM_DATA_STATUSDATA_H
