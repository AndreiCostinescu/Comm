//
// Created by ga78cat on 11.03.2021.
//

#ifndef PINAKOTHEKDRAWING_STATUSDATA_H
#define PINAKOTHEKDRAWING_STATUSDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/socket/Buffer.h>
#include <string>

namespace comm {
    class StatusData : public CommunicationData {
    public:
        static const int headerSize;

        StatusData();

        explicit StatusData(int size);

        explicit StatusData(const std::string &command);

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        bool deserialize(Buffer *buffer, int start, bool verbose) override;

        void reset();

        void setCommand(const std::string &command);

        void setStatus(const std::string &status);

        void setData(char *_data);

        void setData(char *_data, int _dataSize);

        void setData(const char *_data);

        void setData(const char *_data, int _dataSize);

        void setData(const std::string& _data);

        void setData(const std::string& _data, int _dataSize);

        [[nodiscard]] char *getData() const;

        [[nodiscard]] int getDataSize() const;

        [[nodiscard]] char getDataType() const;

    private:
        static const char statusDataType;

        char *data;
        int dataSize, dataLength;
        char dataType;
    };
}

#endif //PINAKOTHEKDRAWING_STATUSDATA_H